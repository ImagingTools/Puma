// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
//
// Recipient routing of the tenant notifications pushed over WebSocket.
//
// Every user of the scenario holds a live session subscribed to all three client
// notification channels, the way the GUI does (AuthorizationController.qml,
// GqlBasedTenantManagementApiClient.qml). Each case performs one action and then
// requires that exactly the intended users received exactly one notification of the
// expected type for that action, and that nobody else - the actor, other members,
// an outsider, the owner of an unrelated organization, a session that subscribed
// naming somebody else's user id - received anything on that channel.
//
// Routing rules under test:
//   InvitationReceived        -> the invited user (also when the invitation is resent)
//   InvitationAccepted        -> the user who sent the invitation
//   InvitationRejected        -> the user who sent the invitation
//   InvitationRevoked         -> the invited user
//   MembershipRoleChanged     -> the member whose organization permissions changed
//   MembershipRemoved         -> the removed member and the organization owner
//   OwnershipTransferred      -> the previous and the new owner
//   ConnectionRequestReceived -> the owner of the requested organization
//   ConnectionRequestApproved -> the owner of the requesting organization
//   ConnectionRequestRejected -> the owner of the requesting organization
//   ConnectionRemoved         -> the owners of both organizations
//   RelationshipProposalReceived -> the owner of the counterparty organization
//   RelationshipProposalApproved -> the owner of the initiating organization
//   RelationshipProposalRejected -> the owner of the initiating organization
//   RelationshipRemoved       -> the owners of both organizations, unless the connection ends with it
//   MessageReceived           -> the owner of the target organization
//   MessageStatusChanged      -> the owner of the source organization

'use strict';

const crypto = require('crypto');
const {
	wsUrl, suLogin, suPassword, productId, registrationGraceMs,
	gqlString, gql, login,
	SubscriptionSession, notificationOf,
	assert, test, requires, runSuite
} = require('./ws-lib');

const MEMBERSHIP = 'OnMembershipNotification';
const CONNECTION = 'OnConnectionCodesNotification';
const MESSAGE = 'OnCrossTenantMessageNotification';
const CHANNELS = [MEMBERSHIP, CONNECTION, MESSAGE];

// How long a misrouted notification gets to show up after the expected ones arrived.
const quietMs = 1500;

const runTag = Date.now().toString(36);
const password = 'WsNotify_2026!';

// --- helpers

// Runs a mutation and fails on GraphQL errors as well as on a refusal in the payload.
async function act(label, query, token) {
	const res = await gql(query, token);
	if (res && res.errors) {
		throw new Error(`${label} returned GraphQL errors: ${JSON.stringify(res.errors).slice(0, 400)}`);
	}
	const payload = res && res.data && res.data[label];
	if (!payload) {
		throw new Error(`${label} returned no data: ${JSON.stringify(res).slice(0, 400)}`);
	}
	if (payload.errorMessage || payload.success === false) {
		throw new Error(`${label} was refused: ${JSON.stringify(payload).slice(0, 400)}`);
	}
	return payload;
}

// SelectTenant ends the session of the token it was called with, so the participant
// carries on with the returned token only - for HTTP and for his WS sessions alike.
async function selectTenant(p, tenantId) {
	const payload = await act('SelectTenant',
		`mutation SelectTenant { SelectTenant(input: { tenantId: ${gqlString(tenantId)} }) { ok userSession { tenantId accessToken } errorMessage } }`,
		p.token);
	const session = payload.userSession;
	assert(session && session.tenantId === tenantId && session.accessToken,
		`SelectTenant did not switch into ${tenantId}: ${JSON.stringify(payload).slice(0, 300)}`);
	p.token = session.accessToken;
	p.tenantToken = session.accessToken;
	return session.accessToken;
}

// The selection names every field a case asserts on (MembershipNotification,
// ConnectionNotification, CrossTenantMessageNotification in the SDL).
const SELECTIONS = {
	[MEMBERSHIP]: 'notificationType membershipId userId tenantId',
	[CONNECTION]: 'notificationType tenantId connectionId requestId proposalId',
	[MESSAGE]: 'notificationType messageId sourceTenantId targetTenantId relationshipId status'
};

function subscriptionQuery(commandId, input) {
	return `subscription ${commandId} { ${commandId}(input: ${input || '{}'}) { ${SELECTIONS[commandId]} } }`;
}

// --- the cast

const people = new Map();

function person(role) {
	return requires(people.get(role), `the "${role}" participant`);
}

async function createUser(role, displayName, suToken) {
	const id = crypto.randomUUID();
	const loginName = `ws_${role}_${runTag}`;
	await act('UserAdd',
		`mutation UserAdd { UserAdd(input: { id: ${gqlString(id)}, typeId: "User", productId: ${gqlString(productId)},` +
		` item: { id: ${gqlString(id)}, productId: ${gqlString(productId)}, name: ${gqlString(displayName)},` +
		` username: ${gqlString(loginName)}, password: ${gqlString(password)}, email: ${gqlString(loginName + '@ws.test')},` +
		' groups: [], roles: [], permissions: [], systemInfos: [] } }) { id } }',
		suToken);
	const auth = await login(loginName, password);
	people.set(role, { role, name: displayName, login: loginName, userId: auth.userId, token: auth.token, subIds: new Map() });
	return people.get(role);
}

// One live session per participant, subscribed to every channel the GUI listens to.
async function openSessions(spyInput) {
	const sessions = [];
	for (const p of people.values()) {
		p.session = new SubscriptionSession(p.role, wsUrl, p.token);
		await p.session.connect();
		await p.session.init();
		for (const commandId of CHANNELS) {
			p.subIds.set(commandId, p.session.beginSubscribe(commandId, subscriptionQuery(commandId)));
		}
		sessions.push(p);
	}

	// The bystander's second session names the invited user in "input": routing has to
	// follow the authenticated user, not what the subscriber claims to be.
	const spy = { role: 'spy', name: 'subscriber naming another user', token: person('bystander').token, subIds: new Map() };
	spy.session = new SubscriptionSession('spy', wsUrl, spy.token);
	await spy.session.connect();
	await spy.session.init();
	for (const commandId of CHANNELS) {
		spy.subIds.set(commandId, spy.session.beginSubscribe(commandId, subscriptionQuery(commandId, spyInput)));
	}
	sessions.push(spy);

	await new Promise(r => setTimeout(r, registrationGraceMs));
	for (const p of sessions) {
		for (const commandId of CHANNELS) {
			p.subIds.set(commandId, p.session.confirmAccepted(p.subIds.get(commandId)));
		}
	}
	return sessions;
}

let everyone = [];

function framesSince(p, commandId, mark) {
	const subId = p.subIds.get(commandId);
	return p.session.frames.slice(mark).filter(f => f.type === 'data' && f.id === subId);
}

// Performs the action and requires the routing described by the expectation:
// each recipient gets exactly one notification carrying every expected field,
// every other session gets nothing at all on that channel. The expectation may be
// a function of what the action returned, for ids the server only now assigned.
async function expectRouting(commandId, expectation, recipients, action) {
	const marks = new Map(everyone.map(p => [p, p.session.mark()]));
	const result = await action();
	const expected = typeof expectation === 'function' ? expectation(result) : expectation;

	// An expected field without a value matches a notification that lacks it too.
	for (const key of Object.keys(expected)) {
		const value = expected[key];
		assert(value !== undefined && value !== null && value !== '',
			`the expectation "${key}" has no value - the action returned ${JSON.stringify(result).slice(0, 300)}`);
	}

	const matches = (frame) => {
		const notification = notificationOf(frame, commandId) || {};
		for (const key of Object.keys(expected)) {
			if (notification[key] !== expected[key]) {
				return false;
			}
		}
		return true;
	};

	for (const p of recipients) {
		const subId = p.subIds.get(commandId);
		await p.session.waitFor(f => f.type === 'data' && f.id === subId && matches(f),
			15000, marks.get(p)).catch(() => {
			throw new Error(`${p.role} did not receive ${JSON.stringify(expected)}; got ` +
				JSON.stringify(framesSince(p, commandId, marks.get(p)).map(f => notificationOf(f, commandId))).slice(0, 400));
		});
	}

	await new Promise(r => setTimeout(r, quietMs));

	const problems = [];
	for (const p of everyone) {
		const frames = framesSince(p, commandId, marks.get(p)).map(f => notificationOf(f, commandId));
		if (recipients.includes(p)) {
			if (frames.length !== 1) {
				problems.push(`${p.role} received ${frames.length} notifications instead of one: ${JSON.stringify(frames).slice(0, 300)}`);
			}
		}
		else if (frames.length > 0) {
			problems.push(`${p.role} must not be notified but received ${JSON.stringify(frames).slice(0, 300)}`);
		}
	}
	assert(problems.length === 0, problems.join('\n        '));

	return result;
}

// The flows both pairs of organizations go through, A -> B and C -> D.

function requestConnectionBetween(sourceTenantId, targetTenantId, code, recipients, token) {
	return expectRouting(CONNECTION,
		sent => ({ notificationType: 'ConnectionRequestReceived', tenantId: targetTenantId, requestId: sent.requestId }), recipients,
		() => act('CreateConnectionRequest',
			`mutation CreateConnectionRequest { CreateConnectionRequest(input: { sourceTenantId: ${gqlString(sourceTenantId)},` +
			` connectionCode: ${gqlString(code)}, message: "ws" }) { requestId errorMessage } }`, token));
}

function proposeRelationshipBetween(connectionId, initiatorTenantId, counterpartyTenantId, recipients, token) {
	return expectRouting(CONNECTION,
		sent => ({ notificationType: 'RelationshipProposalReceived', tenantId: counterpartyTenantId, proposalId: sent.proposalId }), recipients,
		() => act('CreateRelationshipProposal',
			'mutation CreateRelationshipProposal { CreateRelationshipProposal(input: { proposal: { id: "", ' +
			`connectionId: ${gqlString(connectionId)}, proposalType: Create, initiatorTenantId: ${gqlString(initiatorTenantId)},` +
			` counterpartyTenantId: ${gqlString(counterpartyTenantId)}, proposedSourceRole: Customer, proposedTargetRole: Supplier,` +
			' status: Pending, message: "ws" } }) { proposalId errorMessage } }', token));
}

function sendMessageBetween(sourceTenantId, targetTenantId, relationshipId, recipients, token) {
	return expectRouting(MESSAGE,
		sent => ({ notificationType: 'MessageReceived', messageId: sent.messageId, sourceTenantId, targetTenantId, relationshipId }), recipients,
		() => act('SendCrossTenantMessage',
			`mutation SendCrossTenantMessage { SendCrossTenantMessage(input: { sourceTenantId: ${gqlString(sourceTenantId)},` +
			` targetTenantId: ${gqlString(targetTenantId)}, relationshipId: ${gqlString(relationshipId)}, messageType: Custom, payload: "ws" }) { messageId errorMessage } }`,
			token));
}

function acknowledgeMessage(messageId, sourceTenantId, targetTenantId, recipients, token) {
	return expectRouting(MESSAGE,
		{ notificationType: 'MessageStatusChanged', messageId, sourceTenantId, targetTenantId, status: 'Acknowledged' }, recipients,
		() => act('UpdateCrossTenantMessageStatus',
			`mutation UpdateCrossTenantMessageStatus { UpdateCrossTenantMessageStatus(input: { messageId: ${gqlString(messageId)}, status: Acknowledged }) { success errorMessage } }`,
			token));
}

// --- the suite

async function main() {
	const su = await login(suLogin, suPassword);

	// owner: owns organization A; admin: member of A allowed to invite;
	// member: plain member of A; invitee, outsider, guest: users the cases invite;
	// partner: owns organization B; stranger: owns C; bystander: owns D, which has
	// nothing to do with A and B. C and D deal with each other only at the end of
	// the run, which proves that the sessions of the stranger, the bystander and
	// the spy deliver - without that, "received nothing" proves nothing.
	const owner = await createUser('owner', 'WS Owner', su.token);
	const admin = await createUser('admin', 'WS Admin', su.token);
	const member = await createUser('member', 'WS Member', su.token);
	const invitee = await createUser('invitee', 'WS Invitee', su.token);
	const outsider = await createUser('outsider', 'WS Outsider', su.token);
	const guest = await createUser('guest', 'WS Guest', su.token);
	const partner = await createUser('partner', 'WS Partner', su.token);
	const bystander = await createUser('bystander', 'WS Bystander', su.token);
	const stranger = await createUser('stranger', 'WS Stranger', su.token);

	const tenantA = (await act('CreateTenant',
		`mutation CreateTenant { CreateTenant(input: { name: ${gqlString('WS_A_' + runTag)}, description: "ws notifications",` +
		` ownerId: ${gqlString(owner.userId)} }) { tenantId errorMessage } }`, su.token)).tenantId;
	const tenantB = (await act('CreateTenant',
		`mutation CreateTenant { CreateTenant(input: { name: ${gqlString('WS_B_' + runTag)}, description: "ws notifications",` +
		` ownerId: ${gqlString(partner.userId)} }) { tenantId errorMessage } }`, su.token)).tenantId;
	const tenantC = (await act('CreateTenant',
		`mutation CreateTenant { CreateTenant(input: { name: ${gqlString('WS_C_' + runTag)}, description: "ws notifications",` +
		` ownerId: ${gqlString(stranger.userId)} }) { tenantId errorMessage } }`, su.token)).tenantId;
	const tenantD = (await act('CreateTenant',
		`mutation CreateTenant { CreateTenant(input: { name: ${gqlString('WS_D_' + runTag)}, description: "ws notifications",` +
		` ownerId: ${gqlString(bystander.userId)} }) { tenantId errorMessage } }`, su.token)).tenantId;

	// Before any WS session opens: the plain tokens die with these calls.
	await selectTenant(owner, tenantA);
	await selectTenant(partner, tenantB);
	await selectTenant(stranger, tenantC);
	await selectTenant(bystander, tenantD);

	const addMember = async (p) => (await act('AddMembership',
		`mutation AddMembership { AddMembership(input: { userId: ${gqlString(p.userId)}, tenantId: ${gqlString(tenantA)} }) { membershipId errorMessage } }`,
		owner.tenantToken)).membershipId;
	admin.membershipId = await addMember(admin);
	member.membershipId = await addMember(member);
	await selectTenant(admin, tenantA);

	everyone = await openSessions(`{ userId: ${gqlString(invitee.userId)} }`);
	console.log(`Participants: ${everyone.map(p => p.role).join(', ')}\n`);

	// --- membership

	await test('changed organization permissions are announced to that member only', async () => {
		await expectRouting(MEMBERSHIP, { notificationType: 'MembershipRoleChanged', tenantId: tenantA, userId: admin.userId, membershipId: admin.membershipId }, [admin],
			() => act('UpdateMembershipPermissions',
				`mutation UpdateMembershipPermissions { UpdateMembershipPermissions(input: { membershipId: ${gqlString(admin.membershipId)},` +
				' permissions: ["/OrganizationManagement/Members/View", "/OrganizationManagement/Members/Edit/Invite"] }) { success errorMessage } }',
				owner.tenantToken));
	});

	let inviteeInvitationId = null;

	await test('an invitation reaches the invited user only', async () => {
		const payload = await expectRouting(MEMBERSHIP,
			sent => ({ notificationType: 'InvitationReceived', tenantId: tenantA, userId: invitee.userId, membershipId: sent.invitationId }), [invitee],
			() => act('CreateTenantInvitation',
				`mutation CreateTenantInvitation { CreateTenantInvitation(input: { userId: ${gqlString(invitee.userId)},` +
				` tenantId: ${gqlString(tenantA)} }) { invitationId errorMessage } }`, admin.tenantToken));
		inviteeInvitationId = payload.invitationId;
	});

	await test('an accepted invitation is reported to the user who sent it only', async () => {
		requires(inviteeInvitationId, 'the invitation sent by the admin');
		await expectRouting(MEMBERSHIP, { notificationType: 'InvitationAccepted', tenantId: tenantA, userId: invitee.userId, membershipId: inviteeInvitationId }, [admin],
			() => act('AcceptTenantInvitation',
				`mutation AcceptTenantInvitation { AcceptTenantInvitation(input: { invitationId: ${gqlString(inviteeInvitationId)} }) { membershipId success errorMessage } }`,
				invitee.token));
	});

	let outsiderInvitationId = null;

	await test('an invitation sent by the owner reaches the invited user only', async () => {
		const payload = await expectRouting(MEMBERSHIP,
			sent => ({ notificationType: 'InvitationReceived', tenantId: tenantA, userId: outsider.userId, membershipId: sent.invitationId }), [outsider],
			() => act('CreateTenantInvitation',
				`mutation CreateTenantInvitation { CreateTenantInvitation(input: { userId: ${gqlString(outsider.userId)},` +
				` tenantId: ${gqlString(tenantA)} }) { invitationId errorMessage } }`, owner.tenantToken));
		outsiderInvitationId = payload.invitationId;
	});

	await test('a rejected invitation is reported to the user who sent it only', async () => {
		requires(outsiderInvitationId, 'the invitation sent by the owner');
		await expectRouting(MEMBERSHIP, { notificationType: 'InvitationRejected', tenantId: tenantA, userId: outsider.userId, membershipId: outsiderInvitationId }, [owner],
			() => act('RejectTenantInvitation',
				`mutation RejectTenantInvitation { RejectTenantInvitation(input: { invitationId: ${gqlString(outsiderInvitationId)} }) { success errorMessage } }`,
				outsider.token));
	});

	let guestInvitationId = null;

	await test('an invitation to the guest reaches the guest only', async () => {
		const payload = await expectRouting(MEMBERSHIP,
			sent => ({ notificationType: 'InvitationReceived', tenantId: tenantA, userId: guest.userId, membershipId: sent.invitationId }), [guest],
			() => act('CreateTenantInvitation',
				`mutation CreateTenantInvitation { CreateTenantInvitation(input: { userId: ${gqlString(guest.userId)},` +
				` tenantId: ${gqlString(tenantA)} }) { invitationId errorMessage } }`, owner.tenantToken));
		guestInvitationId = payload.invitationId;
	});

	await test('a resent invitation reaches the invited user again and nobody else', async () => {
		requires(guestInvitationId, 'the invitation to the guest');
		await expectRouting(MEMBERSHIP, { notificationType: 'InvitationReceived', tenantId: tenantA, userId: guest.userId, membershipId: guestInvitationId }, [guest],
			() => act('ResendTenantInvitation',
				`mutation ResendTenantInvitation { ResendTenantInvitation(input: { invitationId: ${gqlString(guestInvitationId)} }) { success errorMessage } }`,
				owner.tenantToken));
	});

	await test('a revoked invitation is reported to the invited user only', async () => {
		requires(guestInvitationId, 'the invitation to the guest');
		await expectRouting(MEMBERSHIP, { notificationType: 'InvitationRevoked', tenantId: tenantA, userId: guest.userId, membershipId: guestInvitationId }, [guest],
			() => act('RevokeTenantInvitation',
				`mutation RevokeTenantInvitation { RevokeTenantInvitation(input: { invitationId: ${gqlString(guestInvitationId)} }) { success errorMessage } }`,
				owner.tenantToken));
	});

	await test('an excluded member and the owner are told, nobody else', async () => {
		await expectRouting(MEMBERSHIP, { notificationType: 'MembershipRemoved', tenantId: tenantA, userId: member.userId, membershipId: member.membershipId }, [member, owner],
			() => act('RemoveMembership',
				`mutation RemoveMembership { RemoveMembership(input: { membershipId: ${gqlString(member.membershipId)} }) { success errorMessage } }`,
				owner.tenantToken));
	});

	await test('a member leaving is told to that member and the owner, nobody else', async () => {
		const found = await act('FindMembership',
			`query FindMembership { FindMembership(input: { userId: ${gqlString(invitee.userId)}, tenantId: ${gqlString(tenantA)} }) { membership { id } errorMessage } }`,
			invitee.token);
		const membershipId = requires(found.membership && found.membership.id, 'the membership created by the accepted invitation');
		await expectRouting(MEMBERSHIP, { notificationType: 'MembershipRemoved', tenantId: tenantA, userId: invitee.userId, membershipId }, [invitee, owner],
			() => act('RemoveMembership',
				`mutation RemoveMembership { RemoveMembership(input: { membershipId: ${gqlString(membershipId)} }) { success errorMessage } }`,
				invitee.token));
	});

	// --- connections between organizations

	let connectionCode = null;

	await test('the partner organization accepts connections by code', async () => {
		await act('SetAllowConnectionsByCode',
			`mutation SetAllowConnectionsByCode { SetAllowConnectionsByCode(input: { tenantId: ${gqlString(tenantB)}, allow: true }) { success errorMessage } }`,
			partner.tenantToken);
		const code = await act('GetConnectionCode',
			`query GetConnectionCode { GetConnectionCode(input: { tenantId: ${gqlString(tenantB)} }) { connectionCode { connectionCode } errorMessage } }`,
			partner.tenantToken);
		connectionCode = requires(code.connectionCode && code.connectionCode.connectionCode, 'the connection code of B');
	});

	const requestConnection = () => requestConnectionBetween(tenantA, tenantB, connectionCode, [partner], owner.tenantToken);

	let requestId = null;

	await test('a connection request reaches the owner of the requested organization only', async () => {
		requires(connectionCode, 'the connection code of B');
		requestId = (await requestConnection()).requestId;
	});

	await test('a rejected connection request is reported to the requesting owner only', async () => {
		requires(requestId, 'the pending connection request');
		await expectRouting(CONNECTION, { notificationType: 'ConnectionRequestRejected', tenantId: tenantA, requestId }, [owner],
			() => act('RejectConnectionRequest',
				`mutation RejectConnectionRequest { RejectConnectionRequest(input: { requestId: ${gqlString(requestId)}, tenantId: ${gqlString(tenantB)} }) { success errorMessage } }`,
				partner.tenantToken));
	});

	let connectionId = null;

	await test('an approved connection request is reported to the requesting owner only', async () => {
		requires(connectionCode, 'the connection code of B');
		const secondRequestId = (await requestConnection()).requestId;
		const approved = await expectRouting(CONNECTION, { notificationType: 'ConnectionRequestApproved', tenantId: tenantA, requestId: secondRequestId }, [owner],
			() => act('ApproveConnectionRequest',
				`mutation ApproveConnectionRequest { ApproveConnectionRequest(input: { requestId: ${gqlString(secondRequestId)}, tenantId: ${gqlString(tenantB)} }) { connectionId errorMessage } }`,
				partner.tenantToken));
		connectionId = approved.connectionId;
	});

	const proposeRelationship = () => proposeRelationshipBetween(connectionId, tenantA, tenantB, [partner], owner.tenantToken);

	let proposalId = null;

	await test('a relationship proposal reaches the owner of the counterparty only', async () => {
		requires(connectionId, 'the active connection');
		proposalId = (await proposeRelationship()).proposalId;
	});

	await test('a rejected relationship proposal is reported to the initiating owner only', async () => {
		requires(proposalId, 'the pending relationship proposal');
		await expectRouting(CONNECTION, { notificationType: 'RelationshipProposalRejected', tenantId: tenantA, proposalId }, [owner],
			() => act('RejectRelationshipProposal',
				`mutation RejectRelationshipProposal { RejectRelationshipProposal(input: { proposalId: ${gqlString(proposalId)}, tenantId: ${gqlString(tenantB)} }) { success errorMessage } }`,
				partner.tenantToken));
	});

	let relationshipId = null;

	await test('an approved relationship proposal is reported to the initiating owner only', async () => {
		requires(connectionId, 'the active connection');
		const secondProposalId = (await proposeRelationship()).proposalId;
		const approved = await expectRouting(CONNECTION, { notificationType: 'RelationshipProposalApproved', tenantId: tenantA, proposalId: secondProposalId }, [owner],
			() => act('ApproveRelationshipProposal',
				`mutation ApproveRelationshipProposal { ApproveRelationshipProposal(input: { proposalId: ${gqlString(secondProposalId)}, tenantId: ${gqlString(tenantB)} }) { success relationshipId errorMessage } }`,
				partner.tenantToken));
		relationshipId = approved.relationshipId;
	});

	// --- cross-tenant messages

	let messageId = null;

	await test('a cross-tenant message reaches the owner of the target organization only', async () => {
		requires(relationshipId, 'the applied relationship');
		const sent = await sendMessageBetween(tenantA, tenantB, relationshipId, [partner], owner.tenantToken);
		messageId = sent.messageId;
	});

	await test('a message status change is reported to the owner of the source organization only', async () => {
		requires(messageId, 'the sent message');
		await acknowledgeMessage(messageId, tenantA, tenantB, [owner], partner.tenantToken);
	});

	// --- cross-organization access: a user acts only for the organizations he belongs to

	const refused = async (label, query, token, reason) => {
		const res = await gql(query, token);
		const payload = res && res.data && res.data[label];
		const messages = [].concat((res && res.errors) || []).map(e => e.message)
			.concat(payload && payload.errorMessage ? [payload.errorMessage] : []).join(' ');
		assert(!payload || payload.success !== true, `${label} succeeded: ${JSON.stringify(res).slice(0, 300)}`);
		assert((reason || /Access denied/).test(messages), `${label} was not refused for the expected reason: ${JSON.stringify(res).slice(0, 300)}`);
		return payload;
	};

	await test('the proposals of an organization are listed in full to its members only', async () => {
		const listed = await act('GetRelationshipProposals',
			`query GetRelationshipProposals { GetRelationshipProposals(input: { tenantId: ${gqlString(tenantA)} }) { proposals { id initiatorTenantId counterpartyTenantId status } errorMessage } }`,
			owner.tenantToken);
		const proposals = listed.proposals || [];
		assert(proposals.length >= 2 && proposals.every(p => p.initiatorTenantId === tenantA && p.counterpartyTenantId === tenantB && p.status),
			`proposals are not reported with their parties and status: ${JSON.stringify(proposals).slice(0, 300)}`);

		await refused('GetRelationshipProposals',
			`query GetRelationshipProposals { GetRelationshipProposals(input: { tenantId: ${gqlString(tenantA)} }) { proposals { id } errorMessage } }`,
			bystander.token);
	});

	await test('nobody proposes a relationship on behalf of another organization', async () => {
		requires(connectionId, 'the connection between A and B');
		const proposal = (initiator, counterparty) =>
			'mutation CreateRelationshipProposal { CreateRelationshipProposal(input: { proposal: { id: "", ' +
			`connectionId: ${gqlString(connectionId)}, proposalType: Create, initiatorTenantId: ${gqlString(initiator)},` +
			` counterpartyTenantId: ${gqlString(counterparty)}, proposedSourceRole: Partner, proposedTargetRole: Partner,` +
			' status: Pending } }) { proposalId errorMessage } }';

		await refused('CreateRelationshipProposal', proposal(tenantA, tenantB), bystander.token);
		await refused('CreateRelationshipProposal', proposal(tenantA, tenantB), partner.tenantToken);
		await refused('CreateRelationshipProposal', proposal(tenantC, tenantB), stranger.tenantToken, /not connected/);
	});

	await test('nobody answers a relationship proposal for another organization', async () => {
		requires(proposalId, 'a proposal between A and B');
		for (const [label, field] of [['ApproveRelationshipProposal', 'success'], ['RejectRelationshipProposal', 'success'], ['CancelRelationshipProposal', 'success']]) {
			await refused(label,
				`mutation ${label} { ${label}(input: { proposalId: ${gqlString(proposalId)}, tenantId: ${gqlString(tenantB)} }) { ${field} errorMessage } }`,
				bystander.token);
		}
	});

	await test('an answered proposal cannot be approved again', async () => {
		requires(proposalId, 'the rejected proposal');
		await refused('ApproveRelationshipProposal',
			`mutation ApproveRelationshipProposal { ApproveRelationshipProposal(input: { proposalId: ${gqlString(proposalId)}, tenantId: ${gqlString(tenantB)} }) { success errorMessage } }`,
			partner.tenantToken, /not waiting for approval/);
	});

	await test('nobody sends, reads or settles messages of other organizations', async () => {
		requires(relationshipId, 'the relationship between A and B');
		requires(messageId, 'the message from A to B');
		await refused('SendCrossTenantMessage',
			`mutation SendCrossTenantMessage { SendCrossTenantMessage(input: { sourceTenantId: ${gqlString(tenantA)}, targetTenantId: ${gqlString(tenantB)},` +
			` relationshipId: ${gqlString(relationshipId)}, messageType: Custom, payload: "forged" }) { messageId errorMessage } }`,
			bystander.token);
		await refused('GetCrossTenantMessage',
			`query GetCrossTenantMessage { GetCrossTenantMessage(input: { messageId: ${gqlString(messageId)} }) { message { id } errorMessage } }`,
			bystander.token);
		await refused('GetCrossTenantMessages',
			`query GetCrossTenantMessages { GetCrossTenantMessages(input: { tenantId: ${gqlString(tenantB)} }) { messages { id } errorMessage } }`,
			bystander.token);
		await refused('UpdateCrossTenantMessageStatus',
			`mutation UpdateCrossTenantMessageStatus { UpdateCrossTenantMessageStatus(input: { messageId: ${gqlString(messageId)}, status: Processed }) { success errorMessage } }`,
			bystander.token);

		const own = await act('GetCrossTenantMessage',
			`query GetCrossTenantMessage { GetCrossTenantMessage(input: { messageId: ${gqlString(messageId)} }) { message { id } errorMessage } }`,
			partner.tenantToken);
		assert(own.message && own.message.id === messageId, `the recipient cannot read its message: ${JSON.stringify(own).slice(0, 300)}`);
	});

	await test('nobody manages contracts or relationships of other organizations', async () => {
		requires(relationshipId, 'the relationship between A and B');
		await refused('GetContracts',
			`query GetContracts { GetContracts(input: { tenantId: ${gqlString(tenantA)} }) { contracts { id } errorMessage } }`,
			bystander.token);
		await refused('CreateContract',
			`mutation CreateContract { CreateContract(input: { relationshipId: ${gqlString(relationshipId)}, sourceTenantId: ${gqlString(tenantA)},` +
			` targetTenantId: ${gqlString(tenantB)}, description: "forged" }) { contractId errorMessage } }`,
			bystander.token);
		await refused('RemoveTenantRelationship',
			`mutation RemoveTenantRelationship { RemoveTenantRelationship(input: { tenantId: ${gqlString(tenantA)}, relationshipId: ${gqlString(relationshipId)} }) { success errorMessage } }`,
			bystander.token);
	});

	await test('a user learns only the ids of his own organizations', async () => {
		const idsOf = async (token) => (await act('GetTenantIds', 'query GetTenantIds { GetTenantIds { tenantIds errorMessage } }', token)).tenantIds || [];

		const ownerIds = await idsOf(owner.tenantToken);
		assert(ownerIds.includes(tenantA) && !ownerIds.includes(tenantB) && !ownerIds.includes(tenantC),
			`the owner of A got foreign organization ids: ${JSON.stringify(ownerIds)}`);
		const bystanderIds = await idsOf(bystander.token);
		assert(![tenantA, tenantB, tenantC].some(id => bystanderIds.includes(id)),
			`the bystander got foreign organization ids: ${JSON.stringify(bystanderIds)}`);
		const suIds = await idsOf(su.token);
		assert([tenantA, tenantB, tenantC].every(id => suIds.includes(id)), 'the administrator does not see all organizations');
	});

	await test('only an administrator ensures the system organization', async () => {
		await refused('EnsureSystemTenant',
			'mutation EnsureSystemTenant { EnsureSystemTenant { success errorMessage } }', owner.tenantToken);
	});

	// --- cross-organization grants: only the granting organization delegates its roles

	const grantsOf = async (tenantId, token) => (await act('GetCrossOrgGrants',
		`query GetCrossOrgGrants { GetCrossOrgGrants(input: { tenantId: ${gqlString(tenantId)} }) { grants { id sourceTenantId targetTenantId description } errorMessage } }`,
		token)).grants || [];

	let grantId = null;

	await test('nobody revokes or removes the grants of other organizations', async () => {
		grantId = (await act('CreateCrossOrgGrant',
			`mutation CreateCrossOrgGrant { CreateCrossOrgGrant(input: { sourceTenantId: ${gqlString(tenantA)}, targetTenantId: ${gqlString(tenantB)},` +
			' roleIds: [], description: "ws grant" }) { grantId errorMessage } }', owner.tenantToken)).grantId;
		requires(grantId, 'the grant from A to B');

		for (const token of [bystander.token, stranger.tenantToken]) {
			await refused('RevokeCrossOrgGrant',
				`mutation RevokeCrossOrgGrant { RevokeCrossOrgGrant(input: { grantId: ${gqlString(grantId)} }) { success errorMessage } }`, token);
			await refused('RemoveCrossOrgGrants',
				`mutation RemoveCrossOrgGrants { RemoveCrossOrgGrants(input: { grantIds: [${gqlString(grantId)}] }) { success errorMessage } }`, token);
		}
		assert((await grantsOf(tenantA, owner.tenantToken)).some(g => g.id === grantId), 'the grant from A to B was revoked by an outsider');
	});

	// The owner of C fills a grant document naming A as grantor and saves it; returns the
	// raw SaveDocument response. The document is closed whatever the save answered.
	const saveGrantFromC = async (targetTenantId, description) => {
		const doc = await act('CreateNewDocument',
			'mutation CreateNewDocument { CreateNewDocument(input: { collectionId: "CrossOrgGrants", typeId: "CrossOrgGrant" }) { documentId } }',
			stranger.tenantToken);
		const documentId = requires(doc.documentId, 'a grant document created by the owner of C');

		// CCrossOrgGrantDocumentValidatorComp requires at least one role and does not
		// resolve role ids, so a placeholder keeps the save from failing for that reason.
		const updated = await act('UpdateGrantFromRepresentation',
			'mutation UpdateGrantFromRepresentation { UpdateGrantFromRepresentation(input: { documentId: ' + gqlString(documentId) +
			`, grant: { id: "", sourceTenantId: ${gqlString(tenantA)}, targetTenantId: ${gqlString(targetTenantId)}, name: "ws grantor probe",` +
			` roleIds: ["WsProbeRole"], description: ${gqlString(description)}, isActive: true } }) { status message } }`, stranger.tenantToken);
		assert(updated.status === 'Success', `UpdateGrantFromRepresentation reported ${JSON.stringify(updated).slice(0, 300)}`);

		const saved = await gql('mutation SaveDocument { SaveDocument(input: { collectionId: "CrossOrgGrants", documentId: ' + gqlString(documentId) +
			', documentName: "ws grantor probe" }) { status message } }', stranger.tenantToken);

		const closed = await act('CloseDocument',
			'mutation CloseDocument { CloseDocument(input: { collectionId: "CrossOrgGrants", id: ' + gqlString(documentId) + ' }) { status message } }',
			stranger.tenantToken);
		assert(closed.status === 'Success', `CloseDocument reported ${JSON.stringify(closed).slice(0, 300)}`);

		return saved;
	};

	const grantsWith = async (description) => ({
		inA: (await grantsOf(tenantA, owner.tenantToken)).filter(g => g.description === description),
		inC: (await grantsOf(tenantC, stranger.tenantToken)).filter(g => g.description === description)
	});

	// A grant needs an active connection between grantor and grantee; C and B have none.
	await test('a grant to an unconnected organization is refused and stores nothing', async () => {
		const description = `grant to an unconnected organization ${runTag}`;
		const saved = await saveGrantFromC(tenantB, description);

		const messages = ((saved && saved.errors) || []).map(e => e.message);
		assert(messages.length === 1 && messages[0] === 'The organizations are not connected',
			`SaveDocument was not refused with "The organizations are not connected": ${JSON.stringify(saved).slice(0, 300)}`);

		const stored = await grantsWith(description);
		assert(stored.inA.length === 0 && stored.inC.length === 0,
			`a refused grant was stored anyway: ${JSON.stringify(stored).slice(0, 300)}`);
	});

	await test('the grantor revokes its grant', async () => {
		requires(grantId, 'the grant from A to B');
		await act('RevokeCrossOrgGrant',
			`mutation RevokeCrossOrgGrant { RevokeCrossOrgGrant(input: { grantId: ${gqlString(grantId)} }) { success errorMessage } }`, owner.tenantToken);
		assert(!(await grantsOf(tenantA, owner.tenantToken)).some(g => g.id === grantId), 'the revoked grant is still listed');
	});

	// --- C and D: the positive controls of the stranger, the bystander and the spy
	//
	// Every case above requires these sessions to receive nothing. Each of them now
	// has to receive on every channel, or those absences were silence from a dead
	// subscription. The spy is the bystander authenticated a second time, so routing
	// by identity delivers to both sessions - while what it named in "input" (the
	// invitee) never reached it above.

	let codeOfD = null;

	await test('the bystander organization accepts connections by code', async () => {
		await act('SetAllowConnectionsByCode',
			`mutation SetAllowConnectionsByCode { SetAllowConnectionsByCode(input: { tenantId: ${gqlString(tenantD)}, allow: true }) { success errorMessage } }`,
			bystander.tenantToken);
		const code = await act('GetConnectionCode',
			`query GetConnectionCode { GetConnectionCode(input: { tenantId: ${gqlString(tenantD)} }) { connectionCode { connectionCode } errorMessage } }`,
			bystander.tenantToken);
		codeOfD = requires(code.connectionCode && code.connectionCode.connectionCode, 'the connection code of D');
	});

	const spy = everyone.find(p => p.role === 'spy');
	let connectionCD = null;

	await test('a connection request from C reaches both sessions of the owner of D, and only them', async () => {
		requires(codeOfD, 'the connection code of D');
		const requestCD = (await requestConnectionBetween(tenantC, tenantD, codeOfD, [bystander, spy], stranger.tenantToken)).requestId;
		const approved = await expectRouting(CONNECTION, { notificationType: 'ConnectionRequestApproved', tenantId: tenantC, requestId: requestCD }, [stranger],
			() => act('ApproveConnectionRequest',
				`mutation ApproveConnectionRequest { ApproveConnectionRequest(input: { requestId: ${gqlString(requestCD)}, tenantId: ${gqlString(tenantD)} }) { connectionId errorMessage } }`,
				bystander.tenantToken));
		connectionCD = approved.connectionId;
	});

	let relationshipCD = null;

	await test('a relationship proposal from C reaches both sessions of the owner of D, and only them', async () => {
		requires(connectionCD, 'the connection between C and D');
		const proposalCD = (await proposeRelationshipBetween(connectionCD, tenantC, tenantD, [bystander, spy], stranger.tenantToken)).proposalId;
		const approved = await expectRouting(CONNECTION, { notificationType: 'RelationshipProposalApproved', tenantId: tenantC, proposalId: proposalCD }, [stranger],
			() => act('ApproveRelationshipProposal',
				`mutation ApproveRelationshipProposal { ApproveRelationshipProposal(input: { proposalId: ${gqlString(proposalCD)}, tenantId: ${gqlString(tenantD)} }) { success relationshipId errorMessage } }`,
				bystander.tenantToken));
		relationshipCD = approved.relationshipId;
	});

	await test('a message from C reaches both sessions of the owner of D, and only them', async () => {
		requires(relationshipCD, 'the relationship between C and D');
		const messageCD = (await sendMessageBetween(tenantC, tenantD, relationshipCD, [bystander, spy], stranger.tenantToken)).messageId;
		await acknowledgeMessage(messageCD, tenantC, tenantD, [stranger], bystander.tenantToken);
	});

	// The grantor is the organization of the request context: UpdateGrantFromRepresentation
	// (CCrossOrgGrantCollectionDocumentServiceComp) overwrites whatever sourceTenantId the
	// client names, so naming A from inside C has to produce a grant of C, never one of A.
	// D is connected to C by now; the case fails when nothing was saved at all.
	await test('a grant saved from another organization is stored as that organization\'s own', async () => {
		requires(connectionCD, 'the connection between C and D');
		const description = `grant naming A as grantor ${runTag}`;
		const saved = await saveGrantFromC(tenantD, description);
		assert(saved && !saved.errors && saved.data && saved.data.SaveDocument && saved.data.SaveDocument.status === 'Success',
			`SaveDocument reported ${JSON.stringify(saved).slice(0, 300)}`);

		const stored = await grantsWith(description);
		assert(stored.inA.length === 0, `the owner of C saved a grant in the name of A: ${JSON.stringify(stored.inA).slice(0, 300)}`);
		assert(stored.inC.length === 1 && stored.inC[0].sourceTenantId === tenantC && stored.inC[0].targetTenantId === tenantD,
			`the saved grant is not C's own grant to D: ${JSON.stringify(stored.inC).slice(0, 300)}`);

		await act('RevokeCrossOrgGrant',
			`mutation RevokeCrossOrgGrant { RevokeCrossOrgGrant(input: { grantId: ${gqlString(stored.inC[0].id)} }) { success errorMessage } }`, stranger.tenantToken);
		assert(!(await grantsOf(tenantC, stranger.tenantToken)).some(g => g.id === stored.inC[0].id), 'the revoked grant of C is still listed');
	});

	await test('an invitation from C reaches both sessions of the bystander, and only them', async () => {
		const invitationId = (await expectRouting(MEMBERSHIP,
			sent => ({ notificationType: 'InvitationReceived', tenantId: tenantC, userId: bystander.userId, membershipId: sent.invitationId }), [bystander, spy],
			() => act('CreateTenantInvitation',
				`mutation CreateTenantInvitation { CreateTenantInvitation(input: { userId: ${gqlString(bystander.userId)},` +
				` tenantId: ${gqlString(tenantC)} }) { invitationId errorMessage } }`, stranger.tenantToken))).invitationId;
		await expectRouting(MEMBERSHIP, { notificationType: 'InvitationRejected', tenantId: tenantC, userId: bystander.userId, membershipId: invitationId }, [stranger],
			() => act('RejectTenantInvitation',
				`mutation RejectTenantInvitation { RejectTenantInvitation(input: { invitationId: ${gqlString(invitationId)} }) { success errorMessage } }`,
				bystander.token));
	});

	await test('a removed relationship is reported to the owners of both organizations only', async () => {
		requires(relationshipId, 'the relationship between A and B');
		await expectRouting(CONNECTION, { notificationType: 'RelationshipRemoved', relationshipId }, [owner, partner],
			() => act('RemoveTenantRelationship',
				`mutation RemoveTenantRelationship { RemoveTenantRelationship(input: { tenantId: ${gqlString(tenantA)}, relationshipId: ${gqlString(relationshipId)} }) { success errorMessage } }`,
				owner.tenantToken));
	});

	await test('a removed connection is reported to the owners of both organizations only', async () => {
		requires(connectionId, 'the active connection');
		await expectRouting(CONNECTION, { notificationType: 'ConnectionRemoved', connectionId }, [owner, partner],
			() => act('RemoveConnection',
				`mutation RemoveConnection { RemoveConnection(input: { connectionId: ${gqlString(connectionId)}, tenantId: ${gqlString(tenantA)} }) { success errorMessage } }`,
				owner.tenantToken));
	});

	// --- ownership, last: it changes who owns A

	await test('an ownership transfer is reported to the previous and the new owner only', async () => {
		await expectRouting(MEMBERSHIP, { notificationType: 'OwnershipTransferred', tenantId: tenantA, userId: admin.userId }, [owner, admin],
			() => act('TransferTenantOwnership',
				`mutation TransferTenantOwnership { TransferTenantOwnership(input: { tenantId: ${gqlString(tenantA)}, newOwnerId: ${gqlString(admin.userId)} }) { success errorMessage } }`,
				owner.tenantToken));
	});

	// The sessions that are never the intended recipient in the A/B cases: their
	// "received nothing" counts only for a subscription proven to deliver.
	await test('every session the isolation checks rely on delivered on every channel', async () => {
		const dead = [];
		for (const p of [person('bystander'), spy, person('stranger')]) {
			for (const commandId of CHANNELS) {
				if (p.session.dataFrames(p.subIds.get(commandId)).length === 0) {
					dead.push(`${p.role} / ${commandId}`);
				}
			}
		}
		assert(dead.length === 0,
			'these subscriptions never delivered, so every "must not be notified" check on them proved nothing: ' + dead.join(', '));
	});

	await test('no unaccounted frames or protocol violations on any session', async () => {
		const violations = [].concat(...everyone.map(p => p.session.violations));
		assert(violations.length === 0,
			`${violations.length} protocol violation(s):\n        - ` + violations.join('\n        - '));
	});

	for (const p of everyone) {
		p.session.close();
	}
}

runSuite({ name: 'Tenant API WebSocket notification routing', className: 'TenantApi.WebSocket.Notifications', junitFile: 'junit-report-ws-notifications.xml' }, main);
