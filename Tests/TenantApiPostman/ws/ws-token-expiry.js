// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
// The access token of a session expires while its client is subscribed: requests, subscriptions, refresh, sign-out and the end of the session.
// The server runs with short sessions (Run-CiTests.ps1 puts them into its settings): --jwt-lifetime and --session-lifetime name them.
const crypto = require('crypto');
const http = require('http');
const lib = require('./ws-lib.js');
const { SubscriptionSession, gqlString, notificationOf, assert, jwtClaim, delay, test, runSuite } = lib;

function argument(name, fallback) {
	const index = process.argv.indexOf(`--${name}`);
	return index > 0 && index + 1 < process.argv.length ? process.argv[index + 1] : fallback;
}

const suPassword = lib.suPassword;
const jwtLifetime = Number(argument('jwt-lifetime', 20));
const sessionLifetime = Number(argument('session-lifetime', 75));
const started = Date.now();
const nowSecs = () => Math.floor(Date.now() / 1000);
const log = (text) => console.log(`  [t+${((Date.now() - started) / 1000).toFixed(1)}s] ${text}`);
const rolesQuery = 'subscription OnRolesCollectionChanged { OnRolesCollectionChanged(input: {}) { notification { id } } }';
const protectedQuery = 'query GetExternalIdentities { GetExternalIdentities { items { providerId } } }';

function post(query, token) {
	return new Promise((resolve, reject) => {
		const u = new URL(lib.httpUrl);
		const body = JSON.stringify({ query });
		const headers = { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(body), productId: lib.productId };
		if (token) { headers['x-authentication-token'] = token; }
		const req = http.request({ hostname: u.hostname, port: u.port, path: u.pathname, method: 'POST', headers }, (res) => {
			let data = '';
			res.on('data', (chunk) => { data += chunk; });
			res.on('end', () => {
				let json = null;
				try { json = data ? JSON.parse(data) : null; } catch (e) { json = null; }
				resolve({ status: res.statusCode, json, raw: data });
			});
		});
		req.on('error', reject);
		req.write(body);
		req.end();
	});
}

async function signIn() {
	const res = await post(`query Authorization { Authorization(input: { login: ${gqlString(lib.suLogin)}, password: ${gqlString(suPassword)}, productId: ${gqlString(lib.productId)} }) { userId token refreshToken } }`);
	const auth = res.json && res.json.data && res.json.data.Authorization;
	assert(auth && auth.token && auth.refreshToken, `sign-in failed: ${res.raw.slice(0, 300)}`);
	return { token: auth.token, refreshToken: auth.refreshToken };
}

// As the client does: without the expired access token
async function refresh(refreshToken) {
	const res = await post(`mutation RefreshToken { RefreshToken(input: { refreshToken: ${gqlString(refreshToken)} }) { ok userSession { userId accessToken refreshToken } } }`);
	const payload = res.json && res.json.data && res.json.data.RefreshToken;
	const ok = !!(payload && payload.ok && payload.userSession && payload.userSession.accessToken);
	return { ok, status: res.status, raw: res.raw, token: ok ? payload.userSession.accessToken : null, refreshToken: ok ? payload.userSession.refreshToken : null };
}

async function requestStatus(token) {
	const res = await post(protectedQuery, token);
	if (res.status === 200 && res.json && res.json.errors) { return `200+errors ${JSON.stringify(res.json.errors).slice(0, 120)}`; }
	return res.status;
}

async function addRole() {
	const actor = await signIn();
	const id = crypto.randomUUID();
	const code = `EXPIRY_${id.slice(0, 8)}`;
	const res = await post(`mutation RoleAdd { RoleAdd(input: { id: ${gqlString(id)}, typeId: "Role", productId: ${gqlString(lib.productId)},` +
		` item: { id: ${gqlString(id)}, name: ${gqlString(code)}, roleId: ${gqlString(code)}, productId: ${gqlString(lib.productId)}, parentRoles: [], permissions: [] } }) { id } }`, actor.token);
	assert(res.status === 200 && res.json && res.json.data && res.json.data.RoleAdd, `RoleAdd failed: ${res.raw.slice(0, 300)}`);
	return id;
}

async function expectRoleNotification(session, subId) {
	const mark = session.mark();
	const id = await addRole();
	await session.waitForData(subId, (f) => {
		const n = notificationOf(f, 'OnRolesCollectionChanged');
		return !!n && n.itemId === id;
	}, mark);
}

function refusalCode(frame) {
	return frame && Array.isArray(frame.payload) && frame.payload[0] && frame.payload[0].extensions ? frame.payload[0].extensions.code : undefined;
}

async function openSession(label, token) {
	const session = new SubscriptionSession(label, lib.wsUrl, token);
	await session.connect();
	await session.init();
	return session;
}

async function startExpectingRefusal(session, token) {
	session.token = token;
	const id = crypto.randomUUID();
	session.expectErrorIds.add(id);
	session.send(session.startFrame(id, rolesQuery));
	const frame = await session.settle(id);
	return { id, frame };
}

async function waitUntil(secs) {
	const ms = secs * 1000 - Date.now();
	if (ms > 0) { log(`waiting ${(ms / 1000).toFixed(1)} s`); await delay(ms); }
}

runSuite({ name: 'WS token expiry', className: 'WsTokenExpiry', junitFile: 'junit-report-ws-token-expiry.xml' }, async () => {
	let client = null;
	let main = null;
	let subA = null;
	let subB = null;
	const sessions = [];

	await test('a token lives as long as configured', async () => {
		client = await signIn();
		const life = jwtClaim(client.token, 'exp') - nowSecs();
		assert(life >= jwtLifetime - 2 && life <= jwtLifetime, `the token lives ${life} s, the run needs ${jwtLifetime} s`);
		assert(await requestStatus(client.token) === 200, 'a fresh token opens a request');
	});

	await test('a subscription with a fresh token delivers', async () => {
		main = await openSession('main', client.token);
		sessions.push(main);
		subA = await main.subscribe('OnRolesCollectionChanged', rolesQuery);
		await expectRoleNotification(main, subA);
	});

	await test('an expired token is refused for a request with 401', async () => {
		await waitUntil(jwtClaim(client.token, 'exp') + 1);
		const status = await requestStatus(client.token);
		assert(status === 401, `expected 401, got ${status}`);
	});

	await test('the established subscription outlives its token while the session lives', async () => {
		await expectRoleNotification(main, subA);
		assert(!main.frames.some(f => f.type === 'error' && f.id === subA), 'the subscription was ended');
	});

	await test('a new subscription with the expired token is refused as Unauthorized', async () => {
		const { frame } = await startExpectingRefusal(main, client.token);
		assert(frame, 'the start was accepted');
		assert(refusalCode(frame) === 'Unauthorized', `code ${refusalCode(frame)}: ${JSON.stringify(frame.payload)}`);
	});

	await test('after a reconnect the expired token is refused as Unauthorized', async () => {
		const reconnected = await openSession('reconnected', client.token);
		sessions.push(reconnected);
		const { frame } = await startExpectingRefusal(reconnected, client.token);
		assert(frame && refusalCode(frame) === 'Unauthorized', `got ${JSON.stringify(frame)}`);
		reconnected.close();
	});

	let refreshedB = null;
	await test('a refresh keeps the session and opens requests again', async () => {
		refreshedB = await refresh(client.refreshToken);
		assert(refreshedB.ok, `refresh failed: ${refreshedB.raw.slice(0, 300)}`);
		assert(jwtClaim(refreshedB.token, 'sessionId') === jwtClaim(client.token, 'sessionId'), 'a new session was opened');
		assert(refreshedB.refreshToken !== client.refreshToken, 'the refresh token was not rotated');
		assert(await requestStatus(refreshedB.token) === 200, 'the refreshed token does not open a request');
		assert(await requestStatus(client.token) === 401, 'the expired token works again');
	});

	await test('the replaced refresh token gives the current session again while its successor is unused', async () => {
		// The answer to a refresh was lost, or two clients of the session refreshed at once
		const replay = await refresh(client.refreshToken);
		assert(replay.ok, `the replaced refresh token was refused: ${replay.raw.slice(0, 200)}`);
		assert(replay.refreshToken === refreshedB.refreshToken, 'the replay rotated the session again');
		assert(await requestStatus(replay.token) === 200, 'the token of the replay does not work');
		assert(await requestStatus(refreshedB.token) === 200, 'the replay closed the session');
	});

	await test('a token issued before a refresh holds until its own end', async () => {
		const other = await signIn();
		const next = await refresh(other.refreshToken);
		assert(next.ok, 'refresh failed');
		assert(await requestStatus(other.token) === 200, 'the earlier token was revoked by the refresh');
		assert(await requestStatus(next.token) === 200, 'the new token does not work');
	});

	await test('stop and start with the refreshed token move the delivery to the new subscription', async () => {
		const stopped = await main.stop(subA);
		assert(stopped.type === 'complete', `stop answered ${JSON.stringify(stopped)}`);
		main.token = refreshedB.token;
		subB = await main.subscribe('OnRolesCollectionChanged', rolesQuery);
		await expectRoleNotification(main, subB);
		await delay(800);
		assert(!main.dataFrames(subA).some(f => main.frames.indexOf(f) > main.frames.indexOf(stopped)), 'the stopped subscription still delivers');
	});

	let latest = null;
	await test('concurrent refreshes with one refresh token rotate it once', async () => {
		const results = await Promise.all(Array.from({ length: 5 }, () => refresh(refreshedB.refreshToken)));
		const refused = results.filter(r => !r.ok);
		assert(refused.length === 0, `${refused.length} of the concurrent refreshes were refused`);
		const rotated = new Set(results.map(r => r.refreshToken));
		assert(rotated.size === 1 && !rotated.has(refreshedB.refreshToken), `rotated to ${rotated.size} tokens`);
		latest = results[0];
		assert(await requestStatus(latest.token) === 200, 'the new token does not work');
	});

	await test('a token replaced twice is refused within the leeway and closes nothing', async () => {
		const own = await signIn();
		const first = await refresh(own.refreshToken);
		const second = await refresh(first.refreshToken);
		assert(first.ok && second.ok, 'refresh failed');
		const replay = await refresh(own.refreshToken);
		assert(!replay.ok, `a token replaced twice worked: ${replay.raw.slice(0, 200)}`);
		assert(await requestStatus(second.token) === 200, 'the refusal closed the session');
	});

	await test('a sign-out ends the subscriptions of its session', async () => {
		const leaving = await signIn();
		const session = await openSession('leaving', leaving.token);
		sessions.push(session);
		const subId = await session.subscribe('OnRolesCollectionChanged', rolesQuery);
		session.expectError(subId);
		const mark = session.mark();
		const out = await post(`mutation Logout { Logout(input: { accessToken: ${gqlString(leaving.token)} }) { ok } }`, leaving.token);
		assert(out.status === 200 && out.json && out.json.data && out.json.data.Logout && out.json.data.Logout.ok, `logout failed: ${out.raw.slice(0, 200)}`);
		const loggedOut = Date.now();
		assert(await requestStatus(leaving.token) === 401, 'the token of a closed session opens a request');
		const ended = await session.waitFor(f => f.type === 'error' && f.id === subId, 45000, mark).catch(() => null);
		assert(ended, 'the subscription of a closed session is still served after 45 s');
		log(`ended ${((Date.now() - loggedOut) / 1000).toFixed(1)} s after the sign-out, code ${refusalCode(ended)}`);
		assert(refusalCode(ended) === 'Unauthorized', `code ${refusalCode(ended)}`);
		const refreshAfter = await refresh(leaving.refreshToken);
		assert(!refreshAfter.ok, 'the refresh token of a closed session works');
		session.close();
	});

	await test('a refresh before the end of the session keeps the subscription past its first end', async () => {
		// The session ends sessionLifetime after the last refresh
		const firstEnd = jwtClaim(latest.token, 'exp') - jwtLifetime + sessionLifetime;
		await waitUntil(firstEnd - 15);
		const extended = await refresh(latest.refreshToken);
		assert(extended.ok, `refresh failed: ${extended.raw.slice(0, 200)}`);
		latest = extended;
		await waitUntil(firstEnd + 5);
		assert(!main.frames.some(f => f.type === 'error' && f.id === subB), `the subscription was ended at the first end of the session`);
		await expectRoleNotification(main, subB);
	});

	await test('the end of the session ends the subscription as Unauthorized', async () => {
		main.expectError(subB);
		const mark = main.mark();
		const ended = await main.waitFor(f => f.type === 'error' && f.id === subB, (sessionLifetime + 40) * 1000, mark);
		const lag = Date.now() / 1000 - (jwtClaim(latest.token, 'exp') - jwtLifetime + sessionLifetime);
		log(`ended ${lag.toFixed(1)} s after the end of the session, code ${refusalCode(ended)}`);
		assert(refusalCode(ended) === 'Unauthorized', `code ${refusalCode(ended)}`);
		assert(lag > -1 && lag < 10, `ended ${lag.toFixed(1)} s off the end of the session`);
		const late = await refresh(latest.refreshToken);
		assert(!late.ok, 'the refresh token of an ended session works');
		assert(await requestStatus(latest.token) === 401, 'the token of an ended session opens a request');
	});

	await test('no frame left unaccounted', async () => {
		const violations = sessions.flatMap(s => s.violations);
		assert(violations.length === 0, violations.join('\n'));
	});

	for (const s of sessions) { s.close(); }
});
