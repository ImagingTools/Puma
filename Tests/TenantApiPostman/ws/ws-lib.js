// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
//
// Shared runtime of the Tenant API WebSocket suites: configuration from the Postman
// environment, the HTTP GraphQL client, the subscription session speaking the
// ImtCore WS protocol, assertions and the JUnit-writing test harness.
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
let junitPath = args.junit || '';
let reportName = '';
let reportClass = '';
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

	// Once the grace window of a beginSubscribe() has passed: the refusal frame, or
	// null - and from then on the id counts as accepted, so a late error is a violation.
	outcomeOf(id) {
		const refusal = this.frames.find(f => f.id === id && f.type === 'error');
		if (refusal) {
			this.activeIds.delete(id);
			return refusal;
		}
		this.expectErrorIds.delete(id);
		return null;
	}

	// Turns an id whose grace window has passed into an accepted subscription, or
	// throws with the refusal the server gave. Acceptance is silence only: a case
	// that relies on it has to prove delivery with a data frame of its own.
	confirmAccepted(id) {
		const refusal = this.outcomeOf(id);
		if (refusal) {
			throw new Error(`Subscription refused: ${JSON.stringify(refusal.payload).slice(0, 400)}`);
		}
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

	waitForData(id, matcher, fromIndex) {
		return this.waitFor(f => f.type === 'data' && f.id === id && (!matcher || matcher(f)), frameTimeoutMs, fromIndex);
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

function documentNotification(frame, commandId, documentId, documentOperation) {
	const notification = notificationOf(frame, commandId);
	return !!notification && notification.documentId === documentId &&
		(documentOperation === undefined || notification.documentOperation === documentOperation);
}

// Reads one claim of a JWT access token without verifying it: the server signed it,
// the suite only needs to know which session it names.
function jwtClaim(token, claim) {
	const parts = String(token || '').split('.');
	if (parts.length !== 3) { return undefined; }
	try {
		const json = Buffer.from(parts[1].replace(/-/g, '+').replace(/_/g, '/'), 'base64').toString('utf8');
		return JSON.parse(json)[claim];
	}
	catch (e) {
		return undefined;
	}
}

function delay(ms) {
	return new Promise(r => setTimeout(r, ms));
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
		const head = `    <testcase classname="${reportClass}" name="${escape(r.name)}" time="${(r.ms / 1000).toFixed(3)}"`;
		return r.ok
			? head + '/>'
			: head + `>\n      <failure message="${escape(r.error)}"/>\n    </testcase>`;
	}).join('\n');

	const xml =
		'<?xml version="1.0" encoding="UTF-8"?>\n' +
		`<testsuites name="${reportName}" tests="${results.length}" failures="${failures}" time="${(totalMs / 1000).toFixed(3)}">\n` +
		`  <testsuite name="${reportClass}" tests="${results.length}" failures="${failures}" time="${(totalMs / 1000).toFixed(3)}">\n` +
		cases + '\n  </testsuite>\n</testsuites>\n';

	fs.writeFileSync(filePath, xml, 'utf8');
}

// Names the report and runs the suite; exits non-zero when any case failed.
function runSuite(options, main) {
	reportName = options.name;
	reportClass = options.className;
	junitPath = junitPath || path.join(__dirname, '..', options.junitFile);

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
}

module.exports = {
	httpUrl, wsUrl, suLogin, suPassword, productId, frameTimeoutMs, registrationGraceMs, envValues,
	gqlString, gql, mutate, login,
	SubscriptionSession, notificationOf,
	assert, assertRefusalReason, assertRoutingRefusal, documentNotification, jwtClaim, delay,
	test, requires, runSuite
};
