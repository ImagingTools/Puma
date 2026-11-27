// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
'use strict';

const crypto = require('crypto');
const assert = require('node:assert/strict');
const lib = require('../ws/ws-lib');
const { gqlString: str, test } = lib;
const stamp = crypto.randomBytes(6).toString('hex');
const password = 'Contract_Isolation_2026!';
const right = '/Administration/Users/View';
const products = [{ product: 'ContractsP_' + stamp }, { product: 'ContractsQ_' + stamp }];
const sockets = [];
const users = [];
const tenants = [];

async function act(command, query, actor) {
	const response = await lib.gql(query, actor && actor.token, actor ? actor.product : 'Puma',
		actor && actor.server ? { 'x-server-token': actor.server } : {});
	assert.equal(response.errors, undefined, `${command}: ${JSON.stringify(response.errors)}`);
	const result = response.data && response.data[command];
	assert.ok(result, command + ' returned no payload');
	return result;
}

async function signIn(user, product, tenantId) {
	const actor = { ...product };
	const auth = await act('Authorization', `query { Authorization(input: { login: ${str(user.login)}, password: ${str(password)}, productId: ${str(product.product)} }) { token userId } }`, actor);
	assert.ok(auth.token, 'The fixture user must sign in');
	actor.token = auth.token;
	if (tenantId) {
		const selected = await act('SelectTenant', `mutation { SelectTenant(input: { tenantId: ${str(tenantId)} }) { ok errorMessage userSession { accessToken } } }`, actor);
		assert.ok(selected.ok, selected.errorMessage);
		actor.token = selected.userSession.accessToken;
	}
	return actor;
}

async function subscribe(actor, command) {
	const headers = { productId: actor.product };
	if (actor.server) { headers['x-server-token'] = actor.server; }
	const socket = new lib.SubscriptionSession(command + ' ' + actor.product, lib.wsUrl, actor.token || actor.server, headers);
	sockets.push(socket);
	await socket.connect();
	await socket.init();
	const fields = command === 'OnContractsChanged' ? 'contractId productId' : 'contractId tenantId notificationType';
	const id = await socket.subscribe(command, `subscription { ${command} { ${fields} } }`);
	return { socket, id, command };
}

function events(watch, mark, contractId) {
	return watch.socket.frames.slice(mark).filter(f => f.type === 'data' && f.id === watch.id)
		.map(f => lib.notificationOf(f, watch.command)).filter(e => e && e.contractId === contractId);
}

lib.runSuite({ name: 'Contract product isolation', className: 'ContractProductIsolation', junitFile: 'junit-report-contract-product-isolation.xml' }, async () => {
	const auth = await lib.login(lib.suLogin, lib.suPassword);
	const su = { product: 'Puma', token: auth.token };
	try {
		for (const product of products) {
			await act('AddServerTokenClient', `mutation { AddServerTokenClient(input: { clientProductId: ${str(product.product)}, name: "Contracts isolation" }) { success } }`, su);
			const credential = await act('CreateServerToken', `mutation { CreateServerToken(input: { clientProductId: ${str(product.product)}, name: "Contracts isolation", scopes: [] }) { id token success } }`, su);
			assert.ok(credential.success && credential.token, 'The product server credential must be issued');
			product.server = credential.token;
			product.credentialId = credential.id;
			const published = await act('PublishProductPermissions', `mutation { PublishProductPermissions(input: { productId: ${str(product.product)}, tree: { groups: [{ groupId: "Administration", groupName: "Administration", entries: [{ permissionId: ${str(right)}, displayName: "View users" }] }] } }) { success } }`, { ...product, token: product.server });
			assert.ok(published.success, 'The product tree must be published');
		}
		for (const name of ['a', 'b']) {
			const user = { login: 'contract_isolation_' + name + '_' + stamp };
			const registered = await act('RegisterUser', `mutation { RegisterUser(input: { userData: { name: "Contract isolation ${name}", username: ${str(user.login)}, password: ${str(password)}, email: ${str(user.login + '@contracts.test')} }, productId: "Puma" }) { id } }`, su);
			user.id = registered.id;
			assert.ok(user.id);
			users.push(user);
			const tenant = await act('CreateTenant', `mutation { CreateTenant(input: { name: ${str('ContractIsolation_' + name + '_' + stamp)}, ownerId: ${str(user.id)} }) { tenantId errorMessage } }`, su);
			assert.ok(tenant.tenantId, tenant.errorMessage);
			user.tenantId = tenant.tenantId;
			tenants.push(tenant.tenantId);
		}
		const [a, b] = users;
		const aPuma = await signIn(a, { product: 'Puma' }, a.tenantId);
		const bPuma = await signIn(b, { product: 'Puma' }, b.tenantId);
		const code = await act('GetConnectionCode', `query { GetConnectionCode(input: { tenantId: ${str(b.tenantId)} }) { connectionCode { connectionCode } errorMessage } }`, bPuma);
		const connection = await act('CreateConnectionRequest', `mutation { CreateConnectionRequest(input: { sourceTenantId: ${str(a.tenantId)}, connectionCode: ${str(code.connectionCode.connectionCode)} }) { requestId errorMessage } }`, aPuma);
		const approved = await act('ApproveConnectionRequest', `mutation { ApproveConnectionRequest(input: { requestId: ${str(connection.requestId)}, tenantId: ${str(b.tenantId)} }) { connectionId errorMessage } }`, bPuma);
		assert.ok(approved.connectionId, approved.errorMessage);
		const [p, q] = products;
		const aP = await signIn(a, p, a.tenantId);
		const aQ = await signIn(a, q, a.tenantId);
		const bP = await signIn(b, p, b.tenantId);
		const bQ = await signIn(b, q, b.tenantId);
		const draft = `name: "Product isolation ${stamp}", typeId: "Services", parties: [{ tenantId: ${str(a.tenantId)}, roleId: "Customer" }, { tenantId: ${str(b.tenantId)}, roleId: "ServiceProvider" }], links: [{ tenantAId: ${str(a.tenantId)}, tenantBId: ${str(b.tenantId)}, roleAId: "Customer", roleBId: "ServiceProvider" }]`;
		const create = actor => act('CreateContract', `mutation { CreateContract(input: { tenantId: ${str(a.tenantId)}, draft: { ${draft} } }) { success contractId revision errorMessage } }`, actor);
		const cp = await create(aP);
		const cq = await create(aQ);
		assert.ok(cp.success, cp.errorMessage);
		assert.ok(cq.success, cq.errorMessage);
		const id = cp.contractId;
		const input = `tenantId: ${str(a.tenantId)}, contractId: ${str(id)}`;
		const read = actor => act('GetContract', `query { GetContract(input: { ${input} }) { contract { id productId isReadOnly revision } errorMessage } }`, actor);
		await test('Creation fixes the request product and exposes it read only', async () => {
			assert.equal((await read(aP)).contract.productId, p.product);
			assert.equal((await read(aP)).contract.isReadOnly, false);
		});
		await test('Foreign product cannot read a contract or list it', async () => {
			assert.equal((await read(aQ)).contract, undefined);
			const list = await act('GetContracts', `query { GetContracts(input: { tenantId: ${str(a.tenantId)} }) { contracts { id productId } errorMessage } }`, aQ);
			assert.ok(!list.errorMessage, list.errorMessage);
			assert.ok(list.contracts.some(c => c.id === cq.contractId));
			assert.ok(!list.contracts.some(c => c.id === id));
		});
		await test('Puma administrator reads foreign products without edit access', async () => {
			const data = await read(su);
			assert.equal(data.contract.productId, p.product);
			assert.equal(data.contract.isReadOnly, true);
			const result = await act('RemoveContract', `mutation { RemoveContract(input: { ${input} }) { success errorMessage } }`, su);
			assert.equal(result.success, false);
		});
		const foreignCommands = {
			UpdateContract: `${input}, revision: 1, draft: { name: "foreign edit" }`,
			RemoveContract: input,
			SubmitContract: `${input}, revision: 1`,
			ApproveContract: `${input}, revision: 1`,
			SignContract: `${input}, revision: 1`,
			CancelContract: `${input}, reason: "foreign"`,
			SetContractPrivateInfo: `${input}, notes: "foreign"`,
			SetContractAllocationGroups: `${input}, allocationId: "foreign", groupIds: []`,
			AddContractAttachment: `${input}, fileName: "foreign.txt", mimeType: "text/plain", data: "dGVzdA==", isInternal: true`,
			GiveContractPermissions: `${input}, granteeTenantId: ${str(b.tenantId)}, permissionIds: [${str(right)}]`
		};
		for (const [command, argumentsText] of Object.entries(foreignCommands)) {
			await test('Foreign product refuses ' + command, async () => {
				const result = await act(command, `mutation { ${command}(input: { ${argumentsText} }) { success errorMessage } }`, aQ);
				assert.equal(result.success, false);
				assert.ok(result.errorMessage);
			});
		}
		await test('Foreign product cannot read the journal or attachments', async () => {
			const journal = await act('GetContractEvents', `query { GetContractEvents(input: { ${input} }) { events { id } errorMessage } }`, aQ);
			assert.ok(!journal.events || journal.events.length === 0);
			assert.ok(journal.errorMessage);
			const file = await act('GetContractAttachment', `query { GetContractAttachment(input: { ${input}, attachmentId: "file" }) { data errorMessage } }`, aQ);
			assert.ok(!file.data);
			assert.ok(file.errorMessage);
		});
		await test('A right outside the product tree is refused', async () => {
			const result = await act('GiveContractPermissions', `mutation { GiveContractPermissions(input: { ${input}, granteeTenantId: ${str(b.tenantId)}, permissionIds: ["/AccountManagement/View"] }) { success errorMessage } }`, aP);
			assert.equal(result.success, false);
			assert.equal(result.errorMessage, 'The permission is not part of the contract product');
		});
		const watches = await Promise.all([subscribe(bP, 'OnContractNotification'), subscribe(bQ, 'OnContractNotification'), subscribe({ ...p, token: p.server }, 'OnContractsChanged'), subscribe({ ...q, token: q.server }, 'OnContractsChanged')]);
		await test('Push reaches only the product recipient and its product server, once', async () => {
			const marks = watches.map(w => w.socket.mark());
			const sent = await act('SubmitContract', `mutation { SubmitContract(input: { ${input}, revision: 1 }) { success errorMessage } }`, aP);
			assert.ok(sent.success, sent.errorMessage);
			await lib.delay(1000);
			assert.deepEqual(watches.map((w, i) => events(w, marks[i], id).length), [1, 0, 1, 0]);
			assert.equal(events(watches[2], marks[2], id)[0].productId, p.product);
		});
		await test('Notification center sees the contract only in its product', async () => {
			const inbox = actor => act('NotificationsList', 'query { NotificationsList(input: { pageSize: 200 }) { items { typeId subject { id } } } }', actor);
			const rows = await Promise.all([inbox(bP), inbox(bQ), inbox(bPuma)]);
			assert.deepEqual(rows.map(r => r.items.filter(i => i.subject && i.subject.id === id).length), [1, 0, 0]);
		});
		await test('Foreign product commands left the revision unchanged', async () => {
			assert.equal((await read(aP)).contract.revision, 1);
		});
		await test('A grant from the product tree is accepted', async () => {
			const result = await act('GiveContractPermissions', `mutation { GiveContractPermissions(input: { ${input}, granteeTenantId: ${str(b.tenantId)}, permissionIds: [${str(right)}] }) { success errorMessage } }`, aP);
			assert.ok(result.success, result.errorMessage);
		});
		for (const actor of [bP, aP]) {
			const tenantId = actor === bP ? b.tenantId : a.tenantId;
			const signed = await act('SignContract', `mutation { SignContract(input: { tenantId: ${str(tenantId)}, contractId: ${str(id)}, revision: 1 }) { success errorMessage } }`, actor);
			assert.ok(signed.success, signed.errorMessage);
		}
		const groupId = crypto.randomUUID();
		const group = await act('GroupAdd', `mutation { GroupAdd(input: { typeId: "Group", productId: ${str(p.product)}, id: ${str(groupId)}, item: { id: ${str(groupId)}, productId: ${str(p.product)}, name: "Delegates ${stamp}", users: [{ id: ${str(b.id)}, direct: true }], roles: [], parentGroups: [] } }) { id } }`, bP);
		assert.equal(group.id, groupId);
		const withAllocation = await act('GetContract', `query { GetContract(input: { ${input} }) { contract { number allocations { allocationId } } } }`, aP);
		const allocationId = withAllocation.contract.allocations[0].allocationId;
		const distributed = await act('SetContractAllocationGroups', `mutation { SetContractAllocationGroups(input: { tenantId: ${str(b.tenantId)}, contractId: ${str(id)}, allocationId: ${str(allocationId)}, groupIds: [${str(groupId)}] }) { success errorMessage } }`, bP);
		assert.ok(distributed.success, distributed.errorMessage);
		await test('Delegated organization is visible only in the contract product', async () => {
			const list = actor => act('GetUserOrganizations', `query { GetUserOrganizations(input: { id: ${str(b.id)}, productId: ${str(actor.product)} }) { organizations { id isDelegated } } }`, actor);
			const rows = await Promise.all([list(bP), list(bQ), list(bPuma)]);
			assert.deepEqual(rows.map(r => r.organizations.some(t => t.id === a.tenantId && t.isDelegated)), [true, false, false]);
		});
		await test('Delegated organization accepts a session only in the contract product', async () => {
			for (const product of [q, { product: 'Puma' }, p]) {
				const actor = await signIn(b, product);
				const selected = await act('SelectTenant', `mutation { SelectTenant(input: { tenantId: ${str(a.tenantId)} }) { ok errorMessage userSession { accessToken } } }`, actor);
				assert.equal(selected.ok, product === p);
			}
		});
		await test('Document exchange relationships stay in their contract product', async () => {
			const related = actor => act('GetRelatedTenants', `query { GetRelatedTenants(input: { tenantId: ${str(a.tenantId)} }) { relatedTenants { tenantId sharingContractIds } errorMessage } }`, actor);
			const rows = await Promise.all([related(aP), related(aQ), related(aPuma)]);
			assert.deepEqual(rows.map(r => (r.relatedTenants || []).some(t => t.tenantId === b.tenantId && t.sharingContractIds.includes(id))), [true, false, false]);
		});
		const sentQ = await act('SubmitContract', `mutation { SubmitContract(input: { tenantId: ${str(a.tenantId)}, contractId: ${str(cq.contractId)}, revision: 1 }) { success errorMessage } }`, aQ);
		assert.ok(sentQ.success, sentQ.errorMessage);
		for (const referenceType of ['Framework', 'Supersedes']) {
			await test(referenceType + ' reference cannot cross the product boundary', async () => {
				const result = await act('CreateContract', `mutation { CreateContract(input: { tenantId: ${str(a.tenantId)}, draft: { ${draft}, references: [{ contractId: ${str(cq.contractId)}, referenceType: ${referenceType} }] } }) { success errorMessage } }`, aP);
				assert.equal(result.success, false);
			});
		}
		await test('Contract numbers remain shared by the organization', async () => {
			const other = await act('GetContract', `query { GetContract(input: { tenantId: ${str(a.tenantId)}, contractId: ${str(cq.contractId)} }) { contract { number } } }`, aQ);
			assert.ok(withAllocation.contract.number);
			assert.ok(other.contract.number);
			assert.notEqual(withAllocation.contract.number, other.contract.number);
		});
		await test('An anonymous product header cannot receive server cache events', async () => {
			let watch;
			try {
				watch = await subscribe({ product: p.product }, 'OnContractsChanged');
			} catch (error) {
				assert.match(error.message, /refus|not allowed|forbidden|authorization|denied/i);
				return;
			}
			const mark = watch.socket.mark();
			const changed = await act('SetContractPrivateInfo', `mutation { SetContractPrivateInfo(input: { ${input}, notes: "Check server identity" }) { success errorMessage } }`, aP);
			assert.ok(changed.success, changed.errorMessage);
			await lib.delay(700);
			assert.equal(events(watch, mark, id).length, 0);
		});
	} finally {
		for (const socket of sockets) { socket.close(); }
		for (const product of products) {
			if (product.credentialId) {
				await act('RevokeServerToken', `mutation { RevokeServerToken(input: { id: ${str(product.credentialId)} }) { success } }`, su);
				await act('DeleteServerToken', `mutation { DeleteServerToken(input: { id: ${str(product.credentialId)} }) { success } }`, su);
			}
		}
		if (tenants.length) {
			await act('RemoveElements', `mutation { RemoveElements(input: { collectionId: "Tenants", elementIds: ${JSON.stringify(tenants)} }) { success } }`, su);
		}
	}
});
