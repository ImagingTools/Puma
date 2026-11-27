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
//   OnOrganizationConnectionsChanged -> every session working in an organization whose connections changed

'use strict';

const crypto = require('crypto');
const {
	wsUrl, suLogin, suPassword, productId, frameTimeoutMs,
	gqlString, gql, login,
	SubscriptionSession, notificationOf,
	assert, test, requires, runSuite
} = require('./ws-lib');

const MEMBERSHIP = 'OnMembershipNotification';
const CONNECTION = 'OnConnectionCodesNotification';
const ORGANIZATION_CONNECTIONS = 'OnOrganizationConnectionsChanged';
const CHANNELS = [MEMBERSHIP, CONNECTION, ORGANIZATION_CONNECTIONS];

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
// ConnectionNotification, OrganizationConnectionsNotification in the SDL).
const SELECTIONS = {
	[MEMBERSHIP]: 'notificationType membershipId userId tenantId',
	[CONNECTION]: 'notificationType tenantId connectionId requestId',
	[ORGANIZATION_CONNECTIONS]: 'tenantId'
};

// The organization channel has no input: it follows the organization of the session
function subscriptionQuery(commandId, input) {
	if (commandId === ORGANIZATION_CONNECTIONS) {
		return `subscription ${commandId} { ${commandId} { ${SELECTIONS[commandId]} } }`;
	}

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

	for (const p of sessions) {
		await Promise.all(Array.from(p.subIds.values(), id => p.session.settle(id)));
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
	for (const p of everyone){
		for (const id of p.subIds.values()){
			p.session.confirmAccepted(id);
		}
	}
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
		await p.session.waitFor(f => f.id === subId && f.type === 'data' && matches(f),
			frameTimeoutMs, marks.get(p)).catch(() => {
			const terminal = p.session.frames.slice(marks.get(p)).filter(f => f.id === subId && f.type === 'error');
			throw new Error(`${p.role} did not receive ${JSON.stringify(expected)} on a live pre-mutation subscription; got ` +
				JSON.stringify(framesSince(p, commandId, marks.get(p)).map(f => notificationOf(f, commandId))).slice(0, 400) +
				`; terminal: ${JSON.stringify(terminal.map(f => f.payload)).slice(0, 400)}`);
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

	for (const p of everyone){
		for (const id of p.subIds.values()){
			p.session.confirmAccepted(id);
		}
	}
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

	await test('an excluded member and the owner receive the event on their original subscriptions, nobody else', async () => {
		await expectRouting(MEMBERSHIP, { notificationType: 'MembershipRemoved', tenantId: tenantA, userId: member.userId, membershipId: member.membershipId }, [member, owner],
			() => act('RemoveMembership',
				`mutation RemoveMembership { RemoveMembership(input: { membershipId: ${gqlString(member.membershipId)} }) { success errorMessage } }`,
				owner.tenantToken));
	});

	await test('an excluded member recovers the personal notification after reconnecting without membership', async () => {
		await new Promise((resolve, reject) => {
			const timeout = setTimeout(() => reject(new Error('The previous connection did not close')), frameTimeoutMs);
			member.session.socket.once('close', () => {
				clearTimeout(timeout);
				resolve();
			});
			member.session.close();
		});
		assert(member.session.violations.length === 0, member.session.violations.join('\n'));
		const auth = await login(member.login, password);
		assert(auth.userId === member.userId, 'The returning user must be the same recipient');
		member.token = auth.token;
		member.session = new SubscriptionSession(member.role, wsUrl, member.token);
		await member.session.connect();
		await member.session.init();
		for (const commandId of CHANNELS){
			member.subIds.set(commandId, await member.session.subscribe(commandId, subscriptionQuery(commandId)));
		}

		const membership = await gql(
			`query FindMembership { FindMembership(input: { userId: ${gqlString(member.userId)}, tenantId: ${gqlString(tenantA)} }) { membership { id } errorMessage } }`,
			member.token);
		const current = membership.data && membership.data.FindMembership;
		assert(!membership.errors && current && current.errorMessage === 'Membership not found' && !current.membership,
			'The returning recipient must no longer belong to the organization');

		const list = await act('NotificationsList',
			'query NotificationsList { NotificationsList(input: { pageSize: 100 }) { items { id typeId subject { id } } } }', member.token);
		const ended = list.items.filter(item => item.typeId === 'imtauth.MembershipEnded' && item.subject && item.subject.id === tenantA);
		assert(ended.length === 1 && ended[0].id, 'The personal membership-ended notification must survive reconnection');
		const unrelated = await act('NotificationsList',
			'query NotificationsList { NotificationsList(input: { pageSize: 100 }) { items { id typeId subject { id } } } }', outsider.token);
		assert(!unrelated.items.some(item => item.id === ended[0].id), 'Another recipient must not read the saved notification');
	});

	await test('a leaving member and the owner receive the event on their original subscriptions, nobody else', async () => {
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

	await test('nobody reads or starts the contracts of other organizations', async () => {
		requires(connectionId, 'the connection between A and B');
		await refused('GetContracts',
			`query GetContracts { GetContracts(input: { tenantId: ${gqlString(tenantA)} }) { contracts { id } errorMessage } }`,
			bystander.token);
		// The partner is connected to A, still it does not act for A
		await refused('CreateContract',
			`mutation CreateContract { CreateContract(input: { tenantId: ${gqlString(tenantA)}, draft: { name: "forged",` +
			` parties: [{ tenantId: ${gqlString(tenantA)} }, { tenantId: ${gqlString(tenantB)} }] } }) { success contractId errorMessage } }`,
			partner.tenantToken);
	});

	await test('a draft change reaches the initiator once and the partner only sees it after submission', async () => {
		requires(connectionId, 'the connection between A and B');
		const commandId = 'OnContractNotification';
		const query = `subscription ${commandId} { ${commandId} { notificationType contractId tenantId } }`;
		for (const p of [owner, admin, partner]) {
			p.subIds.set(commandId, await p.session.subscribe(commandId, query));
		}

		const created = await expectRouting(commandId,
			draft => ({ notificationType: 'ContractChanged', contractId: draft.contractId, tenantId: tenantA }), [owner, admin],
			() => act('CreateContract',
				`mutation { CreateContract(input: { tenantId: ${gqlString(tenantA)}, draft: { name: "WebSocket contract draft", typeId: "Services",` +
				` parties: [{ tenantId: ${gqlString(tenantA)}, roleId: "Customer" }, { tenantId: ${gqlString(tenantB)}, roleId: "ServiceProvider" }],` +
				` links: [{ tenantAId: ${gqlString(tenantA)}, tenantBId: ${gqlString(tenantB)}, roleAId: "Customer", roleBId: "ServiceProvider" }] } })` +
				' { success contractId revision errorMessage } }', owner.tenantToken));

		await expectRouting(commandId, { notificationType: 'ContractChanged', contractId: created.contractId }, [owner, admin, partner],
			() => act('SubmitContract',
				`mutation { SubmitContract(input: { tenantId: ${gqlString(tenantA)}, contractId: ${gqlString(created.contractId)}, revision: ${created.revision} }) { success errorMessage } }`,
				owner.tenantToken));
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

	await test('a removed connection is reported to the owners of both organizations only', async () => {
		requires(connectionId, 'the active connection');
		const marks = new Map(everyone.map(p => [p, p.session.mark()]));
		await expectRouting(CONNECTION, { notificationType: 'ConnectionRemoved', connectionId }, [owner, partner],
			() => act('RemoveConnection',
				`mutation RemoveConnection { RemoveConnection(input: { connectionId: ${gqlString(connectionId)}, tenantId: ${gqlString(tenantA)} }) { success errorMessage } }`,
				owner.tenantToken));

		// The sessions working in A (the owner and the administrator) and in B follow the change of their address books;
		// the member has not selected A, the others work elsewhere
		const told = new Map([[owner, tenantA], [admin, tenantA], [partner, tenantB]]);
		const problems = [];
		for (const p of everyone) {
			const frames = framesSince(p, ORGANIZATION_CONNECTIONS, marks.get(p)).map(f => notificationOf(f, ORGANIZATION_CONNECTIONS));
			const tenantId = told.get(p);
			if (tenantId && (frames.length !== 1 || frames[0].tenantId !== tenantId)) {
				problems.push(`${p.role} must get one change of ${tenantId} but received ${JSON.stringify(frames).slice(0, 300)}`);
			}
			else if (!tenantId && frames.length > 0) {
				problems.push(`${p.role} must not be notified but received ${JSON.stringify(frames).slice(0, 300)}`);
			}
		}
		assert(problems.length === 0, problems.join('\n        '));
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
