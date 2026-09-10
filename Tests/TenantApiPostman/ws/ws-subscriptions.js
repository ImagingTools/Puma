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

const fs = require('fs');
const path = require('path');
const http = require('http');
const https = require('https');
const crypto = require('crypto');
const WebSocket = require('ws');

// ---------------------------------------------------------------- configuration

function parseArgs(argv) {
	const out = {};
	for (let i = 2; i < argv.length; i++) {
		const a = argv[i];
		if (a.startsWith('--')) {
			const key = a.slice(2);
			const next = argv[i + 1];
			if (next && !next.startsWith('--')) { out[key] = next; i++; }
			else { out[key] = true; }
		}
	}
	return out;
}

const args = parseArgs(process.argv);

const envPath = args.env || path.join(__dirname, '..', 'Tenant_System_Full.postman_environment.json');
const envValues = {};
if (fs.existsSync(envPath)) {
	const parsed = JSON.parse(fs.readFileSync(envPath, 'utf8'));
	for (const entry of (parsed.values || [])) {
		envValues[entry.key] = entry.value;
	}
}

const httpUrl = args.http || process.env.WS_TEST_HTTP_URL || envValues.base_url || 'http://localhost:17788/Puma/graphql';
const wsUrl = args.ws || process.env.WS_TEST_WS_URL || deriveWsUrl(httpUrl);
const suLogin = args['su-login'] || envValues.su_login || 'su';
const suPassword = args['su-password'] || envValues.su_password || '';
const productId = args['product-id'] || envValues.product_id || '';
const junitPath = args.junit || path.join(__dirname, '..', 'junit-report-ws.xml');
const frameTimeoutMs = Number(args.timeout || 15000);
// How long a refusal has to arrive before a registration counts as accepted.
const registrationGraceMs = Number(args['grace'] || 2500);
// How long a notification already in flight may still land after its "complete".
const drainGraceMs = 750;

// The WS listener is a separate port from HTTP (PumaServerPgTest.acc: HttpPort 17788,
// WebSocketPort 18788). Derive it by convention, override with --ws when it differs.
function deriveWsUrl(httpEndpoint) {
	const u = new URL(httpEndpoint);
	const httpPort = Number(u.port || (u.protocol === 'https:' ? 443 : 80));
	const wsPort = process.env.WS_TEST_WS_PORT || (httpPort + 1000);
	const scheme = u.protocol === 'https:' ? 'wss:' : 'ws:';
	return `${scheme}//${u.hostname}:${wsPort}`;
}

// ---------------------------------------------------------------- http graphql

// Values reach a query through JSON.stringify rather than bare interpolation: a
// quote in a document name the server handed back would otherwise produce a
// syntactically broken query and a failure that reads like a protocol fault.
function gqlString(value) {
	return JSON.stringify(String(value));
}

function gql(query, token) {
	return new Promise((resolve, reject) => {
		const u = new URL(httpUrl);
		const body = JSON.stringify({ query });
		const headers = {
			'Content-Type': 'application/json',
			'Content-Length': Buffer.byteLength(body)
		};
		if (token) { headers['x-authentication-token'] = token; }
		if (productId) { headers['productId'] = productId; }

		const lib = u.protocol === 'https:' ? https : http;
		const req = lib.request({
			hostname: u.hostname,
			port: u.port,
			path: u.pathname + u.search,
			method: 'POST',
			headers
		}, (res) => {
			let data = '';
			res.on('data', (chunk) => { data += chunk; });
			res.on('end', () => {
				try { resolve(JSON.parse(data)); }
				catch (e) { reject(new Error(`Non-JSON response (${res.statusCode}): ${data.slice(0, 300)}`)); }
			});
		});
		req.on('error', reject);
		req.write(body);
		req.end();
	});
}

// Fails on a GraphQL "errors" array too - a mutation that did not happen must not
// be read as a missing notification three assertions later.
async function mutate(label, query, token) {
	const res = await gql(query, token);
	if (res && res.errors) {
		throw new Error(`${label} returned GraphQL errors: ${JSON.stringify(res.errors).slice(0, 400)}`);
	}
	const payload = res && res.data && res.data[label];
	if (!payload) {
		throw new Error(`${label} returned no data: ${JSON.stringify(res).slice(0, 400)}`);
	}
	return payload;
}

async function login(loginName, password) {
	const res = await gql(
		`query Authorization { Authorization(input: { login: ${gqlString(loginName)}, password: ${gqlString(password)},` +
		` productId: ${gqlString(productId)} })` +
		' { userId username token permissions } }');
	const auth = res && res.data && res.data.Authorization;
	if (!auth || !auth.token) {
		throw new Error(`Authorization failed for '${loginName}': ${JSON.stringify(res).slice(0, 300)}`);
	}
	return auth;
}

// ---------------------------------------------------------------- ws client

// Types the client (Qml/imtguigql/SubscriptionClient.qml) and CWebSocketServletComp
// between them can produce. Anything else on the wire is a protocol violation.
const KNOWN_FRAME_TYPES = new Set(['connection_ack', 'data', 'error', 'complete', 'ka', 'pong', 'start_ack']);

class SubscriptionSession {
	constructor(label, url, token) {
		this.label = label;
		this.url = url;
		this.token = token;
		this.frames = [];
		this.waiters = [];
		this.socket = null;

		// Frame accounting. Every data/error/complete frame has to be attributable to
		// something the suite asked for; whatever is not is collected in violations
		// and fails the run in the final case.
		this.violations = [];
		this.activeIds = new Map();      // id -> command id its data frames must carry
		this.retiredIds = new Map();     // id -> { commandId, at } once its complete arrived
		this.expectErrorIds = new Set();
		this.expectCompleteIds = new Set();
	}

	connect() {
		return new Promise((resolve, reject) => {
			const socket = new WebSocket(this.url);
			this.socket = socket;

			const failTimer = setTimeout(() => reject(new Error(`WS connect timeout: ${this.url}`)), frameTimeoutMs);

			socket.on('open', () => { clearTimeout(failTimer); resolve(); });
			socket.on('error', (err) => { clearTimeout(failTimer); reject(err); });
			socket.on('message', (raw) => this.onFrame(raw.toString()));
		});
	}

	violation(text) {
		this.violations.push(`[${this.label}] ${text}`);
	}

	onFrame(raw) {
		let frame;
		try { frame = JSON.parse(raw); }
		catch (e) {
			this.violation(`unparsable frame: ${raw.slice(0, 200)}`);
			frame = { type: 'unparsable', raw };
		}

		this.frames.push(frame);
		this.audit(frame);

		for (let i = this.waiters.length - 1; i >= 0; i--) {
			const waiter = this.waiters[i];
			if (waiter.predicate(frame)) {
				this.waiters.splice(i, 1);
				clearTimeout(waiter.timer);
				waiter.resolve(frame);
			}
		}
	}

	// Shape and attribution checks applied to every inbound frame.
	audit(frame) {
		if (frame.type === 'unparsable') { return; }

		if (!KNOWN_FRAME_TYPES.has(frame.type)) {
			this.violation(`unknown frame type "${frame.type}": ${JSON.stringify(frame).slice(0, 200)}`);
			return;
		}

		if (frame.type === 'connection_ack') {
			const timeout = frame.payload && frame.payload.connectionTimeoutMs;
			if (typeof timeout !== 'number' || timeout <= 0) {
				this.violation(`connection_ack without a usable connectionTimeoutMs: ${JSON.stringify(frame).slice(0, 200)}`);
			}
			return;
		}

		if (frame.type === 'ka' || frame.type === 'pong') { return; }

		if (typeof frame.id !== 'string') {
			this.violation(`"${frame.type}" frame carries no string id: ${JSON.stringify(frame).slice(0, 200)}`);
			return;
		}

		if (frame.type === 'error') {
			this.auditErrorShape(frame);
			if (!this.expectErrorIds.has(frame.id)) {
				this.violation(`unsolicited error for id "${frame.id}": ${JSON.stringify(frame.payload).slice(0, 250)}`);
			}
			return;
		}

		if (frame.type === 'complete') {
			if (!this.activeIds.has(frame.id) && !this.retiredIds.has(frame.id) && !this.expectCompleteIds.has(frame.id)) {
				this.violation(`complete for an id the suite never registered: "${frame.id}"`);
			}
			return;
		}

		const expectedCommand = this.activeIds.has(frame.id)
			? this.activeIds.get(frame.id)
			: this.retiredCommandWithinGrace(frame.id);

		if (expectedCommand === undefined) {
			this.violation(`data frame for id "${frame.id}", which holds no live subscription: ` +
				JSON.stringify(frame.payload).slice(0, 250));
			return;
		}

		const data = frame.payload && frame.payload.data;
		if (!data || typeof data !== 'object') {
			this.violation(`data frame for "${frame.id}" has no payload.data object: ${JSON.stringify(frame).slice(0, 250)}`);
			return;
		}
		if (!Object.prototype.hasOwnProperty.call(data, expectedCommand)) {
			this.violation(`data frame for "${frame.id}" carries ${JSON.stringify(Object.keys(data))} ` +
				`instead of the subscribed command "${expectedCommand}"`);
		}
	}

	auditErrorShape(frame) {
		const payload = frame.payload;
		if (!Array.isArray(payload) || payload.length === 0) {
			this.violation(`error frame payload is not a non-empty array: ${JSON.stringify(frame).slice(0, 250)}`);
			return;
		}
		for (const entry of payload) {
			if (!entry || typeof entry.message !== 'string' || entry.message.length === 0) {
				this.violation(`error entry without a message: ${JSON.stringify(entry).slice(0, 250)}`);
			}
			if (!entry || !entry.extensions || typeof entry.extensions.type !== 'string') {
				this.violation(`error entry without extensions.type: ${JSON.stringify(entry).slice(0, 250)}`);
			}
		}
	}

	retiredCommandWithinGrace(id) {
		const retired = this.retiredIds.get(id);
		if (retired === undefined) { return undefined; }
		if (Date.now() - retired.at > drainGraceMs) { return undefined; }
		return retired.commandId;
	}

	// Index into frames, so a wait can be scoped to what arrives after this point.
	// Without it a predicate matching on type alone is satisfied by a frame from an
	// earlier case and the wait proves nothing.
	mark() {
		return this.frames.length;
	}

	waitFor(predicate, timeoutMs, fromIndex) {
		const start = fromIndex === undefined ? 0 : fromIndex;
		const existing = this.frames.slice(start).find(predicate);
		if (existing) { return Promise.resolve(existing); }

		return new Promise((resolve, reject) => {
			const waiter = { predicate, resolve };
			waiter.timer = setTimeout(() => {
				const index = this.waiters.indexOf(waiter);
				if (index >= 0) { this.waiters.splice(index, 1); }
				reject(new Error(`Timed out after ${timeoutMs}ms waiting for a matching frame. Seen since mark: ` +
					JSON.stringify(this.frames.slice(start).map(f => ({ type: f.type, id: f.id }))).slice(0, 500)));
			}, timeoutMs);
			this.waiters.push(waiter);
		});
	}

	send(obj) {
		this.socket.send(JSON.stringify(obj));
	}

	async init() {
		const mark = this.mark();
		this.send({ type: 'connection_init' });
		return this.waitFor(f => f.type === 'connection_ack', frameTimeoutMs, mark);
	}

	// Frames are built exactly as Qml/imtguigql/SubscriptionManager.qml builds them
	// for a Qml/imtguigql/SubscriptionClient.qml. Two details are easy to get wrong
	// and both are load-bearing: the payload carries the query as a JSON *string*
	// ({"query": "subscription ..."}) rather than raw GraphQL, and the access token
	// and productId travel in the frame's own headers, not the socket's. Any other
	// shape tests a protocol the client does not speak - and a malformed payload is
	// refused by CGqlRequest::ParseQuery before the case under test is ever reached,
	// which turns a negative case into a pass for the wrong reason.
	headers() {
		const headers = {};
		if (this.token) { headers['x-authentication-token'] = this.token; }
		if (productId) { headers['productId'] = productId; }
		return headers;
	}

	startFrame(id, query) {
		return { id, headers: this.headers(), type: 'start', payload: { data: JSON.stringify({ query }) } };
	}

	stopFrame(id) {
		return { id, headers: this.headers(), type: 'stop', payload: {} };
	}

	// A successful "start" is answered with silence: nothing in ImtCore ever emits
	// start_ack (CWebSocketRequest only parses one), and CWebSocketServletComp
	// returns an empty response on success. So a refusal is the only registration
	// outcome that produces a frame, and the absence of one is the acceptance.
	settle(id, waitMs) {
		const isOutcome = (f) => f.id === id && (f.type === 'error' || f.type === 'start_ack');
		const existing = this.frames.find(isOutcome);
		if (existing) { return Promise.resolve(existing.type === 'error' ? existing : null); }

		return new Promise((resolve) => {
			const waiter = { predicate: isOutcome, resolve: (frame) => resolve(frame.type === 'error' ? frame : null) };
			waiter.timer = setTimeout(() => {
				const index = this.waiters.indexOf(waiter);
				if (index >= 0) { this.waiters.splice(index, 1); }
				resolve(null);
			}, waitMs === undefined ? registrationGraceMs : waitMs);
			this.waiters.push(waiter);
		});
	}

	// Registers without waiting, so a batch can share one grace window instead of
	// paying it once per subscription.
	beginSubscribe(commandId, query) {
		const id = crypto.randomUUID();
		this.activeIds.set(id, commandId);
		this.expectErrorIds.add(id);
		this.send(this.startFrame(id, query));
		return id;
	}

	// Turns an id whose grace window has passed into an accepted subscription, or
	// throws with the refusal the server gave.
	confirmAccepted(id) {
		const refusal = this.frames.find(f => f.id === id && f.type === 'error');
		if (refusal) {
			this.activeIds.delete(id);
			throw new Error(`Subscription refused: ${JSON.stringify(refusal.payload).slice(0, 400)}`);
		}
		this.expectErrorIds.delete(id);
		return id;
	}

	async subscribe(commandId, query) {
		const id = this.beginSubscribe(commandId, query);
		await this.settle(id);
		return this.confirmAccepted(id);
	}

	// Registers and requires the server to refuse. Returns the error frame so the
	// caller can assert on the reason; returns null when the server accepted it.
	async subscribeExpectingRefusal(commandId, query) {
		const id = crypto.randomUUID();
		this.expectErrorIds.add(id);
		this.send(this.startFrame(id, query));

		const refusal = await this.settle(id);
		if (!refusal) {
			// Accepted after all - register it properly so the cleanup stop and any
			// notification it now produces are accounted for rather than reported twice.
			this.activeIds.set(id, commandId);
			await this.stop(id);
			return null;
		}

		return refusal;
	}

	async stop(id) {
		const mark = this.mark();
		const commandId = this.activeIds.get(id);
		this.expectCompleteIds.add(id);
		this.send(this.stopFrame(id));

		const frame = await this.waitFor(f => f.id === id && (f.type === 'complete' || f.type === 'error'),
			frameTimeoutMs, mark);

		if (this.activeIds.has(id)) {
			this.activeIds.delete(id);
			this.retiredIds.set(id, { commandId, at: Date.now() });
		}
		return frame;
	}

	dataFrames(id) {
		return this.frames.filter(f => f.type === 'data' && f.id === id);
	}

	waitForData(id, matcher) {
		return this.waitFor(f => f.type === 'data' && f.id === id && (!matcher || matcher(f)), frameTimeoutMs);
	}

	close() {
		if (this.socket) { this.socket.close(); }
	}
}

// A data frame wraps the notification as {"data": {"<CommandId>": {...}}} - the
// same shape SubscriptionClient.deliverReady() unwraps before handing it to the
// view. Reading frame.payload directly finds nothing, which fails slowly and
// looks exactly like "the notification never arrived".
function notificationOf(frame, commandId) {
	const data = frame && frame.payload && frame.payload.data;
	if (!data) { return null; }

	return commandId ? data[commandId] : data[Object.keys(data)[0]];
}

// ---------------------------------------------------------------- assertions

function assert(condition, message) {
	if (!condition) { throw new Error(message); }
}

// A refusal only counts when the server refused for the reason under test. Without
// this, a case that accidentally sends a malformed payload or a broken token still
// gets its error frame and passes while proving nothing.
function assertRefusalReason(frame, expectations) {
	assert(frame !== null && frame !== undefined, expectations.absent);

	const message = frame.payload && frame.payload[0] && frame.payload[0].message;
	assert(typeof message === 'string' && message.length > 0,
		`refusal carries no message: ${JSON.stringify(frame).slice(0, 300)}`);

	assert(!/Error when parsing/.test(message),
		'the server refused this because the frame itself was malformed, not because of the case under ' +
		`test - the case would prove nothing as written: "${message}"`);

	for (const forbidden of (expectations.mustNotMatch || [])) {
		assert(!forbidden.test(message), `refused for the wrong reason (matched ${forbidden}): "${message}"`);
	}

	assert(expectations.mustMatch.test(message),
		`refusal message does not match ${expectations.mustMatch}: "${message}"`);

	return message;
}

// What the routing cases require: the publisher's IsRequestSupported rejected the
// request, so no servlet claimed it (CWebSocketServletComp::RegisterSubscription).
function assertRoutingRefusal(frame, commandId, absent) {
	return assertRefusalReason(frame, {
		absent,
		mustMatch: new RegExp(`No servlet was found for the given command: '${commandId}`),
		mustNotMatch: [/^Unauthorized:/, /^Forbidden:/]
	});
}

function documentNotification(frame, commandId, documentId) {
	const notification = notificationOf(frame, commandId);
	return !!notification && notification.documentId === documentId;
}

// ---------------------------------------------------------------- test harness

const results = [];

async function test(name, fn) {
	const started = Date.now();
	try {
		await fn();
		results.push({ name, ok: true, ms: Date.now() - started });
		console.log(`  PASS  ${name}`);
	} catch (err) {
		results.push({ name, ok: false, ms: Date.now() - started, error: err && err.message ? err.message : String(err) });
		console.log(`  FAIL  ${name}\n        ${err && err.message ? err.message : err}`);
	}
	writeJUnit(junitPath);
}

// Cases downstream of a failed one must not run their assertions against undefined
// state - that reports a second, misleading defect. They fail explicitly instead.
function requires(value, what) {
	if (value === null || value === undefined || value === '') {
		throw new Error(`precondition not met: ${what} - an earlier case did not produce it, so this case was not verified`);
	}
	return value;
}

function writeJUnit(filePath) {
	const failures = results.filter(r => !r.ok).length;
	const totalMs = results.reduce((sum, r) => sum + r.ms, 0);
	const escape = (s) => String(s).replace(/[<>&"']/g, c => (
		{ '<': '&lt;', '>': '&gt;', '&': '&amp;', '"': '&quot;', "'": '&apos;' }[c]));

	const cases = results.map(r => {
		const head = `    <testcase classname="TenantApi.WebSocket" name="${escape(r.name)}" time="${(r.ms / 1000).toFixed(3)}"`;
		return r.ok
			? head + '/>'
			: head + `>\n      <failure message="${escape(r.error)}"/>\n    </testcase>`;
	}).join('\n');

	const xml =
		'<?xml version="1.0" encoding="UTF-8"?>\n' +
		`<testsuites name="Tenant API WebSocket subscriptions" tests="${results.length}" failures="${failures}" time="${(totalMs / 1000).toFixed(3)}">\n` +
		`  <testsuite name="TenantApi.WebSocket" tests="${results.length}" failures="${failures}" time="${(totalMs / 1000).toFixed(3)}">\n` +
		cases + '\n  </testsuite>\n</testsuites>\n';

	fs.writeFileSync(filePath, xml, 'utf8');
}

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
		});

		const notification = notificationOf(frame, 'OnUndoRedoChanged');
		assert(notification.availableUndoSteps >= 1,
			`an edit must leave at least one undo step, got ${JSON.stringify(notification).slice(0, 250)}`);

		// The same edit has to reach the document-scoped subscription too.
		await session.waitForData(docSubId, f => documentNotification(f, 'OnDocumentChanged', documentId));
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
		const managerMark = session.mark();

		await mutate('UpdateTenantFromRepresentation',
			'mutation UpdateTenantFromRepresentation { UpdateTenantFromRepresentation(input: {' +
			` documentId: ${gqlString(documentId)},` +
			` tenant: { id: ${gqlString(seedId)}, name: ${gqlString(tenantName)}, description: "edit after stop",` +
			` ownerId: ${gqlString(su.userId)},` +
			` isActive: true, members: [{ id: ${gqlString(su.userId)}, name: ${gqlString(suLogin)} }],` +
			' memberOrganizationPermissions: [], tenantPermissions: [] } })' +
			' { status message } }', su.token);

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

main()
	.catch((err) => {
		results.push({ name: 'suite bootstrap', ok: false, ms: 0, error: err && err.message ? err.message : String(err) });
		console.error(`\nSuite aborted: ${err && err.message ? err.message : err}`);
	})
	.finally(() => {
		writeJUnit(junitPath);
		const failures = results.filter(r => !r.ok).length;
		console.log(`\n${results.length - failures}/${results.length} passed. JUnit: ${junitPath}`);
		process.exit(failures > 0 ? 1 : 0);
	});
