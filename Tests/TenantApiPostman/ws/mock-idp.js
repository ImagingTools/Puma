// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
//
// OpenID Connect provider for the external sign-in tests (ImtCore CExternalLoginControllerComp,
// configured as the generic "oidc" provider of Puma).
//
// GET  /authorize  approves at once: redirects to redirect_uri with a code for the current account
// POST /token      exchanges the code; checks client credentials, redirect URI and the PKCE verifier
// GET  /userinfo   claims of the account for the access token
// POST /mock/account  sets the account the next /authorize signs in: {"sub","email","email_verified","name"}
// POST /mock/next-id-token  overrides claims of the next issued ID token (e.g. {"aud":"other"})
// GET  /mock/stats  {"rejectedTokenRequests": n, "authorizeRequests": [...query]}
//
// Standalone: node mock-idp.js [--port 18932] [--client-id ...] [--client-secret ...] [--redirect-uri ...]

'use strict';

const http = require('http');
const crypto = require('crypto');

function base64Url(buffer) {
	return Buffer.from(buffer).toString('base64').replace(/\+/g, '-').replace(/\//g, '_').replace(/=+$/, '');
}

function readBody(req) {
	return new Promise((resolve) => {
		let data = '';
		req.on('data', (chunk) => { data += chunk; });
		req.on('end', () => resolve(data));
	});
}

function sendJson(res, status, value) {
	const body = JSON.stringify(value);
	res.writeHead(status, { 'Content-Type': 'application/json', 'Content-Length': Buffer.byteLength(body) });
	res.end(body);
}

function startMockIdp(options) {
	const port = options.port || 18932;
	const issuer = `http://127.0.0.1:${port}`;
	const clientId = options.clientId || 'puma-test-client';
	const clientSecret = options.clientSecret || 'puma-test-secret';
	const redirectUri = options.redirectUri || 'http://localhost:17788/Puma/ExternalLogin/Callback';

	const state = {
		account: { sub: 'mock-user', email: 'mock.user@example.com', email_verified: true, name: 'Mock User' },
		nextIdTokenOverrides: null,
		codes: new Map(),
		accessTokens: new Map(),
		rejectedTokenRequests: 0,
		authorizeRequests: []
	};

	const server = http.createServer(async (req, res) => {
		const url = new URL(req.url, issuer);

		if (req.method === 'GET' && url.pathname === '/authorize') {
			const query = Object.fromEntries(url.searchParams.entries());
			state.authorizeRequests.push(query);

			if (query.client_id !== clientId || query.redirect_uri !== redirectUri || query.response_type !== 'code') {
				res.writeHead(400, { 'Content-Type': 'text/plain' });
				res.end('invalid authorization request');
				return;
			}

			const code = base64Url(crypto.randomBytes(24));
			state.codes.set(code, {
				challenge: query.code_challenge || '',
				nonce: query.nonce || '',
				account: Object.assign({}, state.account)
			});

			const target = new URL(redirectUri);
			target.searchParams.set('code', code);
			target.searchParams.set('state', query.state || '');
			res.writeHead(302, { Location: target.toString() });
			res.end();
			return;
		}

		if (req.method === 'POST' && url.pathname === '/token') {
			const form = new URLSearchParams(await readBody(req));
			const grant = state.codes.get(form.get('code'));
			state.codes.delete(form.get('code'));

			const verifier = form.get('code_verifier') || '';
			const isValid = grant &&
				form.get('grant_type') === 'authorization_code' &&
				form.get('client_id') === clientId &&
				form.get('client_secret') === clientSecret &&
				form.get('redirect_uri') === redirectUri &&
				grant.challenge !== '' &&
				base64Url(crypto.createHash('sha256').update(verifier).digest()) === grant.challenge;
			if (!isValid) {
				state.rejectedTokenRequests++;
				sendJson(res, 400, { error: 'invalid_grant', error_description: 'The code, the client or the verifier is invalid' });
				return;
			}

			const now = Math.floor(Date.now() / 1000);
			const claims = Object.assign({
				iss: issuer,
				aud: clientId,
				sub: grant.account.sub,
				email: grant.account.email,
				email_verified: grant.account.email_verified,
				name: grant.account.name,
				nonce: grant.nonce,
				iat: now,
				exp: now + 600
			}, state.nextIdTokenOverrides || {});
			state.nextIdTokenOverrides = null;

			const accessToken = base64Url(crypto.randomBytes(24));
			state.accessTokens.set(accessToken, { sub: grant.account.sub });

			const idToken = [
				base64Url(JSON.stringify({ alg: 'RS256', typ: 'JWT' })),
				base64Url(JSON.stringify(claims)),
				base64Url('signature')
			].join('.');

			sendJson(res, 200, { access_token: accessToken, token_type: 'Bearer', expires_in: 3600, id_token: idToken });
			return;
		}

		if (req.method === 'GET' && url.pathname === '/userinfo') {
			const authorization = req.headers.authorization || '';
			const userInfo = state.accessTokens.get(authorization.replace(/^Bearer /, ''));
			if (!userInfo) {
				sendJson(res, 401, { error: 'invalid_token' });
				return;
			}
			sendJson(res, 200, userInfo);
			return;
		}

		if (req.method === 'POST' && url.pathname === '/mock/account') {
			state.account = JSON.parse(await readBody(req));
			sendJson(res, 200, { ok: true });
			return;
		}

		if (req.method === 'POST' && url.pathname === '/mock/next-id-token') {
			state.nextIdTokenOverrides = JSON.parse(await readBody(req));
			sendJson(res, 200, { ok: true });
			return;
		}

		if (req.method === 'GET' && url.pathname === '/mock/stats') {
			sendJson(res, 200, { rejectedTokenRequests: state.rejectedTokenRequests, authorizeRequests: state.authorizeRequests });
			return;
		}

		sendJson(res, 404, { error: 'not_found' });
	});

	return new Promise((resolve, reject) => {
		server.on('error', reject);
		server.listen(port, '127.0.0.1', () => {
			resolve({
				issuer, clientId, clientSecret, redirectUri, state,
				setAccount(account) { state.account = Object.assign({}, account); },
				overrideNextIdToken(claims) { state.nextIdTokenOverrides = claims; },
				close() { return new Promise((done) => server.close(done)); }
			});
		});
	});
}

module.exports = { startMockIdp };

if (require.main === module) {
	const args = {};
	for (let i = 2; i < process.argv.length; i += 2) {
		args[process.argv[i].replace(/^--/, '')] = process.argv[i + 1];
	}

	startMockIdp({
		port: Number(args.port || 18932),
		clientId: args['client-id'],
		clientSecret: args['client-secret'],
		redirectUri: args['redirect-uri']
	}).then((idp) => {
		console.log(`Mock OpenID Connect provider at ${idp.issuer} (client '${idp.clientId}', redirect '${idp.redirectUri}')`);
	});
}
