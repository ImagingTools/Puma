// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
// Can a user end or read the session of another? At Puma and, with --product-http, through the server of a product.
const crypto = require('crypto');
const http = require('http');
const lib = require('./ws-lib.js');
const { gqlString, assert, jwtClaim, test, runSuite } = lib;

function argument(name, fallback) {
	const index = process.argv.indexOf(`--${name}`);
	return index > 0 && index + 1 < process.argv.length ? process.argv[index + 1] : fallback;
}

function target(name, url, productId) {
	const u = new URL(url);
	return { name, port: Number(u.port), path: u.pathname, productId };
}

const suPassword = lib.suPassword;
const userPassword = 'Session_Probe_2026!';
const targets = [target('Puma', lib.httpUrl, lib.productId || 'Puma')];
if (argument('product-http', '')) {
	targets.push(target(argument('product', 'Product'), argument('product-http', ''), argument('product', 'Product')));
}
const protectedQuery = 'query GetExternalIdentities { GetExternalIdentities { items { providerId } } }';

function post(target, query, token) {
	return new Promise((resolve, reject) => {
		const body = JSON.stringify({ query });
		const headers = { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(body), productId: target.productId };
		if (token) { headers['x-authentication-token'] = token; }
		const req = http.request({ hostname: 'localhost', port: target.port, path: target.path, method: 'POST', headers }, (res) => {
			let data = '';
			res.on('data', (c) => { data += c; });
			res.on('end', () => { let json = null; try { json = data ? JSON.parse(data) : null; } catch (e) {} resolve({ status: res.statusCode, json, raw: data }); });
		});
		req.on('error', reject);
		req.write(body);
		req.end();
	});
}

async function signIn(target, login, password) {
	const res = await post(target, `query Authorization { Authorization(input: { login: ${gqlString(login)}, password: ${gqlString(password)}, productId: ${gqlString(target.productId)} }) { userId token refreshToken } }`);
	const auth = res.json && res.json.data && res.json.data.Authorization;
	assert(auth && auth.token, `sign-in of '${login}' at ${target.name} failed: ${res.raw.slice(0, 300)}`);
	return auth;
}

async function works(target, token) {
	const res = await post(target, protectedQuery, token);
	return res.status === 200 && !(res.json && res.json.errors);
}

const field = (res, command, name) => res.json && res.json.data && res.json.data[command] ? res.json.data[command][name] : undefined;

runSuite({ name: 'Session security', className: 'SessionSecurity', junitFile: 'junit-report-ws-session-security.xml' }, async () => {
	const puma = targets[0];
	const tag = crypto.randomBytes(4).toString('hex');
	const attackerLogin = `attacker_${tag}`;
	const victimLogin = `victim_${tag}`;

	await test('two plain users exist', async () => {
		const su = await signIn(puma, lib.suLogin, suPassword);
		for (const login of [attackerLogin, victimLogin]) {
			const id = crypto.randomUUID();
			const res = await post(puma, `mutation UserAdd { UserAdd(input: { id: ${gqlString(id)}, typeId: "User", productId: "Puma",` +
				` item: { id: ${gqlString(id)}, productId: "Puma", name: ${gqlString(login)}, username: ${gqlString(login)},` +
				` password: ${gqlString(userPassword)}, email: ${gqlString(login + '@probe.test')}, groups: [], roles: [], permissions: [], systemInfos: [] } }) { id } }`, su.token);
			assert(res.json && res.json.data && res.json.data.UserAdd, `UserAdd ${login}: ${res.raw.slice(0, 300)}`);
		}
	});

	for (const target of targets) {
		const both = async () => ({ attacker: await signIn(target, attackerLogin, userPassword), victim: await signIn(target, victimLogin, userPassword) });

		await test(`${target.name}: Logout naming the token of another user ends only the own session`, async () => {
			const { attacker, victim } = await both();
			const res = await post(target, `mutation Logout { Logout(input: { accessToken: ${gqlString(victim.token)} }) { ok } }`, attacker.token);
			assert(field(res, 'Logout', 'ok') === true, `Logout: ${res.raw.slice(0, 200)}`);
			assert(await works(target, victim.token), 'the session of the victim was ended');
			assert(!(await works(target, attacker.token)), 'the session of the caller is still open: the positive control failed');
		});

		await test(`${target.name}: an anonymous Logout naming a token ends nothing`, async () => {
			const { victim } = await both();
			const res = await post(target, `mutation Logout { Logout(input: { accessToken: ${gqlString(victim.token)} }) { ok } }`, null);
			assert(field(res, 'Logout', 'ok') !== true, `anonymous Logout answered ok: ${res.raw.slice(0, 200)}`);
			assert(await works(target, victim.token), 'the session of the victim was ended');
		});

		await test(`${target.name}: RemoveSession of the session of another user is refused`, async () => {
			const { attacker, victim } = await both();
			const sessionId = jwtClaim(victim.token, 'sessionId');
			const res = await post(target, `mutation RemoveSession { RemoveSession(input: { sessionId: ${gqlString(sessionId)} }) { ok } }`, attacker.token);
			assert(field(res, 'RemoveSession', 'ok') !== true, `RemoveSession answered ok: ${res.raw.slice(0, 200)}`);
			assert(await works(target, victim.token), 'the session of the victim was ended');
			const own = await post(target, `mutation RemoveSession { RemoveSession(input: { sessionId: ${gqlString(jwtClaim(attacker.token, 'sessionId'))} }) { ok } }`, attacker.token);
			assert(field(own, 'RemoveSession', 'ok') === true, `the own session was not removed: the positive control failed: ${own.raw.slice(0, 200)}`);
		});

		await test(`${target.name}: an anonymous RemoveSession is refused`, async () => {
			const { victim } = await both();
			const res = await post(target, `mutation RemoveSession { RemoveSession(input: { sessionId: ${gqlString(jwtClaim(victim.token, 'sessionId'))} }) { ok } }`, null);
			assert(field(res, 'RemoveSession', 'ok') !== true, `anonymous RemoveSession answered ok: ${res.raw.slice(0, 200)}`);
			assert(await works(target, victim.token), 'the session of the victim was ended');
		});

		await test(`${target.name}: GetSession of another user reveals nothing`, async () => {
			const { attacker, victim } = await both();
			const res = await post(target, `query GetSession { GetSession(input: { sessionId: ${gqlString(jwtClaim(victim.token, 'sessionId'))} }) { sessionData } }`, attacker.token);
			const data = field(res, 'GetSession', 'sessionData');
			assert(!data, `the session of another user was read: ${String(data).slice(0, 200)}`);
		});

		await test(`${target.name}: a forged token cannot log anybody out`, async () => {
			const { victim } = await both();
			const parts = victim.token.split('.');
			const payload = JSON.parse(Buffer.from(parts[1], 'base64url').toString());
			payload.exp += 1000;
			const forged = `${parts[0]}.${Buffer.from(JSON.stringify(payload)).toString('base64url')}.${parts[2]}`;
			const res = await post(target, `mutation Logout { Logout(input: { accessToken: ${gqlString(forged)} }) { ok } }`, forged);
			assert(field(res, 'Logout', 'ok') !== true, `a forged token logged out: ${res.raw.slice(0, 200)}`);
			assert(await works(target, victim.token), 'the session of the victim was ended');
		});
	}
});
