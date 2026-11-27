// Builds the folder "32 Contract notifications": run "node tools/contract-notifications-folder.js" after editing it.
// Three organizations negotiate contracts; after every step the inbox of every user is compared exactly, so a
// notification sent twice, to the wrong user or never withdrawn fails the step.
const fs = require('fs');
const path = require('path');

const collectionPath = path.join(__dirname, '..', 'Tenant_System_Full.postman_collection.json');
const FOLDER = '32 Contract notifications';

// X is the initiator (owner: A owner, responsible for C1: A member), Y (B owner) and Z (outsider) are its partners
const SESSIONS = {
	A: '{{cnAToken}}',
	M: '{{cnMToken}}',
	B: '{{cnBToken}}',
	O: '{{cnOToken}}'
};

const CONTRACT_PERMISSIONS = ['View', 'Edit', 'Approve', 'Sign', 'Give', 'Distribute'].map((p) => '"/OrganizationManagement/Contracts/' + p + '"');
const GIVEN_PERMISSION = '/Administration/Users/View';

function item(name, query, options){
	const headers = [];
	if (options.auth){
		headers.push({ key: 'x-authentication-token', value: options.auth });
	}

	const event = [];
	if (options.pre){
		event.push({ listen: 'prerequest', script: { type: 'text/javascript', exec: options.pre } });
	}
	event.push({ listen: 'test', script: { type: 'text/javascript', exec: options.test } });

	return {
		name: 'CN ' + name,
		event,
		request: {
			method: 'POST',
			header: headers,
			body: { mode: 'graphql', graphql: { query, variables: '' } },
			url: { raw: '{{base_url}}', host: ['{{base_url}}'] }
		},
		response: []
	};
}

function setResponsible(description, userId, success){
	return item(description, `mutation SetContractPrivateInfo { SetContractPrivateInfo(input: { tenantId: "{{cnXId}}", contractId: "{{cnC1Id}}", responsibleUserId: "${userId}" }) ${COMMAND_FIELDS} }`, {
		auth: SESSIONS.A,
		test: accepted('SetContractPrivateInfo', [`pm.test('assignment result', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(${success}); });`])
	});
}

const js = (value) => JSON.stringify(value);

// The payload of the command, answered without a GraphQL error; "keep" stores fields of it
function accepted(command, checks, keep){
	return [
		'const json = pm.response.json();',
		`const payload = json.data ? json.data.${command} : null;`,
		`pm.test(${js(command + ' answered')}, function () { pm.expect(json.errors, JSON.stringify(json.errors)).to.eql(undefined); pm.expect(payload, JSON.stringify(json)).to.be.an('object'); });`,
		...Object.keys(keep || {}).map((key) => `if (payload) { pm.environment.set(${js(key)}, ${keep[key]}); }`),
		...(checks || [])
	];
}

// ---------------------------------------------------------------- the world

function signIn(name, loginVar, passwordVar, tokenVar){
	return item(name, `query Authorization { Authorization(input: { login: "{{${loginVar}}}", password: "{{${passwordVar}}}", productId: "{{product_id}}" }) { userId token } }`, {
		test: accepted('Authorization', ["pm.test('signed in', function () { pm.expect(payload.token).to.be.a('string').and.not.empty; });"], { [tokenVar]: 'payload.token' })
	});
}

function select(name, tokenVar, tenantVar){
	return item(name, `mutation SelectTenant { SelectTenant(input: { tenantId: "{{${tenantVar}}}" }) { ok userSession { tenantId accessToken } errorMessage } }`, {
		auth: '{{' + tokenVar + '}}',
		test: accepted('SelectTenant', [`pm.test('works in the organization', function () { pm.expect(payload.ok, JSON.stringify(payload)).to.eql(true); pm.expect(payload.userSession.tenantId).to.eql(pm.environment.get(${js(tenantVar)})); });`],
			{ [tokenVar]: 'payload.userSession ? payload.userSession.accessToken : ""' })
	});
}

function createTenant(letter, ownerVar){
	return item(`SU creates ${letter}`, `mutation CreateTenant { CreateTenant(input: { name: "{{cn${letter}Name}}", description: "Contract notifications", ownerId: "{{${ownerVar}}}" }) { tenantId errorMessage } }`, {
		auth: '{{cnSuToken}}',
		test: accepted('CreateTenant', [`pm.test('created', function () { pm.expect(payload.tenantId, JSON.stringify(payload)).to.be.a('string').and.not.empty; });`], { ['cn' + letter + 'Id']: 'payload.tenantId' })
	});
}

function connect(letter, token){
	return [
		item(`${letter} gives its connection code`, `query GetConnectionCode { GetConnectionCode(input: { tenantId: "{{cn${letter}Id}}" }) { connectionCode { connectionCode } errorMessage } }`, {
			auth: token,
			test: accepted('GetConnectionCode', [], { ['cn' + letter + 'Code']: 'payload.connectionCode ? payload.connectionCode.connectionCode : ""' })
		}),
		item(`X asks ${letter} for a connection`, `mutation CreateConnectionRequest { CreateConnectionRequest(input: { sourceTenantId: "{{cnXId}}", connectionCode: "{{cn${letter}Code}}", message: "contracts" }) { requestId errorMessage } }`, {
			auth: SESSIONS.A,
			test: accepted('CreateConnectionRequest', ["pm.test('asked', function () { pm.expect(payload.requestId, JSON.stringify(payload)).to.be.a('string').and.not.empty; });"], { ['cnX' + letter + 'RequestId']: 'payload.requestId' })
		}),
		item(`${letter} approves`, `mutation ApproveConnectionRequest { ApproveConnectionRequest(input: { requestId: "{{cnX${letter}RequestId}}", tenantId: "{{cn${letter}Id}}" }) { connectionId errorMessage } }`, {
			auth: token,
			test: accepted('ApproveConnectionRequest', ["pm.test('connected', function () { pm.expect(payload.connectionId, JSON.stringify(payload)).to.be.a('string').and.not.empty; });"])
		})
	];
}

// ---------------------------------------------------------------- contracts

// X is the customer, linked with every other party
function draft(nameVar, letters){
	const parties = ['X', ...letters].map((l, i) => `{ tenantId: "{{cn${l}Id}}", roleId: "${i === 0 ? 'Customer' : 'ServiceProvider'}" }`).join(', ');
	const links = letters.map((l) => `{ tenantAId: "{{cnXId}}", tenantBId: "{{cn${l}Id}}", roleAId: "Customer", roleBId: "ServiceProvider" }`).join(', ');
	return `{ name: "{{${nameVar}}}", typeId: "Services", termDays: 365, autoRenew: true, renewalPeriodDays: 365, renewalNoticeDays: 30, terminationNoticeDays: 30, parties: [${parties}], links: [${links}] }`;
}

const COMMAND_FIELDS = '{ success contractId revision errorMessage }';

// A command of the contract `c` (variables cn<c>Id, cn<c>Rev): it succeeds and keeps the revision it returns
function command(name, token, mutation, c, tenantLetter, extra){
	return item(name, `mutation ${mutation} { ${mutation}(input: { tenantId: "{{cn${tenantLetter}Id}}", contractId: "{{cn${c}Id}}", revision: {{cn${c}Rev}}${extra || ''} }) ${COMMAND_FIELDS} }`, {
		auth: token,
		test: accepted(mutation, ["pm.test('done', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"],
			{ ['cn' + c + 'Rev']: 'payload.revision !== undefined && payload.revision !== null ? payload.revision : pm.environment.get(' + js('cn' + c + 'Rev') + ')' })
	});
}

function notice(name, token, c, tenantLetter, isImmediate){
	return item(name, `mutation GiveContractNotice { GiveContractNotice(input: { tenantId: "{{cn${tenantLetter}Id}}", contractId: "{{cn${c}Id}}", reason: "Leaving", isImmediate: ${isImmediate} }) ${COMMAND_FIELDS} }`, {
		auth: token,
		test: accepted('GiveContractNotice', ["pm.test('done', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"])
	});
}

function createContract(c, letters){
	return item(`X creates the contract ${c}`, `mutation CreateContract { CreateContract(input: { tenantId: "{{cnXId}}", draft: ${draft('cn' + c + 'Name', letters)} }) ${COMMAND_FIELDS} }`, {
		auth: SESSIONS.A,
		test: accepted('CreateContract', ["pm.test('created', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"],
			{ ['cn' + c + 'Id']: 'payload.contractId', ['cn' + c + 'Rev']: 'payload.revision' })
	});
}

function updateContract(name, c, letters){
	return item(name, `mutation UpdateContract { UpdateContract(input: { tenantId: "{{cnXId}}", contractId: "{{cn${c}Id}}", revision: {{cn${c}Rev}}, draft: ${draft('cn' + c + 'Name', letters)} }) ${COMMAND_FIELDS} }`, {
		auth: SESSIONS.A,
		test: accepted('UpdateContract', ["pm.test('done', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"], { ['cn' + c + 'Rev']: 'payload.revision' })
	});
}

// ---------------------------------------------------------------- inboxes

/**
	The notifications of the contract `c` in the inbox of the session, compared as a sorted list of
	"<type> x<count> (<partner>)": the type without "imtauth.Contract", the count when grouped, the partner as X, Y or Z,
	and " unlinked" for a notification that does not open the contract.
*/
function inbox(who, sessionKey, c, expected){
	const label = expected.length ? expected.join(', ') : 'nothing';
	return item(`${who} has ${label} of ${c}`, 'query NotificationsList { NotificationsList(input: { pageSize: 200 }) { items { typeId groupCount subject { id name } arguments { id value } } } }', {
		auth: SESSIONS[sessionKey],
		test: [
			'const json = pm.response.json();',
			'const items = (json.data && json.data.NotificationsList && json.data.NotificationsList.items) || [];',
			`const contractName = pm.environment.get(${js('cn' + c + 'Name')});`,
			"const letters = {}; ['X', 'Y', 'Z'].forEach(function (l) { letters[pm.environment.get('cn' + l + 'Name')] = l; });",
			'const rows = items.filter(function (i) { return i.subject && (i.subject.name || "").indexOf(contractName) >= 0; }).map(function (i) {',
			"	const partner = (i.arguments || []).find(function (a) { return a.id === 'partner'; });",
			"	return i.typeId.replace('imtauth.Contract', '') + (i.groupCount > 1 ? ' x' + i.groupCount : '') + (partner ? ' (' + (letters[partner.value] || partner.value) + ')' : '') + (i.subject.id ? '' : ' unlinked');",
			'}).sort();',
			`pm.test(${js('inbox: ' + label)}, function () { pm.expect(json.errors, JSON.stringify(json.errors)).to.eql(undefined); pm.expect(rows).to.eql(${js(expected.slice().sort())}); });`
		]
	});
}

// The inboxes of all four users at once
function inboxes(c, a, m, b, o){
	return [inbox('A owner of X', 'A', c, a), inbox('A member of X', 'M', c, m), inbox('B owner of Y', 'B', c, b), inbox('Outsider of Z', 'O', c, o)];
}

// ---------------------------------------------------------------- the folder

const items = [
	item('Sign in SU', 'query Authorization { Authorization(input: { login: "{{su_login}}", password: "{{su_password}}", productId: "{{product_id}}" }) { userId token } }', {
		pre: [
			"const run = pm.variables.replaceIn('{{$timestamp}}');",
			"['X', 'Y', 'Z'].forEach(function (l) { pm.environment.set('cn' + l + 'Name', 'CN ' + l + ' ' + run); });",
			"['C1', 'C2', 'C3', 'C4'].forEach(function (c) { pm.environment.set('cn' + c + 'Name', 'CN contract ' + c + ' ' + run); });"
		],
		test: accepted('Authorization', [], { cnSuToken: 'payload.token' })
	}),
	createTenant('X', 'a_owner_user_id'),
	createTenant('Y', 'b_owner_user_id'),
	createTenant('Z', 'outsider_user_id'),
	item('SU adds the A member to X', 'mutation AddMembership { AddMembership(input: { userId: "{{a_member_user_id}}", tenantId: "{{cnXId}}" }) { membershipId errorMessage } }', {
		auth: '{{cnSuToken}}',
		test: accepted('AddMembership', ["pm.test('member', function () { pm.expect(payload.membershipId, JSON.stringify(payload)).to.be.a('string').and.not.empty; });"], { cnMembershipId: 'payload.membershipId' })
	}),
	signIn('Sign in A owner', 'a_owner_login', 'a_owner_password', 'cnAToken'),
	select('A owner works in X', 'cnAToken', 'cnXId'),
	signIn('Sign in A member', 'a_member_login', 'a_member_password', 'cnMToken'),
	select('A member works in X', 'cnMToken', 'cnXId'),
	signIn('Sign in B owner', 'b_owner_login', 'b_owner_password', 'cnBToken'),
	select('B owner works in Y', 'cnBToken', 'cnYId'),
	signIn('Sign in outsider', 'outsider_login', 'outsider_password', 'cnOToken'),
	select('Outsider works in Z', 'cnOToken', 'cnZId'),
	...connect('Y', SESSIONS.B),
	...connect('Z', SESSIONS.O),

	// C1: the whole life of a contract; the A member answers for X, so the A owner hears nothing
	createContract('C1', ['Y', 'Z']),
	item('Y cannot change its private data before C1 is sent', 'mutation SetContractPrivateInfo { SetContractPrivateInfo(input: { tenantId: "{{cnYId}}", contractId: "{{cnC1Id}}", responsibleUserId: "{{b_owner_user_id}}" }) ' + COMMAND_FIELDS + ' }', {
		auth: SESSIONS.B,
		test: accepted('SetContractPrivateInfo', ["pm.test('draft is private to the initiator', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(false); });"])
	}),
	setResponsible('X cannot assign a member of Z to C1', '{{outsider_user_id}}', false),
	item('X saves internal fields of C1', 'mutation SetContractPrivateInfo { SetContractPrivateInfo(input: { tenantId: "{{cnXId}}", contractId: "{{cnC1Id}}", internalNumber: "internal-1", notes: "Review annually", reminderDays: 7 }) ' + COMMAND_FIELDS + ' }', {
		auth: SESSIONS.A,
		test: accepted('SetContractPrivateInfo', ["pm.test('saved', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"])
	}),
	setResponsible('X makes the A member responsible for C1', '{{a_member_user_id}}', true),
	setResponsible('X repeats the same assignment without a new journal entry', '{{a_member_user_id}}', true),
	item('X keeps its internal fields after the assignment', 'query GetContract { GetContract(input: { tenantId: "{{cnXId}}", contractId: "{{cnC1Id}}" }) { privateInfo { internalNumber responsibleUserId notes reminderDays } errorMessage } }', {
		auth: SESSIONS.A,
		test: accepted('GetContract', ["pm.test('assignment preserves other fields', function () { pm.expect(payload.privateInfo).to.include({ internalNumber: 'internal-1', notes: 'Review annually', reminderDays: 7, responsibleUserId: pm.environment.get('a_member_user_id') }); });"])
	}),
	item('X sees two private changes in the journal', 'query GetContractEvents { GetContractEvents(input: { tenantId: "{{cnXId}}", contractId: "{{cnC1Id}}" }) { events { eventType } errorMessage } }', {
		auth: SESSIONS.A,
		test: accepted('GetContractEvents', ["pm.test('two private changes', function () { pm.expect(payload.events.filter(function (event) { return event.eventType === 'PrivateInfoChanged'; })).to.have.lengthOf(2); });"])
	}),
	...inboxes('C1', [], [], [], []),
	command('X sends C1', SESSIONS.A, 'SubmitContract', 'C1', 'X'),
	...inboxes('C1', [], [], ['InvitationReceived (X)'], ['InvitationReceived (X)']),
	command('Y signs C1', SESSIONS.B, 'SignContract', 'C1', 'Y'),
	...inboxes('C1', [], ['Signed (Y)'], [], ['InvitationReceived (X)']),
	command('Z asks for changes of C1', SESSIONS.O, 'RequestContractChanges', 'C1', 'Z', ', comment: "Fix the price"'),
	...inboxes('C1', [], ['ChangesRequested (Z)', 'Signed (Y)'], [], []),
	updateContract('X changes the terms of C1', 'C1', ['Y', 'Z']),
	...inboxes('C1', [], [], ['TermsChanged (X)'], ['TermsChanged (X)']),
	item('SU gives Y its scope', `mutation UpdateTenant { UpdateTenant(input: { tenantId: "{{cnYId}}", name: "{{cnYName}}", tenantPermissions: [${CONTRACT_PERMISSIONS.join(', ')}, "${GIVEN_PERMISSION}"] }) { success errorMessage } }`, {
		auth: '{{cnSuToken}}',
		test: accepted('UpdateTenant', ["pm.test('the scope is set', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"])
	}),
	item('Y gives X a permission before the conclusion', `mutation GiveContractPermissions { GiveContractPermissions(input: { tenantId: "{{cnYId}}", contractId: "{{cnC1Id}}", granteeTenantId: "{{cnXId}}", productId: "{{product_id}}", permissionIds: ["${GIVEN_PERMISSION}"] }) ${COMMAND_FIELDS} }`, {
		auth: SESSIONS.B,
		test: accepted('GiveContractPermissions', ["pm.test('given', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"])
	}),
	...inboxes('C1', [], [], ['TermsChanged (X)'], ['TermsChanged (X)']),
	command('Y signs C1 again', SESSIONS.B, 'SignContract', 'C1', 'Y'),
	command('Z signs C1', SESSIONS.O, 'SignContract', 'C1', 'Z'),
	...inboxes('C1', [], ['Signed x2 (Z)'], [], []),
	command('X signs C1: concluded', SESSIONS.A, 'SignContract', 'C1', 'X'),
	...inboxes('C1', [], ['Concluded', 'PermissionsReceived (Y)'], ['Concluded'], ['Concluded']),
	notice('Y gives notice on C1', SESSIONS.B, 'C1', 'Y', false),
	...inboxes('C1', [], ['Concluded', 'NoticeGiven (Y)', 'PermissionsReceived (Y)'], ['Concluded'], ['Concluded', 'NoticeGiven (Y)']),
	command('Y revokes its notice', SESSIONS.B, 'RevokeContractNotice', 'C1', 'Y'),
	...inboxes('C1', [], ['Concluded', 'NoticeRevoked (Y)', 'PermissionsReceived (Y)'], ['Concluded'], ['Concluded', 'NoticeRevoked (Y)']),
	command('Z stops the renewal of C1', SESSIONS.O, 'GiveContractNonRenewalNotice', 'C1', 'Z'),
	...inboxes('C1', [], ['Concluded', 'NoticeRevoked (Y)', 'PermissionsReceived (Y)', 'RenewalStopped (Z)'], ['Concluded', 'RenewalStopped (Z)'], ['Concluded', 'NoticeRevoked (Y)']),
	item('Y finds the permissions it gave', 'query GetContract { GetContract(input: { tenantId: "{{cnYId}}", contractId: "{{cnC1Id}}" }) { contract { allocations { allocationId grantorTenantId } } errorMessage } }', {
		auth: SESSIONS.B,
		test: accepted('GetContract', [
			"const allocation = ((payload.contract && payload.contract.allocations) || []).find(function (a) { return a.grantorTenantId === pm.environment.get('cnYId'); });",
			"pm.test('one allocation of Y', function () { pm.expect(allocation, JSON.stringify(payload)).to.be.an('object'); });",
			"if (allocation) { pm.environment.set('cnAllocationId', allocation.allocationId); }"])
	}),
	item('Y takes the permissions back', 'mutation WithdrawContractPermissions { WithdrawContractPermissions(input: { tenantId: "{{cnYId}}", contractId: "{{cnC1Id}}", allocationId: "{{cnAllocationId}}" }) ' + COMMAND_FIELDS + ' }', {
		auth: SESSIONS.B,
		test: accepted('WithdrawContractPermissions', ["pm.test('taken back', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"])
	}),
	...inboxes('C1', [], ['Concluded', 'NoticeRevoked (Y)', 'PermissionsReceived (Y)', 'PermissionsWithdrawn (Y)', 'RenewalStopped (Z)'], ['Concluded', 'RenewalStopped (Z)'], ['Concluded', 'NoticeRevoked (Y)']),
	notice('Z leaves C1 at once', SESSIONS.O, 'C1', 'Z', true),
	...inboxes('C1', [], ['Concluded', 'NoticeRevoked (Y)', 'PermissionsReceived (Y)', 'PermissionsWithdrawn (Y)', 'RenewalStopped (Z)', 'Withdrawn (Z)'], ['Concluded', 'RenewalStopped (Z)', 'Withdrawn (Z)'], ['Concluded', 'NoticeRevoked (Y)']),
	item('SU removes the A member from X', 'mutation RemoveMembership { RemoveMembership(input: { membershipId: "{{cnMembershipId}}" }) { success errorMessage } }', {
		auth: '{{cnSuToken}}',
		test: accepted('RemoveMembership', ["pm.test('removed', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"])
	}),
	notice('Y gives notice again', SESSIONS.B, 'C1', 'Y', false),
	inbox('A owner of X: the responsible user left, the owner hears', 'A', 'C1', ['NoticeGiven (Y)']),

	// C2: a removed party and a refusal; nobody is responsible, the A owner hears for X
	createContract('C2', ['Y', 'Z']),
	command('X sends C2', SESSIONS.A, 'SubmitContract', 'C2', 'X'),
	inbox('B owner of Y', 'B', 'C2', ['InvitationReceived (X)']),
	inbox('Outsider of Z', 'O', 'C2', ['InvitationReceived (X)']),
	updateContract('X removes Z from C2', 'C2', ['Y']),
	inbox('Outsider of Z: removed, without a link', 'O', 'C2', ['PartyRemoved (X) unlinked']),
	inbox('B owner of Y: the terms changed, one row', 'B', 'C2', ['TermsChanged (X)']),
	command('Y declines C2', SESSIONS.B, 'DeclineContract', 'C2', 'Y', ', comment: "No"'),
	inbox('A owner of X', 'A', 'C2', ['Declined (Y)']),
	inbox('B owner of Y', 'B', 'C2', []),

	// C3: the initiator withdraws its offer
	createContract('C3', ['Y']),
	command('X sends C3', SESSIONS.A, 'SubmitContract', 'C3', 'X'),
	command('X withdraws C3', SESSIONS.A, 'CancelContract', 'C3', 'X', ', comment: "Not needed"'),
	inbox('B owner of Y', 'B', 'C3', ['Cancelled (X)']),
	inbox('A owner of X', 'A', 'C3', []),

	// C4: the terms change by a file and by a party added by its code; every signature collected so far is gone
	createContract('C4', ['Y']),
	command('X sends C4', SESSIONS.A, 'SubmitContract', 'C4', 'X'),
	command('Y signs C4', SESSIONS.B, 'SignContract', 'C4', 'Y'),
	inbox('A owner of X', 'A', 'C4', ['Signed (Y)']),
	item('X attaches a file of the terms of C4', 'mutation AddContractAttachment { AddContractAttachment(input: { tenantId: "{{cnXId}}", contractId: "{{cnC4Id}}", fileName: "terms.bin", mimeType: "application/octet-stream", data: "AAH//g==" }) ' + COMMAND_FIELDS + ' }', {
		auth: SESSIONS.A,
		test: accepted('AddContractAttachment', ["pm.test('attached', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"],
			{ cnC4Rev: 'payload.revision !== undefined && payload.revision !== null ? payload.revision : pm.environment.get("cnC4Rev")' })
	}),
	inbox('A owner of X: the signature of Y is gone', 'A', 'C4', []),
	inbox('B owner of Y: review the file', 'B', 'C4', ['TermsChanged (X)']),
	item('Y sees the attachment of C4', 'query GetContract { GetContract(input: { tenantId: "{{cnYId}}", contractId: "{{cnC4Id}}" }) { contract { attachments { attachmentId fileName fileSize } } errorMessage } }', {
		auth: SESSIONS.B,
		test: accepted('GetContract', ["pm.test('one attachment of four bytes', function () { pm.expect(payload.contract.attachments).to.have.lengthOf(1); pm.expect(payload.contract.attachments[0]).to.include({ fileName: 'terms.bin', fileSize: 4 }); });"],
			{ cnC4AttachmentId: 'payload.contract.attachments[0].attachmentId' })
	}),
	item('Y downloads exactly the bytes X attached', 'query GetContractAttachment { GetContractAttachment(input: { tenantId: "{{cnYId}}", contractId: "{{cnC4Id}}", attachmentId: "{{cnC4AttachmentId}}" }) { fileName mimeType data errorMessage } }', {
		auth: SESSIONS.B,
		test: accepted('GetContractAttachment', ["pm.test('binary data round-trips', function () { pm.expect(payload).to.include({ fileName: 'terms.bin', mimeType: 'application/octet-stream', data: 'AAH//g==' }); });"])
	}),
	command('Y signs C4 again', SESSIONS.B, 'SignContract', 'C4', 'Y'),
	inbox('A owner of X', 'A', 'C4', ['Signed (Y)']),
	inbox('B owner of Y', 'B', 'C4', []),
	command('X adds Z to C4 by its connection code', SESSIONS.A, 'AddContractPartyByCode', 'C4', 'X', ', connectionCode: "{{cnZCode}}", roleId: "ServiceProvider"'),
	inbox('A owner of X: the signature of Y is gone again', 'A', 'C4', []),
	inbox('B owner of Y: the terms changed with the new party', 'B', 'C4', ['TermsChanged (X)']),
	inbox('Outsider of Z: invited', 'O', 'C4', ['InvitationReceived (X)']),
	command('X withdraws C4', SESSIONS.A, 'CancelContract', 'C4', 'X', ', comment: "Not needed"'),
	inbox('B owner of Y', 'B', 'C4', ['Cancelled (X)']),
	inbox('Outsider of Z', 'O', 'C4', ['Cancelled (X)']),
	inbox('A owner of X', 'A', 'C4', []),

	item('SU removes the organizations of this folder', 'mutation RemoveElements { RemoveElements(input: { collectionId: "Tenants", elementIds: ["{{cnXId}}", "{{cnYId}}", "{{cnZId}}"] }) { success } }', {
		auth: '{{cnSuToken}}',
		test: accepted('RemoveElements', ["pm.test('removed', function () { pm.expect(payload.success, JSON.stringify(payload)).to.eql(true); });"])
	})
];

const collection = JSON.parse(fs.readFileSync(collectionPath, 'utf8'));
const folder = {
	name: FOLDER,
	description: 'Who of the parties hears what of a contract: the responsible user or else the owner, once, never the user who acted; what a step answers is withdrawn.',
	item: items
};
const index = collection.item.findIndex((i) => i.name === FOLDER);
if (index >= 0){
	collection.item[index] = folder;
}
else{
	const cleanupIndex = collection.item.findIndex((i) => i.name.startsWith('99 '));
	collection.item.splice(cleanupIndex >= 0 ? cleanupIndex : collection.item.length, 0, folder);
}

fs.writeFileSync(collectionPath, JSON.stringify(collection, null, 4).replace(/\n/g, '\r\n'));
console.log(FOLDER + ': ' + items.length + ' requests');
