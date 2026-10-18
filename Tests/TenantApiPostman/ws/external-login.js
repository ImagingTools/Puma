// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
//
// External sign-in suite (Sign in with Google / Apple / Facebook / OpenID Connect).
//
// Drives the whole authorization code flow against a running Puma server: the GraphQL
// operations of the client (GetExternalLoginProviders, BeginExternalLogin,
// CompleteExternalLogin, GetExternalIdentities, UnlinkExternalIdentity), the browser
// redirect through the identity provider and the server callback servlet.
// The provider is mock-idp.js, started by this suite; the server must have been started
// with its settings written by external-login-settings.js (the generic "oidc" provider).
//
// node ws/external-login.js [--su-password ...] [--idp-port 18932] [--callback-url ...]

'use strict';

const http = require('http');
const crypto = require('crypto');
const {
	httpUrl, productId,
	gqlString, gql, mutate, login,
	assert, test, requires, runSuite
} = require('./ws-lib');
const { startMockIdp } = require('./mock-idp');

const argv = {};
for (let i = 2; i < process.argv.length; i += 2) {
	argv[process.argv[i].replace(/^--/, '')] = process.argv[i + 1];
}

const idpPort = Number(argv['idp-port'] || 18932);
const callbackUrl = argv['callback-url'] || new URL('/Puma/ExternalLogin/Callback', httpUrl).toString();
const providerId = 'oidc';
const runId = crypto.randomBytes(4).toString('hex');

function request(method, url, body, headers) {
	return new Promise((resolve, reject) => {
		const u = new URL(url);
		const req = http.request({
			hostname: u.hostname,
			port: u.port,
			path: u.pathname + u.search,
			method,
			headers: Object.assign(body ? { 'Content-Length': Buffer.byteLength(body) } : {}, headers || {})
		}, (res) => {
			let data = '';
			res.on('data', (chunk) => { data += chunk; });
			res.on('end', () => resolve({ status: res.statusCode, headers: res.headers, body: data }));
		});
		req.on('error', reject);
		if (body) {
			req.write(body);
		}
		req.end();
	});
}

function pageMessage(html) {
	const match = /<h1>([^<]*)<\/h1><p>([^<]*)<\/p>/.exec(html);
	return match ? { title: match[1], message: match[2] } : { title: '', message: html.slice(0, 200) };
}

async function beginLogin(token, link) {
	const linkArgument = link ? ', link: true' : '';
	const res = await gql(`mutation BeginExternalLogin { BeginExternalLogin(input: { providerId: ${gqlString(providerId)},` +
		` productId: ${gqlString(productId)}${linkArgument} }) { loginId authorizationUrl message } }`, token);
	return (res && res.data && res.data.BeginExternalLogin) || {};
}

async function completeLogin(loginId, token) {
	return mutate('CompleteExternalLogin', `mutation CompleteExternalLogin { CompleteExternalLogin(input: { loginId: ${gqlString(loginId)} })` +
		' { status message authorization { userId username token refreshToken permissions } } }', token);
}

// The browser part: the provider approves and redirects to the server callback.
async function authorizeInBrowser(authorizationUrl) {
	const authorize = await request('GET', authorizationUrl);
	assert(authorize.status === 302, `the provider did not redirect (HTTP ${authorize.status}): ${authorize.body.slice(0, 200)}`);

	const callback = await request('GET', authorize.headers.location);
	return { location: authorize.headers.location, status: callback.status, page: pageMessage(callback.body) };
}

async function signInWith(idp, account, token, link) {
	idp.setAccount(account);
	const started = await beginLogin(token, link);
	requires(started.loginId, `BeginExternalLogin loginId (${started.message || 'no message'})`);
	const browser = await authorizeInBrowser(started.authorizationUrl);
	const result = await completeLogin(started.loginId, token);
	return { started, browser, result };
}

async function registerUser(name) {
	const username = `ext_${name}_${runId}`;
	const password = `Ext_${name}_2026!${runId}`;
	const email = `${username}@test.local`;
	await mutate('RegisterUser', `mutation RegisterUser { RegisterUser(input: { userData: { name: ${gqlString(name)},` +
		` username: ${gqlString(username)}, password: ${gqlString(password)}, email: ${gqlString(email)} },` +
		` productId: ${gqlString(productId)} }) { id } }`);
	const auth = await login(username, password);
	return { username, password, email, userId: auth.userId, token: auth.token };
}

async function linkedProviders(token) {
	const res = await gql('query GetExternalIdentities { GetExternalIdentities { identities { providerId providerName providerType email linkedAt } hasPassword } }', token);
	return res;
}

async function main() {
	console.log(`HTTP     : ${httpUrl}`);
	console.log(`Callback : ${callbackUrl}`);

	const idp = await startMockIdp({ port: idpPort, redirectUri: callbackUrl });
	console.log(`IdP      : ${idp.issuer}\n`);

	try {
		await runCases(idp);
	}
	finally {
		await idp.close();
	}
}

async function runCases(idp) {
	let alice = null;
	let bob = null;
	let carol = null;
	const aliceSub = `alice-${runId}`;
	const bobSub = `bob-${runId}`;

	await test('only configured providers are offered', async () => {
		const res = await mutate('GetExternalLoginProviders', 'query GetExternalLoginProviders { GetExternalLoginProviders { providers { id name providerType } } }');
		const providers = res.providers || [];
		assert(providers.length === 1, `expected only the configured OpenID Connect provider (the server settings come from external-login-settings.js), got ${JSON.stringify(providers)}`);
		assert(providers[0].id === providerId && providers[0].name === 'Test IdP' && providers[0].providerType === 'Generic',
			`unexpected provider ${JSON.stringify(providers[0])}`);
	});

	await test('an unknown provider is refused', async () => {
		const res = await mutate('BeginExternalLogin', 'mutation BeginExternalLogin { BeginExternalLogin(input: { providerId: "google" }) { loginId authorizationUrl message } }');
		assert(!res.loginId && !res.authorizationUrl, `a provider without client ID started a login: ${JSON.stringify(res)}`);
		assert(/Unknown sign-in provider/.test(res.message || ''), `unexpected refusal message '${res.message}'`);
	});

	await test('the authorization request carries state, nonce and an S256 PKCE challenge', async () => {
		const started = await beginLogin();
		requires(started.loginId, 'loginId');
		const url = new URL(started.authorizationUrl);
		assert(url.origin + url.pathname === `${idp.issuer}/authorize`, `unexpected endpoint ${url.origin}${url.pathname}`);
		assert(url.searchParams.get('client_id') === idp.clientId, 'client_id');
		assert(url.searchParams.get('redirect_uri') === callbackUrl, `redirect_uri ${url.searchParams.get('redirect_uri')}`);
		assert(url.searchParams.get('code_challenge_method') === 'S256', 'code_challenge_method');
		assert((url.searchParams.get('code_challenge') || '').length === 43, 'code_challenge length');
		assert((url.searchParams.get('state') || '').length >= 43, 'state length');
		assert((url.searchParams.get('nonce') || '').length >= 43, 'nonce length');
		assert(started.authorizationUrl.indexOf(started.loginId) < 0, 'the login ID must not be sent to the provider');

		const pending = await completeLogin(started.loginId);
		assert(pending.status === 'Pending', `before the callback the login must be pending, got ${JSON.stringify(pending)}`);
	});

	await test('setup: users with password', async () => {
		alice = await registerUser('Alice');
		bob = await registerUser('Bob');
		carol = await registerUser('Carol');
	});

	await test('an account matching no user is refused', async () => {
		const flow = await signInWith(idp, { sub: `stranger-${runId}`, email: `stranger-${runId}@example.com`, email_verified: true, name: 'Stranger' });
		assert(flow.browser.page.title === 'Sign-in failed', `browser page: ${JSON.stringify(flow.browser.page)}`);
		assert(flow.result.status === 'Failed' && /No user is linked/.test(flow.result.message || ''), `result ${JSON.stringify(flow.result)}`);
		assert(!flow.result.authorization || !flow.result.authorization.token, 'a refused login must carry no session');
	});

	let aliceSession = null;

	await test('first sign-in links the user with the verified e-mail and opens a session', async () => {
		requires(alice, 'user Alice');
		const flow = await signInWith(idp, { sub: aliceSub, email: alice.email.toUpperCase(), email_verified: true, name: 'Alice' });
		assert(flow.browser.status === 200 && flow.browser.page.title === 'Signed in', `browser page: ${JSON.stringify(flow.browser.page)}`);
		assert(flow.result.status === 'SignedIn', `result ${JSON.stringify(flow.result)}`);
		const authorization = flow.result.authorization || {};
		assert(authorization.userId === alice.userId, `signed in as '${authorization.userId}', expected '${alice.userId}'`);
		assert(authorization.username === alice.username, `username '${authorization.username}'`);
		assert(authorization.token && authorization.refreshToken, 'the result must carry the session tokens');
		aliceSession = authorization;

		const again = await completeLogin(flow.started.loginId);
		assert(again.status === 'Failed', `a finished login must be handed out once, got ${JSON.stringify(again)}`);
	});

	await test('the session of an external sign-in is a regular session', async () => {
		requires(aliceSession, 'session of Alice');
		const res = await linkedProviders(aliceSession.token);
		assert(!res.errors, `GetExternalIdentities refused the session: ${JSON.stringify(res.errors)}`);
		const list = res.data.GetExternalIdentities;
		assert(list.hasPassword === true, 'Alice has a password');
		assert(list.identities.length === 1 && list.identities[0].providerId === providerId, `identities ${JSON.stringify(list.identities)}`);
		assert(list.identities[0].providerName === 'Test IdP' && list.identities[0].linkedAt, `identity ${JSON.stringify(list.identities[0])}`);
	});

	await test('the linked account signs in even with another, unverified e-mail', async () => {
		requires(alice, 'user Alice');
		const flow = await signInWith(idp, { sub: aliceSub, email: `changed-${runId}@example.org`, email_verified: false, name: 'Alice' });
		assert(flow.result.status === 'SignedIn' && flow.result.authorization.userId === alice.userId, `result ${JSON.stringify(flow.result)}`);
	});

	await test('a replayed provider redirect is refused', async () => {
		requires(alice, 'user Alice');
		idp.setAccount({ sub: aliceSub, email: alice.email, email_verified: true, name: 'Alice' });
		const started = await beginLogin();
		const browser = await authorizeInBrowser(started.authorizationUrl);
		assert(browser.page.title === 'Signed in', `first redirect: ${JSON.stringify(browser.page)}`);

		const replay = await request('GET', browser.location);
		assert(pageMessage(replay.body).title === 'Sign-in failed', `the replayed redirect was accepted: ${replay.body.slice(0, 200)}`);
		await completeLogin(started.loginId);
	});

	await test('an ID token for another sign-in request (nonce) is refused', async () => {
		requires(alice, 'user Alice');
		idp.overrideNextIdToken({ nonce: 'nonce-of-another-request' });
		const flow = await signInWith(idp, { sub: aliceSub, email: alice.email, email_verified: true, name: 'Alice' });
		assert(flow.result.status === 'Failed', `result ${JSON.stringify(flow.result)}`);
	});

	await test('an ID token issued for another client is refused', async () => {
		requires(alice, 'user Alice');
		idp.overrideNextIdToken({ aud: 'another-client' });
		const flow = await signInWith(idp, { sub: aliceSub, email: alice.email, email_verified: true, name: 'Alice' });
		assert(flow.result.status === 'Failed', `result ${JSON.stringify(flow.result)}`);
	});

	await test('an expired ID token is refused', async () => {
		requires(alice, 'user Alice');
		idp.overrideNextIdToken({ exp: Math.floor(Date.now() / 1000) - 3600 });
		const flow = await signInWith(idp, { sub: aliceSub, email: alice.email, email_verified: true, name: 'Alice' });
		assert(flow.result.status === 'Failed', `result ${JSON.stringify(flow.result)}`);
	});

	await test('the callback accepts a form post (response_mode=form_post)', async () => {
		requires(alice, 'user Alice');
		idp.setAccount({ sub: aliceSub, email: alice.email, email_verified: true, name: 'Alice' });
		const started = await beginLogin();
		const authorize = await request('GET', started.authorizationUrl);
		const redirect = new URL(authorize.headers.location);

		const form = `code=${encodeURIComponent(redirect.searchParams.get('code'))}&state=${encodeURIComponent(redirect.searchParams.get('state'))}`;
		const callback = await request('POST', callbackUrl, form, { 'Content-Type': 'application/x-www-form-urlencoded' });
		assert(pageMessage(callback.body).title === 'Signed in', `form post: ${callback.body.slice(0, 200)}`);

		const result = await completeLogin(started.loginId);
		assert(result.status === 'SignedIn' && result.authorization.userId === alice.userId, `result ${JSON.stringify(result)}`);
	});

	await test('a cancelled sign-in at the provider fails the login', async () => {
		const started = await beginLogin();
		const state = new URL(started.authorizationUrl).searchParams.get('state');
		const callback = await request('GET', `${callbackUrl}?error=access_denied&state=${encodeURIComponent(state)}`);
		assert(/cancelled/.test(pageMessage(callback.body).message), `browser page: ${callback.body.slice(0, 200)}`);

		const result = await completeLogin(started.loginId);
		assert(result.status === 'Failed' && /cancelled/.test(result.message || ''), `result ${JSON.stringify(result)}`);
	});

	await test('linked accounts are not readable or changeable anonymously', async () => {
		const list = await linkedProviders();
		assert(list.errors && /signed-in user/.test(list.errors[0].message), `anonymous GetExternalIdentities: ${JSON.stringify(list)}`);

		const unlink = await gql(`mutation UnlinkExternalIdentity { UnlinkExternalIdentity(input: { providerId: ${gqlString(providerId)} }) { ok message } }`);
		assert(unlink.errors && /signed-in user/.test(unlink.errors[0].message), `anonymous UnlinkExternalIdentity: ${JSON.stringify(unlink)}`);

		const link = await beginLogin(undefined, true);
		assert(!link.loginId && /Sign in before linking/.test(link.message || ''), `anonymous link: ${JSON.stringify(link)}`);
	});

	await test('a signed-in user links another external account', async () => {
		requires(bob, 'user Bob');
		const flow = await signInWith(idp, { sub: bobSub, email: `bob.private-${runId}@example.net`, email_verified: false, name: 'Bob' }, bob.token, true);
		assert(flow.browser.page.title === 'Signed in', `browser page: ${JSON.stringify(flow.browser.page)}`);
		assert(flow.result.status === 'Linked', `result ${JSON.stringify(flow.result)}`);
		assert(!flow.result.authorization || !flow.result.authorization.token, 'linking must not open a new session');

		const list = (await linkedProviders(bob.token)).data.GetExternalIdentities;
		assert(list.identities.length === 1 && list.identities[0].email === `bob.private-${runId}@example.net`, `identities ${JSON.stringify(list.identities)}`);

		const signIn = await signInWith(idp, { sub: bobSub, email: `bob.private-${runId}@example.net`, email_verified: false, name: 'Bob' });
		assert(signIn.result.status === 'SignedIn' && signIn.result.authorization.userId === bob.userId, `sign-in with the linked account: ${JSON.stringify(signIn.result)}`);
	});

	await test('an account linked to another user cannot be linked', async () => {
		requires(carol, 'user Carol');
		const flow = await signInWith(idp, { sub: bobSub, email: `bob.private-${runId}@example.net`, email_verified: false, name: 'Bob' }, carol.token, true);
		assert(flow.result.status === 'Failed' && /already linked to another user/.test(flow.result.message || ''), `result ${JSON.stringify(flow.result)}`);

		const list = (await linkedProviders(carol.token)).data.GetExternalIdentities;
		assert(list.identities.length === 0, `Carol got the account of Bob: ${JSON.stringify(list.identities)}`);
	});

	await test('a user unlinks an account and can no longer sign in with it', async () => {
		requires(aliceSession, 'session of Alice');
		const unlink = await mutate('UnlinkExternalIdentity', `mutation UnlinkExternalIdentity { UnlinkExternalIdentity(input: { providerId: ${gqlString(providerId)} }) { ok message } }`, alice.token);
		assert(unlink.ok === true, `unlink ${JSON.stringify(unlink)}`);

		const list = (await linkedProviders(alice.token)).data.GetExternalIdentities;
		assert(list.identities.length === 0, `identities after unlink ${JSON.stringify(list.identities)}`);

		const again = await mutate('UnlinkExternalIdentity', `mutation UnlinkExternalIdentity { UnlinkExternalIdentity(input: { providerId: ${gqlString(providerId)} }) { ok message } }`, alice.token);
		assert(again.ok === false && /not linked/.test(again.message || ''), `second unlink ${JSON.stringify(again)}`);

		const flow = await signInWith(idp, { sub: aliceSub, email: `changed-${runId}@example.org`, email_verified: false, name: 'Alice' });
		assert(flow.result.status === 'Failed', `the unlinked account still signs in: ${JSON.stringify(flow.result)}`);
	});

	await test('every code exchange carried valid client credentials, redirect URI and PKCE verifier', async () => {
		assert(idp.state.rejectedTokenRequests === 0, `${idp.state.rejectedTokenRequests} token requests were rejected by the provider`);
	});
}

runSuite({ name: 'External sign-in suite', className: 'ExternalLogin', junitFile: 'junit-report-external-login.xml' }, main);
