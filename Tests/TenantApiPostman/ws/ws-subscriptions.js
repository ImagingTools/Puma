// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
//
// WebSocket subscription suite for the Tenant API.
//
// Newman does not execute Postman WebSocket requests - it only runs HTTP ones -
// so the WS side of the tenant API cannot live in Tenant_System_Full.postman_collection.json.
// This runner covers it with the same shape as the Postman suite: same environment
// file, same base_url/credentials, JUnit XML for the same TeamCity report step.
//
// Protocol (see ImtCore CWebSocketServletComp / Qml/imtguigql/SubscriptionManager.qml):
//   -> {"type":"connection_init"}                                   <- {"type":"connection_ack"}
//   -> {"id":..,"type":"start","headers":{..},"payload":{"data":Q}} <- (silence on success)
//                                                                   <- {"type":"data","id":..,"payload":{..}}
//   -> {"id":..,"type":"stop"}                                      <- {"type":"complete","id":..}
//   errors arrive as {"id":..,"type":"error","payload":[{message,extensions:{type}}]}
//   the server also pushes unsolicited {"type":"ka"} keep-alives
//
// Every assertion here fails on a mismatch rather than on absence alone: refusals
// are checked against the reason the server actually gives, notifications against
// their documentId and operation, and every inbound frame against the accounting
// in SubscriptionSession.audit() - an error, a data frame or a complete for an id
// the suite did not expect fails the run even when no single case asked for it.

'use strict';

const crypto = require('crypto');
const {
	httpUrl, wsUrl, suLogin, suPassword, frameTimeoutMs, registrationGraceMs,
	gqlString, gql, mutate, login,
	SubscriptionSession, notificationOf,
	assert, assertRefusalReason, assertRoutingRefusal, documentNotification,
	test, requires, runSuite
} = require('./ws-lib');

// ---------------------------------------------------------------- the suite

async function main() {
	console.log(`HTTP : ${httpUrl}`);
	console.log(`WS   : ${wsUrl}`);

	// Mirrors "00 System bootstrap" so the suite also runs standalone against a
	// freshly reset database, not only after newman has already seeded it.
	await gql(`mutation CreateSuperuser { CreateSuperuser(input: { password: ${gqlString(suPassword)},` +
		' mail: "su@imagingtools.local", name: "Super User" }) { success message } }').catch(() => {});

	const su = await login(suLogin, suPassword);
	console.log(`Authenticated as '${suLogin}' (userId=${su.userId})\n`);

	const session = new SubscriptionSession('main', wsUrl, su.token);
	await session.connect();

	await test('connection_init is acknowledged', async () => {
		const frame = await session.init();
		assert(frame.type === 'connection_ack', `expected connection_ack, got ${JSON.stringify(frame).slice(0, 200)}`);
		assert(typeof frame.payload.connectionTimeoutMs === 'number' && frame.payload.connectionTimeoutMs > 0,
			`connection_ack must carry a positive connectionTimeoutMs, got ${JSON.stringify(frame.payload)}`);
	});

	// --- document-service subscriptions (the ones the tenant editor depends on)

	const collectionQuery = (op, coll) =>
		`subscription ${op} { ${op}(input: { __typename: "CollectionId", collectionId: ${gqlString(coll)} })` +
		(op === 'OnUndoRedoChanged'
			? ' { documentId isDirty availableUndoSteps availableRedoSteps } }'
			: ' { documentOperation documentId documentName objectId isDirty isLoading } }');

	let managerSubId = null;
	let undoSubId = null;

	await test('OnDocumentManagerChanged accepts a subscription', async () => {
		managerSubId = await session.subscribe('OnDocumentManagerChanged',
			collectionQuery('OnDocumentManagerChanged', 'Tenants'));
		assert(managerSubId, 'no subscription id');
	});

	await test('OnUndoRedoChanged accepts a subscription', async () => {
		undoSubId = await session.subscribe('OnUndoRedoChanged', collectionQuery('OnUndoRedoChanged', 'Tenants'));
		assert(undoSubId, 'no subscription id');
	});

	// Creating a tenant draft must reach subscribers of the collection.
	// A UUID, not a readable label: the Tenants table's Id column is uuid, so
	// anything else is rejected by Postgres on insert - and the document service
	// reports that as a bare "Failed to save document".
	const seedId = crypto.randomUUID();
	// Unique per run: a fixed name makes a second run against the same database fail
	// on "Tenant with the same name already exists", which reads like a subscription
	// fault and is not one.
	const tenantName = `Tenant_WS_Probe_${Date.now()}`;
	let documentId = null;
	// Saving under a *different* name means "save as" to DoSaveDocument, so the
	// current name is tracked as the server reports it back.
	let documentName = null;

	await test('CreateNewDocument notifies OnDocumentManagerChanged', async () => {
		requires(managerSubId, 'a live OnDocumentManagerChanged subscription');

		const info = await mutate('CreateNewDocument',
			'mutation CreateNewDocument { CreateNewDocument(input: { collectionId: "Tenants", typeId: "Tenant",' +
			` proposedSourceDocumentId: ${gqlString(seedId)} }) { documentId objectId objectTypeId documentName } }`,
			su.token);
		assert(info.documentId, `CreateNewDocument returned no documentId: ${JSON.stringify(info).slice(0, 300)}`);
		documentId = info.documentId;
		documentName = info.documentName;

		// Not "some notification arrived": it has to be this document's creation.
		const frame = await session.waitForData(managerSubId, f => {
			const notification = notificationOf(f, 'OnDocumentManagerChanged');
			return !!notification &&
				notification.documentId === documentId &&
				notification.documentOperation === 'NewDocumentCreated';
		});

		const notification = notificationOf(frame, 'OnDocumentManagerChanged');
		assert(notification.documentOperation === 'NewDocumentCreated',
			`expected documentOperation "NewDocumentCreated", got "${notification.documentOperation}"`);
		assert(notification.documentId === documentId,
			`notification is for document "${notification.documentId}", not "${documentId}"`);
	});

	let docSubId = null;

	await test('OnDocumentChanged accepts a subscription for the new document', async () => {
		requires(documentId, 'a created document id');

		docSubId = await session.subscribe('OnDocumentChanged',
			'subscription OnDocumentChanged { OnDocumentChanged(input: { __typename: "DocumentId",' +
			` collectionId: "Tenants", id: ${gqlString(documentId)} })` +
			' { documentOperation documentId documentName objectId isDirty isLoading } }');
		assert(docSubId, 'no subscription id');
	});

	await test('UpdateTenantFromRepresentation makes the document dirty over WS', async () => {
		requires(documentId, 'a created document id');
		requires(undoSubId, 'a live OnUndoRedoChanged subscription');
		requires(docSubId, 'a live OnDocumentChanged subscription');

		// A never-saved document is already dirty on creation, so only frames the edit caused count.
		const editMark = session.mark();

		const status = await mutate('UpdateTenantFromRepresentation',
			'mutation UpdateTenantFromRepresentation { UpdateTenantFromRepresentation(input: {' +
			` documentId: ${gqlString(documentId)},` +
			` tenant: { id: ${gqlString(seedId)}, name: ${gqlString(tenantName)},` +
			' description: "created by ws-subscriptions.js",' +
			// The owner has to be among the members, as in "01A Tenant document-service
			// create/update" - the document validator rejects a tenant without it and
			// the later SaveDocument then fails with a bare "Failed to save document".
			` ownerId: ${gqlString(su.userId)},` +
			` isActive: true, members: [{ id: ${gqlString(su.userId)}, name: ${gqlString(suLogin)} }],` +
			' memberOrganizationPermissions: [], tenantPermissions: [] } })' +
			' { status message documentName } }', su.token);

		assert(status.status === 'Success',
			`UpdateTenantFromRepresentation reported ${JSON.stringify(status).slice(0, 300)}`);

		if (status.documentName) { documentName = status.documentName; }

		const frame = await session.waitForData(undoSubId, f => {
			const notification = notificationOf(f, 'OnUndoRedoChanged');
			return !!notification && notification.documentId === documentId && notification.isDirty === true;
		}, editMark);

		const notification = notificationOf(frame, 'OnUndoRedoChanged');
		assert(notification.availableUndoSteps >= 1,
			`an edit must leave at least one undo step, got ${JSON.stringify(notification).slice(0, 250)}`);

		// The same edit has to reach the document-scoped subscription too.
		await session.waitForData(docSubId, f => documentNotification(f, 'OnDocumentChanged', documentId), editMark);
	});

	// Regression probe for "Save stays enabled after creating a tenant": the last
	// dirty state the client can observe after SaveDocument has to be false.
	await test('SaveDocument leaves the document clean (Save button regression)', async () => {
		requires(documentId, 'a created document id');
		requires(undoSubId, 'a live OnUndoRedoChanged subscription');
		requires(managerSubId, 'a live OnDocumentManagerChanged subscription');

		const before = session.dataFrames(undoSubId).length;
		const managerBefore = session.dataFrames(managerSubId).length;

		// Save under the name the document already carries. DoSaveDocument treats a
		// *different* name as "save as" and inserts a copy instead of updating - which
		// is what the GUI's Save button must not do, and what made this fail with a
		// bare "Failed to save document".
		const currentName = documentName;
		assert(typeof currentName === 'string',
			`no document name was reported by CreateNewDocument/UpdateTenantFromRepresentation: ${currentName}`);

		const status = await mutate('SaveDocument',
			'mutation SaveDocument { SaveDocument(input: { collectionId: "Tenants",' +
			` documentId: ${gqlString(documentId)}, documentName: ${gqlString(currentName)} })` +
			' { status message documentName } }', su.token);

		assert(status.status === 'Success', `SaveDocument reported ${JSON.stringify(status).slice(0, 300)}`);
		assert(status.documentName === tenantName,
			`SaveDocument persisted the document as "${status.documentName}" instead of the edited name ` +
			`"${tenantName}" - that is a "save as" copy rather than an update`);
		documentName = status.documentName;

		// Give the server the same grace the GUI has before it reads the button state.
		await new Promise(r => setTimeout(r, 2000));

		const frames = session.dataFrames(undoSubId).slice(before);
		assert(frames.length > 0, 'SaveDocument produced no OnUndoRedoChanged notification at all');

		const dirtyStates = frames
			.map(f => notificationOf(f, 'OnUndoRedoChanged'))
			.filter(n => !!n && n.documentId === documentId)
			.map(n => n.isDirty);

		assert(dirtyStates.length > 0,
			`SaveDocument produced OnUndoRedoChanged frames, but none for document "${documentId}"`);
		assert(dirtyStates.every(state => typeof state === 'boolean'),
			`OnUndoRedoChanged reported a non-boolean isDirty: ${JSON.stringify(dirtyStates)}`);
		assert(dirtyStates[dirtyStates.length - 1] === false,
			'after SaveDocument the last OnUndoRedoChanged still reports isDirty=true ' +
			'(this is what keeps the Save button enabled): ' + JSON.stringify(dirtyStates));

		// The save itself must also be announced on the collection, with the id, the
		// operation and the object it bound to - the manager view redraws off exactly this.
		const saved = session.dataFrames(managerSubId).slice(managerBefore)
			.map(f => notificationOf(f, 'OnDocumentManagerChanged'))
			.filter(n => !!n && n.documentId === documentId);

		assert(saved.some(n => n.documentOperation === 'DocumentSaved'),
			'no OnDocumentManagerChanged notification with documentOperation="DocumentSaved" for this document: ' +
			JSON.stringify(saved.map(n => n.documentOperation)));
		assert(saved.some(n => n.documentOperation === 'DocumentSaved' && !!n.objectId),
			'the DocumentSaved notification carries no objectId, so the view cannot bind the saved row: ' +
			JSON.stringify(saved.filter(n => n.documentOperation === 'DocumentSaved')).slice(0, 300));
	});

	// --- tenant notification subscriptions
	//
	// One case per subscription this server serves, in the shape the QML client
	// sends it: empty input, "notification { id }" selection (taken from a live
	// trace of a client start - the client does not pass a userId, the server
	// takes the identity from the request context).
	// Not listed, deliberately: PumaWsConnection is the ProLife/Lisa engines'
	// probe *towards* Puma, so it belongs to the ProLife suite, and
	// OnConnectionNotification is declared in Tenants.sdl but published by
	// nothing - the client uses OnConnectionCodesNotification instead.
	//
	// These assert registration only - nothing here triggers the events that would
	// make them deliver. Registered as one batch sharing a single grace window:
	// waiting per subscription cost more than the rest of the suite put together.
	const clientSubscriptions = [
		'OnMembershipNotification',
		'OnCrossTenantMessageNotification',
		'OnConnectionCodesNotification',
		'OnUsersCollectionChanged',
		'OnGroupsCollectionChanged',
		'OnRolesCollectionChanged',
		'OnUserSessionsCollectionChanged',
		'OnTenantsCollectionChanged'
	];

	const clientQuery = (commandId) =>
		`subscription ${commandId} { ${commandId}(input: {}) { notification { id } } }`;

	const clientSubIds = new Map();
	for (const commandId of clientSubscriptions) {
		clientSubIds.set(commandId, session.beginSubscribe(commandId, clientQuery(commandId)));
	}
	await new Promise(r => setTimeout(r, registrationGraceMs));

	for (const commandId of clientSubscriptions) {
		await test(`the client subscription '${commandId}' is accepted`, async () => {
			const id = session.confirmAccepted(clientSubIds.get(commandId));
			const frame = await session.stop(id);
			assert(frame.type === 'complete',
				`stopping an accepted subscription answered ${JSON.stringify(frame).slice(0, 250)}`);
			assert(frame.id === id, `complete carries id "${frame.id}" instead of "${id}"`);
		});
	}

	// --- routing contract the ProLife -> Puma bridge depends on
	//
	// The document-service subscriptions are one command per operation and name
	// their collection in "input". A forwarder that passes the command on without
	// the arguments, or that decides on the command alone, cannot be told apart
	// from a correct one unless the server is strict about both. Each case checks
	// the refusal *reason*, so a case that stops reaching the routing check - a
	// broken token, a malformed payload - fails instead of passing on any error.

	await test('OnDocumentManagerChanged without input is refused', async () => {
		const frame = await session.subscribeExpectingRefusal('OnDocumentManagerChanged',
			'subscription OnDocumentManagerChanged { OnDocumentManagerChanged { documentOperation documentId } }');

		assertRoutingRefusal(frame, 'OnDocumentManagerChanged',
			'a document-service subscription carrying no input was accepted - a forwarder that drops ' +
			'the arguments would look healthy while delivering nothing');
	});

	await test('OnUndoRedoChanged without input is refused', async () => {
		const frame = await session.subscribeExpectingRefusal('OnUndoRedoChanged',
			'subscription OnUndoRedoChanged { OnUndoRedoChanged { documentId isDirty } }');

		assertRoutingRefusal(frame, 'OnUndoRedoChanged', 'a subscription carrying no input was accepted');
	});

	await test('OnDocumentManagerChanged for an unknown collection is refused', async () => {
		const frame = await session.subscribeExpectingRefusal('OnDocumentManagerChanged',
			'subscription OnDocumentManagerChanged { OnDocumentManagerChanged(input: { collectionId: "NoSuchCollection" })' +
			' { documentOperation documentId } }');

		assertRoutingRefusal(frame, 'OnDocumentManagerChanged',
			'a subscription for a collection this server does not serve was accepted - collection scoping is not enforced');
	});

	await test('OnDocumentChanged without a document id is refused', async () => {
		const frame = await session.subscribeExpectingRefusal('OnDocumentChanged',
			'subscription OnDocumentChanged { OnDocumentChanged(input: { __typename: "DocumentId", collectionId: "Tenants" })' +
			' { documentOperation documentId } }');

		assertRoutingRefusal(frame, 'OnDocumentChanged',
			'a document-scoped subscription without a document id was accepted');
	});

	// --- protocol hygiene

	await test('start with an empty subscription id is refused', async () => {
		// Everything on both sides is keyed on this id: the publisher map, data
		// frames, errors and stop. Accepting an empty one makes every subscription
		// that has it collide on a single key and receive each other's messages.
		//
		// The payload is the well-formed one every other case uses. Sending raw
		// GraphQL here instead - as this case used to - is refused by
		// CGqlRequest::ParseQuery before the id is ever looked at, so the case
		// passes without the server having checked anything about the id.
		session.expectErrorIds.add('');
		session.expectCompleteIds.add('');
		session.send(session.startFrame('', collectionQuery('OnDocumentManagerChanged', 'Tenants')));

		const refusal = await session.settle('');
		if (!refusal) {
			session.send(session.stopFrame(''));
		}

		assertRefusalReason(refusal, {
			absent: 'the server bound a subscription under an empty id',
			mustMatch: /.+/,
			mustNotMatch: [/^Unauthorized:/, /^Forbidden:/]
		});
	});

	await test('stop of a live subscription is completed and ends delivery', async () => {
		requires(docSubId, 'a live OnDocumentChanged subscription');
		requires(documentId, 'a created document id');
		requires(managerSubId, 'a live OnDocumentManagerChanged subscription');

		const frame = await session.stop(docSubId);
		assert(frame.type === 'complete', `expected "complete", got ${JSON.stringify(frame).slice(0, 300)}`);
		assert(frame.id === docSubId, `complete carries id "${frame.id}" instead of "${docSubId}"`);

		// "complete" is only half the contract: a stopped subscription must also stop
		// receiving. Left registered, it keeps a destroyed view's request alive.
		const stoppedAt = session.dataFrames(docSubId).length;

		// A stored organization is edited only from inside it.
		const selected = await mutate('SelectTenant',
			`mutation SelectTenant { SelectTenant(input: { tenantId: ${gqlString(seedId)} }) { ok userSession { accessToken } errorMessage } }`,
			su.token);
		const tenantToken = selected.userSession && selected.userSession.accessToken;
		assert(tenantToken, `SelectTenant gave no token: ${JSON.stringify(selected).slice(0, 300)}`);

		const managerMark = session.mark();

		await mutate('UpdateTenantFromRepresentation',
			'mutation UpdateTenantFromRepresentation { UpdateTenantFromRepresentation(input: {' +
			` documentId: ${gqlString(documentId)},` +
			` tenant: { id: ${gqlString(seedId)}, name: ${gqlString(tenantName)}, description: "edit after stop",` +
			` ownerId: ${gqlString(su.userId)},` +
			` isActive: true, members: [{ id: ${gqlString(su.userId)}, name: ${gqlString(suLogin)} }],` +
			' memberOrganizationPermissions: [], tenantPermissions: [] } })' +
			' { status message } }', tenantToken);

		// The still-live collection subscription proves the edit did reach the
		// publisher, so the absence below is a real unregistration and not a
		// server that simply had nothing to say.
		await session.waitFor(f => f.type === 'data' && f.id === managerSubId &&
			documentNotification(f, 'OnDocumentManagerChanged', documentId), frameTimeoutMs, managerMark);
		await new Promise(r => setTimeout(r, 1000));

		const leaked = session.dataFrames(docSubId).slice(stoppedAt);
		assert(leaked.length === 0,
			`${leaked.length} notification(s) were still delivered to a stopped subscription: ` +
			JSON.stringify(leaked.map(f => notificationOf(f, 'OnDocumentChanged'))).slice(0, 300));
	});

	await test('stop of an unknown subscription is idempotent and carries its id', async () => {
		const unknownId = crypto.randomUUID();
		const mark = session.mark();
		session.expectCompleteIds.add(unknownId);
		session.expectErrorIds.add(unknownId);
		session.send(session.stopFrame(unknownId));
		const frame = await session.waitFor(f => (f.type === 'error' || f.type === 'complete') && f.id === unknownId,
			frameTimeoutMs, mark);

		// Unregistering what the server no longer holds is the caller's intent, so
		// it answers "complete". Whatever it answers has to carry the id: the client
		// attributes frames to subscriptions by id and cannot act on one without.
		assert(frame.id === unknownId, `frame carries id="${frame.id}" instead of "${unknownId}"`);
		assert(frame.type === 'complete',
			`expected an idempotent "complete", got ${JSON.stringify(frame).slice(0, 300)}`);
	});

	await test('double stop does not desynchronise the session', async () => {
		requires(managerSubId, 'a live OnDocumentManagerChanged subscription');

		const first = await session.stop(managerSubId);
		assert(first.type === 'complete', `first stop answered ${JSON.stringify(first).slice(0, 250)}`);

		// Scoped to this id and to what arrives after this point. An unscoped wait is
		// satisfied instantly by any earlier "complete" and asserts nothing.
		const mark = session.mark();
		session.send(session.stopFrame(managerSubId));
		const second = await session.waitFor(f => f.id === managerSubId && (f.type === 'complete' || f.type === 'error'),
			frameTimeoutMs, mark);
		assert(second.type === 'complete',
			`a repeated stop answered ${JSON.stringify(second).slice(0, 250)} instead of an idempotent "complete"`);

		// The session must still serve new subscriptions afterwards.
		const probeId = await session.subscribe('OnDocumentManagerChanged',
			collectionQuery('OnDocumentManagerChanged', 'Tenants'));
		const closing = await session.stop(probeId);
		assert(closing.type === 'complete',
			`the session no longer serves subscriptions after a double stop: ${JSON.stringify(closing).slice(0, 250)}`);
	});

	const rogue = new SubscriptionSession('rogue', wsUrl, 'not-a-valid-token');

	await test('a subscription registered with an invalid token is refused as an auth failure', async () => {
		await rogue.connect();
		await rogue.init();

		const frame = await rogue.subscribeExpectingRefusal('OnDocumentManagerChanged',
			collectionQuery('OnDocumentManagerChanged', 'Tenants'));

		// The class of refusal matters: a bogus credential has to come back as an
		// authentication/authorization failure. Anything else - "no servlet found",
		// an internal error - means the token was never the thing that stopped it.
		assertRefusalReason(frame, {
			absent: 'the server accepted a subscription presenting an invalid access token',
			mustMatch: /^(Unauthorized|Forbidden): /,
			mustNotMatch: [/No servlet was found/]
		});
	});

	// Everything SubscriptionSession.audit() collected along the way: frames of an
	// unknown type or shape, errors nobody asked for, notifications delivered to an
	// id that holds no subscription, payloads carrying another command's data. No
	// case above looks for these, which is the point - this is what catches a
	// mismatch no assertion was written for.
	await test('no unaccounted frames or protocol violations on any session', async () => {
		const violations = session.violations.concat(rogue.violations);
		assert(violations.length === 0,
			`${violations.length} protocol violation(s):\n        - ` + violations.join('\n        - '));
	});

	rogue.close();

	// --- cleanup

	if (documentId) {
		await gql('mutation CloseDocument { CloseDocument(input: { collectionId: "Tenants",' +
			` id: ${gqlString(documentId)} }) { status message } }`, su.token).catch(() => {});
	}

	session.close();
}


runSuite({ name: 'Tenant API WebSocket subscriptions', className: 'TenantApi.WebSocket', junitFile: 'junit-report-ws.xml' }, main);
