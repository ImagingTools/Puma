// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
//
// Writes the "External Sign-in" settings of a Puma server so its generic OpenID Connect
// provider points to the mock provider (mock-idp.js). The server reads them at start.
//
// node external-login-settings.js --file "<settings xml>" [--idp http://127.0.0.1:18932]
//      [--callback http://localhost:17788/Puma/ExternalLogin/Callback] [--clear]

'use strict';

const fs = require('fs');

const args = {};
for (let i = 2; i < process.argv.length; i++) {
	const key = process.argv[i].replace(/^--/, '');
	const next = process.argv[i + 1];
	if (next === undefined || next.startsWith('--')) {
		args[key] = true;
	}
	else {
		args[key] = next;
		i++;
	}
}

if (!args.file) {
	console.error('--file <settings xml> is required');
	process.exit(2);
}

const idp = args.idp || 'http://127.0.0.1:18932';
const values = args.clear ? {} : {
	CallbackUrl: args.callback || 'http://localhost:17788/Puma/ExternalLogin/Callback',
	OidcProviderName: 'Test IdP',
	OidcAuthorizationEndpoint: `${idp}/authorize`,
	OidcTokenEndpoint: `${idp}/token`,
	OidcUserInfoEndpoint: `${idp}/userinfo`,
	OidcIssuer: idp,
	OidcClientId: 'puma-test-client',
	OidcClientSecret: 'puma-test-secret'
};

const ids = ['CallbackUrl', 'GoogleClientId', 'GoogleClientSecret', 'AppleClientId', 'AppleClientSecret',
	'FacebookClientId', 'FacebookClientSecret', 'OidcProviderName', 'OidcAuthorizationEndpoint', 'OidcTokenEndpoint',
	'OidcUserInfoEndpoint', 'OidcIssuer', 'OidcClientId', 'OidcClientSecret'];

function xmlAttribute(value) {
	return String(value).replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/</g, '&lt;');
}

const section = [
	'        <Parameter Id="ExternalLogin">',
	'            <Parameters>',
	...ids.map((id) => `                <Parameter Id="${id}" Text="${xmlAttribute(values[id] || '')}"/>`),
	'            </Parameters>',
	'        </Parameter>'
].join('\r\n');

let xml = fs.readFileSync(args.file, 'utf8');
xml = xml.replace(/\r?\n        <Parameter Id="ExternalLogin">[\s\S]*?\r?\n        <\/Parameter>/, '');

const closing = xml.lastIndexOf('    </Parameters>\r\n</Acf>') >= 0
	? xml.lastIndexOf('    </Parameters>\r\n</Acf>')
	: xml.lastIndexOf('    </Parameters>');
if (closing < 0) {
	console.error(`No top-level <Parameters> in ${args.file}`);
	process.exit(1);
}

xml = xml.slice(0, closing) + section + '\r\n' + xml.slice(closing);
fs.writeFileSync(args.file, xml, 'utf8');

console.log(`External sign-in settings ${args.clear ? 'cleared' : `point to ${idp}`} in ${args.file}`);
