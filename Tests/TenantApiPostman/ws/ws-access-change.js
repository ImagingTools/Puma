// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
// A change of the permissions of a user applies at once: at Puma the next request of its session is answered with the new
// ones, at a product server within a moment (its caches follow the notification of Puma), and the session hears of the change (OnAccessChanged), granted or taken. At Puma and, with --product-http, at a product server.
const crypto = require('crypto');
const http = require('http');
const lib = require('./ws-lib.js');
const { SubscriptionSession, gqlString, assert, delay, test, runSuite } = lib;

function argument(name, fallback) {
	const index = process.argv.indexOf(`--${name}`);
	return index > 0 && index + 1 < process.argv.length ? process.argv[index + 1] : fallback;
}

// A server, the product its roles are of, a right of that product and a request that needs it
function target(name, httpUrl, wsUrl, productId, right, checkQuery, checkCommand) {
	const u = new URL(httpUrl);
	return { name, port: Number(u.port), path: u.pathname, wsUrl, productId, right, checkQuery, checkCommand, settleMs: 0 };
}

const userPassword = 'Access_Probe_2026!';
const targets = [target('Puma', lib.httpUrl, lib.wsUrl, 'Puma', '/OrganizationManagement/Add',
	'mutation CreateTenant { CreateTenant(input: { name: "AccessProbe_%NAME%", description: "probe" }) { tenantId errorMessage } }', 'CreateTenant')];
if (argument('product-http', '')) {
	targets.push(target(argument('product', 'Product'), argument('product-http', ''), argument('product-ws', ''), argument('product', 'Product'),
		argument('product-right', ''), argument('product-check', ''), argument('product-check-command', '')));
	// A product server drops its caches of the users and roles of Puma on the notification of Puma, a moment later
	targets[1].settleMs = 2000;
}

// The changed access reaches the session within this time
const accessChangeWaitMs = 3000;

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
	const res = await post(target, `query Authorization { Authorization(input: { login: ${gqlString(login)}, password: ${gqlString(password)}, productId: ${gqlString(target.productId)} }) { userId token } }`);
	const auth = res.json && res.json.data && res.json.data.Authorization;
	assert(auth && auth.token, `sign-in of '${login}' at ${target.name} failed: ${res.raw.slice(0, 300)}`);
	return auth;
}

// An administration request of the superuser, signed in afresh: its own token may be short-lived
async function administer(target, query, command) {
	const su = await signIn(target, lib.suLogin, lib.suPassword);
	const res = await post(target, query, su.token);
	assert(res.status === 200 && res.json && res.json.data && res.json.data[command] && !(res.json.errors && res.json.errors.length),
		`${command} at ${target.name}: ${res.raw.slice(0, 300)}`);
}

async function hasRight(target, token) {
	const res = await post(target, target.checkQuery.replace('%NAME%', crypto.randomBytes(4).toString('hex')), token);
	const payload = res.json && res.json.data ? res.json.data[target.checkCommand] : null;
	const isRefused = !payload || (res.json.errors && res.json.errors.length) || (payload.errorMessage && payload.errorMessage !== '');
	return res.status === 200 && !isRefused;
}

// The right as the next requests see it: at once at Puma, within settleMs at a product server
async function rightSettles(target, token, expected) {
	const deadline = Date.now() + target.settleMs;
	while (await hasRight(target, token) !== expected) {
		if (Date.now() >= deadline) {
			return false;
		}
		await delay(100);
	}
	return true;
}

function roleQuery(target, roleId, stamp, permissions, command) {
	const list = permissions.map(p => `{id: ${gqlString(p)}, direct: true}`).join(', ');
	return `mutation ${command} { ${command}(input: { id: ${gqlString(roleId)}, typeId: "Role", productId: ${gqlString(target.productId)},` +
		` item: { id: ${gqlString(roleId)}, name: "Access probe ${stamp}", roleId: "ACCESS_PROBE_${stamp}", productId: ${gqlString(target.productId)},` +
		` parentRoles: [], permissions: [${list}] } }) { id } }`;
}

function userQuery(target, userId, login, roles, command) {
	const list = roles.map(r => `{id: ${gqlString(r)}, direct: true}`).join(', ');
	const password = command === 'UserAdd' ? ` password: ${gqlString(userPassword)},` : '';
	return `mutation ${command} { ${command}(input: { id: ${gqlString(userId)}, typeId: "User", productId: ${gqlString(target.productId)},` +
		` item: { id: ${gqlString(userId)}, productId: ${gqlString(target.productId)}, name: ${gqlString(login)}, username: ${gqlString(login)},${password}` +
		` email: ${gqlString(login + '@probe.test')}, groups: [], roles: [${list}], permissions: [], systemInfos: [] } }) { id } }`;
}

runSuite({ name: 'Access change', className: 'AccessChange', junitFile: 'junit-report-ws-access-change.xml' }, async () => {
	for (const target of targets) {
		const stamp = crypto.randomBytes(4).toString('hex');
		const roleId = crypto.randomUUID();
		const userId = crypto.randomUUID();
		const login = `access_${stamp}`;
		let user = null;
		let session = null;
		let watchId = null;
		const changesSince = (mark) => session.frames.slice(mark).filter(f => f.type === 'data' && f.id === watchId).length;

		await test(`${target.name}: a user holding a role without the right`, async () => {
			await administer(target, roleQuery(target, roleId, stamp, [], 'RoleAdd'), 'RoleAdd');
			await administer(target, userQuery(target, userId, login, [roleId], 'UserAdd'), 'UserAdd');
			user = await signIn(target, login, userPassword);
			assert(!(await hasRight(target, user.token)), 'the right is held before it was given');

			session = new SubscriptionSession(`${target.name}-access`, target.wsUrl, user.token);
			await session.connect();
			await session.init();
			watchId = await session.subscribe('OnAccessChanged', 'subscription OnAccessChanged { OnAccessChanged { changed } }');
		});

		await test(`${target.name}: a right added to the role applies to the next request`, async () => {
			const mark = session.mark();
			await administer(target, roleQuery(target, roleId, stamp, [target.right], 'RoleUpdate'), 'RoleUpdate');
			assert(await rightSettles(target, user.token, true), 'the next request does not have the right');
			await session.waitForData(watchId, null, mark).catch(() => { throw new Error('the session did not hear of the change'); });
		});

		await test(`${target.name}: a right removed from the role is refused at the next request`, async () => {
			const mark = session.mark();
			await administer(target, roleQuery(target, roleId, stamp, [], 'RoleUpdate'), 'RoleUpdate');
			assert(await rightSettles(target, user.token, false), 'the removed right still works');
			await session.waitForData(watchId, null, mark).catch(() => { throw new Error('the session did not hear of the change'); });
		});

		await test(`${target.name}: a role taken from the user is refused at the next request`, async () => {
			// The session hears of the right given again first: a change and its undoing seen in one check of the session tell nothing
			const givenMark = session.mark();
			await administer(target, roleQuery(target, roleId, stamp, [target.right], 'RoleUpdate'), 'RoleUpdate');
			assert(await rightSettles(target, user.token, true), 'the right given again does not work');
			await session.waitForData(watchId, null, givenMark).catch(() => { throw new Error('the session did not hear of the right given again'); });
			const mark = session.mark();
			await administer(target, userQuery(target, userId, login, [], 'UserUpdate'), 'UserUpdate');
			assert(await rightSettles(target, user.token, false), 'the right of the taken role still works');
			await session.waitForData(watchId, null, mark).catch(() => { throw new Error('the session did not hear of the change'); });
		});

		await test(`${target.name}: a change of somebody else is not told to the session`, async () => {
			await delay(accessChangeWaitMs);
			const mark = session.mark();
			const otherRoleId = crypto.randomUUID();
			await administer(target, roleQuery(target, otherRoleId, `${stamp}x`, [target.right], 'RoleAdd'), 'RoleAdd');
			await delay(accessChangeWaitMs);
			assert(changesSince(mark) === 0, `told ${changesSince(mark)} times of a role it does not hold`);
		});

		await test(`${target.name}: no frame left unaccounted`, async () => {
			await session.stop(watchId);
			session.close();
			assert(session.violations.length === 0, session.violations.join('; '));
		});
	}
});
