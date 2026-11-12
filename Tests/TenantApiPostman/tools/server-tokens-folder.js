// Builds isolated bootstrap users and server token folders: run "node tools/server-tokens-folder.js"
// after editing the cases here. The receiver's permission tree is authoritative; refusals pin their reason.
const fs = require('fs');
const path = require('path');

const collectionPath = path.join(__dirname, '..', 'Tenant_System_Full.postman_collection.json');

// Shared clients stay registered; only these probe products belong to this run.
const probeProducts = ['StProbe', 'StAdded', 'StNoProduct', 'StOther', 'StBySrv', 'StByUser', 'StByAnonymous'];
const clientProductId = (id) => probeProducts.includes(id) ? `${id}{{stRun}}` : id;

function item(name, query, options = {}){
	query = query.replace(/"(StProbe|StAdded|StNoProduct|StOther|StBySrv|StByUser|StByAnonymous)"/g, (_, id) => `"${clientProductId(id)}"`);
	const headers = [];
	if (options.auth){
		headers.push({ key: 'x-authentication-token', value: options.auth });
	}
	if (options.server){
		headers.push({ key: 'x-server-token', value: options.server });
	}

	const prerequest = [];
	if (options.product){
		prerequest.push(`pm.request.headers.upsert({ key: 'productid', value: pm.variables.replaceIn('${clientProductId(options.product)}') });`);
	}
	else if (options.server || options.bare){
		// A request of another product: the collection names Puma on every request otherwise
		prerequest.push("pm.request.headers.remove('productid');");
	}
	if (options.status){
		prerequest.push(`pm.variables.set('expectedHttpStatus', ${options.status});`);
	}
	if (options.pre){
		prerequest.push(...options.pre);
	}

	const event = [];
	if (prerequest.length){
		event.push({ listen: 'prerequest', script: { type: 'text/javascript', exec: prerequest } });
	}
	event.push({ listen: 'test', script: { type: 'text/javascript', exec: options.test } });

	return {
		name,
		event,
		request: {
			method: 'POST',
			header: headers,
			body: { mode: 'graphql', graphql: { query, variables: '' } },
			url: { raw: '{{base_url}}', host: ['{{base_url}}'] }
		},
		response: []
	};
}

// A refusal: the GraphQL errors carry the reason
function refused(pattern){
	const source = pattern.slice(1, -1).replace(/'(StProbe|StAdded|StNoProduct|StOther|StBySrv|StByUser|StByAnonymous)'/g, (_, id) => `'${clientProductId(id)}'`);
	return [
		'const json = pm.response.json();',
		"const messages = (json.errors || []).map(function (e) { return e.message || ''; }).join(' | ');",
		`pm.test('refused: ${pattern.replace(/\\/g, '').replace(/'/g, '')}', function () { pm.expect(messages, JSON.stringify(json)).to.match(new RegExp(pm.variables.replaceIn(${JSON.stringify(source)}))); });`,
		"pm.test('no data', function () { const data = json.data || {}; Object.keys(data).forEach(function (k) { pm.expect(data[k], k).to.eql(null); }); });"
	];
}

// The data of the command with no GraphQL error
function accepted(command, checks){
	return [
		'const json = pm.response.json();',
		`const payload = json.data && json.data.${command};`,
		`pm.test('${command} answered', function () { pm.expect(json.errors, JSON.stringify(json.errors)).to.eql(undefined); pm.expect(payload, JSON.stringify(json)).to.be.an('object'); });`,
		...(checks || [])
	];
}

const readUsers = 'query UsersList { UsersList(input: { viewParams: { offset: 0, count: 1, filterModel: { textFilter: { text: "{{pbAdminLogin}}", fieldIds: ["name"] } } } }) { items { id } } }';
const readRoles = 'query RolesList { RolesList(input: { viewParams: { offset: 0, count: 1 } }) { items { id } } }';
const appInfo = 'query GetApplicationInfo { GetApplicationInfo(input: {}) { applicationId } }';
const catalogList = 'query ConnectedProductsList { ConnectedProductsList(input: { viewParams: { offset: 0, count: -1 } }) { items { id productId status productVersion tokenCreatedAt lastUsedAt } } }';
const issue = (product, name, scopes) => `mutation CreateServerToken { CreateServerToken(input: { clientProductId: "${product}", name: "${name}", scopes: ${scopes} }) { id token success scopes } }`;
const update = (id, scopes) => `mutation UpdateServerTokenScopes { UpdateServerTokenScopes(input: { id: "${id}", scopes: ${scopes} }) { success scopes } }`;
const probeTree = '{ groups: [{ groupId: "/StProbe", groupName: "Probe", entries: [{ permissionId: "/StProbe/View", displayName: "View the probe" }] }] }';
const publish = (product, version) => `mutation PublishProductPermissions { PublishProductPermissions(input: { productId: "${product}", productVersion: "${version}", tree: ${probeTree} }) { success changed } }`;

const addClient = (product) => `mutation AddServerTokenClient { AddServerTokenClient(input: { clientProductId: "${product}", name: "${product} added" }) { success added } }`;

function catalogEntry(product){
	return [
		'const json = pm.response.json();',
		'const items = (json.data && json.data.ConnectedProductsList && json.data.ConnectedProductsList.items) || [];',
		`const entry = items.find(function (i) { return i.productId === pm.variables.replaceIn('${clientProductId(product)}'); });`,
		`pm.test('the catalog has ${product}', function () { pm.expect(entry, JSON.stringify(items)).to.be.an('object'); });`
	];
}

// ---------------------------------------------------------------- 00E: the servers of ProLife and Lisa

const seedFolder = {
	name: '00E Server tokens of the products',
	description: 'ProLife and Lisa reach Puma through their servers. Their tokens hold every permission a server may get, so the forwarded requests of later folders are decided by the rights of the user.',
	item: [
		item('SRV su ensures ProLife is connected', addClient('ProLife'), {
			auth: '{{su_token}}',
			pre: ["pm.environment.set('product_id', 'Puma');", "['srvProLifeTestTokenId', 'srvLisaTestTokenId', 'srvProLifeToken', 'srvLisaToken'].forEach(function (key) { pm.environment.unset(key); });"],
			test: accepted('AddServerTokenClient', ["pm.test('the client is available without replacing it', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.added).to.be.a('boolean'); });"])
		}),
		item('SRV su ensures Lisa is connected', addClient('Lisa'), {
			auth: '{{su_token}}',
			test: accepted('AddServerTokenClient', ["pm.test('the client is available without replacing it', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.added).to.be.a('boolean'); });"])
		}),
		item('SRV every permission a server may hold', 'query GetServerTokenReceiver { GetServerTokenReceiver(input: { clientProductId: "ProLife" }) { groups { entries { permissionId } } } }', {
			auth: '{{su_token}}',
			test: accepted('GetServerTokenReceiver', [
				'const ids = []; (payload.groups || []).forEach(function (g) { (g.entries || []).forEach(function (e) { ids.push(e.permissionId); }); });',
				"pm.test('the receiver permission leaves are offered', function () { pm.expect(ids).to.include('/Administration/Roles/Edit/Change'); pm.expect(ids).to.include('/Administration/Users/Edit/Change'); });",
				"pm.environment.set('srvAllScopes', JSON.stringify(ids));"
			])
		}),
		item('SRV the token of ProLife', issue('ProLife', 'ProLife for Puma', '{{srvAllScopes}}'), {
			auth: '{{su_token}}',
			test: accepted('CreateServerToken', [
				"pm.test('issued with every permission', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.scopes).to.include.members(JSON.parse(pm.environment.get('srvAllScopes'))); });",
				"pm.environment.set('srvProLifeToken', payload.token);",
				"pm.environment.set('srvProLifeTestTokenId', payload.id);"
			])
		}),
		item('SRV the token of Lisa', issue('Lisa', 'Lisa for Puma', '{{srvAllScopes}}'), {
			auth: '{{su_token}}',
			test: accepted('CreateServerToken', [
				"pm.test('issued with every permission', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.scopes).to.include.members(JSON.parse(pm.environment.get('srvAllScopes'))); });",
				"pm.environment.set('srvLisaToken', payload.token);",
				"pm.environment.set('srvLisaTestTokenId', payload.id);"
			])
		})
	]
};

// ---------------------------------------------------------------- 28: server tokens

const st = [];

// The actors
st.push(item('ST seed: auth su', 'query Authorization { Authorization(input: { login: "{{su_login}}", password: "{{su_password}}", productId: "Puma" }) { token userId } }', {
	pre: ["pm.environment.set('product_id', 'Puma');", "pm.environment.set('stRun', pm.variables.replaceIn('{{$guid}}').replace(/-/g, ''));", "pm.environment.set('stCard', pm.variables.replaceIn('{{$guid}}'));", "pm.environment.set('stCard2', pm.variables.replaceIn('{{$guid}}'));", "['stId', 'stSecondId', 'stEditId', 'stAddedCard', 'stSuPatId', 'stToken', 'stSecondToken', 'stRemovedToken'].forEach(function (key) { pm.environment.unset(key); });"],
	test: [
		'const auth = pm.response.json().data.Authorization;',
		"pm.test('su authorized', function () { pm.expect(auth.token).to.be.a('string').and.not.empty; });",
		"pm.environment.set('su_token', auth.token);",
		"pm.environment.set('st_su_id', auth.userId);"
	]
}));
st.push(item('ST seed: auth an ordinary user', 'query Authorization { Authorization(input: { login: "{{pbAdminLogin}}", password: "Bound_2026!", productId: "Puma" }) { token } }', {
	test: [
		'const auth = pm.response.json().data.Authorization;',
		"pm.test('authorized', function () { pm.expect(auth.token).to.be.a('string').and.not.empty; });",
		"pm.environment.set('stUserToken', auth.token);"
	]
}));

// Catalog
st.push(item('ST: a token for a product outside the catalog is refused', issue('StNoProduct', 'st StNoProduct', '[]'), { auth: '{{su_token}}', test: refused("/The product 'StNoProduct' is not a client of this server/") }));
st.push(item('ST: su adds the product to the catalog', 'mutation ConnectedProductAdd { ConnectedProductAdd(input: { id: "{{stCard}}", typeId: "ConnectedProduct", item: { id: "{{stCard}}", productId: "StProbe", name: "StProbe", description: "catalog probe" } }) { id } }', {
	auth: '{{su_token}}',
	test: accepted('ConnectedProductAdd', ["pm.test('the entry is added', function () { pm.expect(payload.id).to.eql(pm.environment.get('stCard')); });"])
}));
st.push(item('ST: the same product cannot be added twice', 'mutation ConnectedProductAdd { ConnectedProductAdd(input: { id: "{{stCard2}}", typeId: "ConnectedProduct", item: { id: "{{stCard2}}", productId: "StProbe", name: "StProbe again" } }) { id } }', { auth: '{{su_token}}', test: refused("/The product 'StProbe' is already connected/") }));
st.push(item('ST: a product needs an ID', 'mutation ConnectedProductAdd { ConnectedProductAdd(input: { id: "{{stCard2}}", typeId: "ConnectedProduct", item: { id: "{{stCard2}}", name: "No id" } }) { id } }', { auth: '{{su_token}}', test: refused("/Product-ID can.t be empty/") }));
st.push(item('ST: the catalog lists this server and the new product without a token', catalogList, {
	auth: '{{su_token}}',
	test: [
		...catalogEntry('StProbe'),
		"const own = items.find(function (i) { return i.productId === 'Puma'; });",
		"pm.test('Puma is this server', function () { pm.expect(own && own.status, JSON.stringify(items)).to.eql('This server'); });",
		"pm.test('StProbe has no token yet', function () { pm.expect(entry.status).to.eql('No token'); pm.expect(entry.productVersion).to.eql(''); });"
	]
}));
st.push(item('ST: su renames the product', 'mutation ConnectedProductUpdate { ConnectedProductUpdate(input: { id: "{{stCard}}", typeId: "ConnectedProduct", item: { id: "{{stCard}}", productId: "StProbe", name: "StProbe renamed", description: "catalog probe" } }) { id } }', {
	auth: '{{su_token}}',
	test: accepted('ConnectedProductUpdate')
}));
st.push(item('ST: the rename is stored', 'query ConnectedProductItem { ConnectedProductItem(input: { id: "{{stCard}}" }) { id productId name description } }', {
	auth: '{{su_token}}',
	test: accepted('ConnectedProductItem', ["pm.test('renamed', function () { pm.expect(payload.productId).to.eql(pm.variables.replaceIn('StProbe{{stRun}}')); pm.expect(payload.name).to.eql('StProbe renamed'); pm.expect(payload.description).to.eql('catalog probe'); });"])
}));
st.push(item('ST: the product ID cannot be changed', 'mutation ConnectedProductUpdate { ConnectedProductUpdate(input: { id: "{{stCard}}", typeId: "ConnectedProduct", item: { id: "{{stCard}}", productId: "StOther", name: "x" } }) { id } }', { auth: '{{su_token}}', test: refused('/product ID cannot be changed/') }));
st.push(item('ST: an ordinary user does not see the catalog', 'query ConnectedProductsList { ConnectedProductsList(input: { viewParams: { offset: 0, count: 1 } }) { items { id } } }', { auth: '{{stUserToken}}', test: refused('/Invalid permissions for the user/') }));
st.push(item('ST: an ordinary user does not change the catalog', 'mutation ConnectedProductUpdate { ConnectedProductUpdate(input: { id: "{{stCard}}", typeId: "ConnectedProduct", item: { id: "{{stCard}}", productId: "StProbe", name: "hacked" } }) { id } }', { auth: '{{stUserToken}}', test: refused('/Invalid permissions for the user/') }));

// The permissions a token may hold
st.push(item('ST: su reads the permissions a server may get', 'query GetServerTokenReceiver { GetServerTokenReceiver(input: { clientProductId: "StProbe" }) { productId groups { groupId entries { permissionId } } } }', {
	auth: '{{su_token}}',
	test: accepted('GetServerTokenReceiver', [
		'const ids = []; (payload.groups || []).forEach(function (g) { (g.entries || []).forEach(function (e) { ids.push(e.permissionId); }); });',
		"pm.test('this server receives', function () { pm.expect(payload.productId).to.eql('Puma'); });",
		"pm.test('the receiver offers its administration permission leaves', function () { pm.expect(ids).to.include.members(['/Administration/Users/View', '/Administration/Roles/Edit/Change', '/OrganizationManagement/View']); });"
	])
}));
st.push(item('ST: the permissions for an unknown product are refused', 'query GetServerTokenReceiver { GetServerTokenReceiver(input: { clientProductId: "StNoProduct" }) { productId } }', { auth: '{{su_token}}', test: refused("/The product 'StNoProduct' is not a client of this server/") }));
st.push(item('ST: the permissions for this server itself are refused', 'query GetServerTokenReceiver { GetServerTokenReceiver(input: { clientProductId: "Puma" }) { productId } }', { auth: '{{su_token}}', test: refused('/A server token is issued to the server of another product/') }));
st.push(item('ST: an ordinary user does not read the permissions', 'query GetServerTokenReceiver { GetServerTokenReceiver(input: { clientProductId: "StProbe" }) { productId } }', { auth: '{{stUserToken}}', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: anonymous does not read the permissions', 'query GetServerTokenReceiver { GetServerTokenReceiver(input: { clientProductId: "StProbe" }) { productId } }', { test: refused('/Server tokens are managed by a superuser session/') }));

// Issuing
st.push(item('ST: su issues a token of StProbe for Puma', issue('StProbe', 'st StProbe for Puma', '["/Administration/Users/View"]'), {
	auth: '{{su_token}}',
	test: accepted('CreateServerToken', [
		"pm.test('issued', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.token).to.match(/^imt_srv_/); });",
		"pm.test('only the requested permission is held', function () { pm.expect(payload.scopes).to.have.members(['/Administration/Users/View']); });",
		"pm.environment.set('stId', payload.id);",
		"pm.environment.set('stToken', payload.token);"
	])
}));
st.push(item('ST: the catalog shows a product that has never connected', catalogList, {
	auth: '{{su_token}}',
	test: [...catalogEntry('StProbe'), "pm.test('never connected', function () { pm.expect(entry.status).to.eql('Never connected'); pm.expect(entry.tokenCreatedAt).to.match(/^\\d{2}\\.\\d{2}\\.\\d{4}$/); pm.expect(entry.lastUsedAt).to.eql(undefined); });"]
}));
st.push(item('ST: su issues a second token of StProbe', issue('StProbe', 'st StProbe for Puma, second', '[]'), {
	auth: '{{su_token}}',
	test: accepted('CreateServerToken', [
		"pm.test('empty scopes grant no product permissions', function () { pm.expect(payload.scopes).to.eql([]); });",
		"pm.environment.set('stSecondId', payload.id);",
		"pm.environment.set('stSecondToken', payload.token);"
	])
}));
st.push(item('ST: an edit permission brings its view permission', issue('StProbe', 'st edit', '["/Administration/Roles/Edit"]'), {
	auth: '{{su_token}}',
	test: accepted('CreateServerToken', [
		"pm.test('Roles/Edit expands to its leaves and requires Roles/View', function () { pm.expect(payload.scopes).to.have.members(['/Administration/Roles/View', '/Administration/Roles/Edit/Add', '/Administration/Roles/Edit/Change', '/Administration/Roles/Edit/Remove']); });",
		"pm.environment.set('stEditId', payload.id);"
	])
}));
st.push(item('ST: a token needs a name', issue('StProbe', '', '[]'), { auth: '{{su_token}}', test: refused('/The name of the token is required/') }));
st.push(item('ST: an unknown permission is refused', issue('StProbe', 'st unknown', '["/StNoSuchPermission"]'), { auth: '{{su_token}}', test: refused("/The permission '\\/StNoSuchPermission' cannot be given to a server token/") }));
st.push(item('ST: a permission outside the receiver tree is refused', issue('StProbe', 'st foreign permission', '["/StProbe/View"]'), { auth: '{{su_token}}', test: refused("/The permission '\\/StProbe\\/View' cannot be given to a server token/") }));
st.push(item('ST: a token for the own server is refused', issue('Puma', 'st Puma', '[]'), { auth: '{{su_token}}', test: refused('/A server token is issued to the server of another product/') }));
st.push(item('ST: an ordinary user is refused', issue('StProbe', 'st user', '[]'), { auth: '{{stUserToken}}', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: anonymous is refused', issue('StProbe', 'st anonymous', '[]'), { test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: su PAT seed', 'mutation CreateToken { CreateToken(input: { userId: "{{st_su_id}}", productId: "Puma", name: "st su pat {{stRun}}", scopes: ["/StAny"] }) { token success } }', {
	auth: '{{su_token}}',
	test: accepted('CreateToken', ["pm.test('created', function () { pm.expect(payload.success).to.eql(true); });", "pm.environment.set('stSuPat', payload.token);"])
}));
st.push(item('ST: a personal token of su cannot issue', issue('StProbe', 'st pat', '[]'), { auth: '{{stSuPat}}', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: the list shows the tokens of the product', 'query GetServerTokenList { GetServerTokenList(input: { clientProductId: "StProbe" }) { tokens { id name clientProductId productId scopes revoked createdAt lastUsedAt } } }', {
	auth: '{{su_token}}',
	test: accepted('GetServerTokenList', [
		'const tokens = payload.tokens || [];',
		"const first = tokens.find(function (t) { return t.id === pm.environment.get('stId'); });",
		"pm.test('the three tokens of StProbe', function () { pm.expect(tokens.map(function (t) { return t.id; })).to.have.members([pm.environment.get('stId'), pm.environment.get('stSecondId'), pm.environment.get('stEditId')]); });",
		"pm.test('a token names both products and its permissions', function () { pm.expect(first.clientProductId).to.eql(pm.variables.replaceIn('StProbe{{stRun}}')); pm.expect(first.productId).to.eql('Puma'); pm.expect(first.revoked).to.eql(false); pm.expect(first.scopes).to.include('/Administration/Users/View'); pm.expect(first.lastUsedAt).to.eql(''); });"
	])
}));
st.push(item('ST: the list is for su only', 'query GetServerTokenList { GetServerTokenList(input: { clientProductId: "StProbe" }) { tokens { id } } }', { auth: '{{stUserToken}}', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: ValidateToken names both products', 'query ValidateToken { ValidateToken(input: { token: "{{stToken}}" }) { valid userId productId clientProductId scopes } }', {
	auth: '{{su_token}}',
	test: accepted('ValidateToken', ["pm.test('a token of the server of StProbe for Puma, owned by nobody', function () { pm.expect(payload.valid).to.eql(true); pm.expect(payload.productId).to.eql('Puma'); pm.expect(payload.clientProductId).to.eql(pm.variables.replaceIn('StProbe{{stRun}}')); pm.expect(payload.userId || '').to.eql(''); });"])
}));

// Using the token: the server alone
st.push(item('ST: the server reads what is open to everybody', appInfo, {
	server: '{{stToken}}',
	test: accepted('GetApplicationInfo', ["pm.test('Puma answers', function () { pm.expect(payload.applicationId).to.eql('Puma'); });"])
}));
st.push(item('ST: the catalog shows the product online once its server is heard', catalogList, {
	auth: '{{su_token}}',
	test: [...catalogEntry('StProbe'), "pm.test('online', function () { pm.expect(entry.status).to.eql('Online'); pm.expect(entry.lastUsedAt).to.match(/^\\d{2}\\.\\d{2}\\.\\d{4} \\d{2}:\\d{2}:\\d{2}$/); });"]
}));
st.push(item('ST: a service principal reads with its own view permission', readUsers, {
	auth: '{{stToken}}', product: 'StProbe',
	test: accepted('UsersList', ["pm.test('the ordinary user is read', function () { pm.expect(payload.items.map(function (i) { return i.id; })).to.eql([pm.environment.get('pbAdmin')]); });"])
}));
st.push(item('ST: a reading outside the permissions is refused', readRoles, { auth: '{{stToken}}', product: 'StProbe', test: refused("/Invalid permissions for the user 'StProbe'/") }));
st.push(item('ST: a change outside the permissions is refused', 'mutation UserAdd { UserAdd(input: { id: "{{stCard2}}", typeId: "User", productId: "StProbe", item: { username: "st" } }) { id } }', { auth: '{{stToken}}', product: 'StProbe', test: refused("/Invalid permissions for the user 'StProbe'/") }));
st.push(item('ST: a collection read follows the view permission of the collection', 'query GetElementIds { GetElementIds(input: { collectionId: "Roles", offset: 0, count: 1 }) { elementIds } }', { auth: '{{stToken}}', product: 'StProbe', test: refused("/Invalid permissions for the user 'StProbe'/") }));
st.push(item('ST: empty scopes do not read protected users', readUsers, { auth: '{{stSecondToken}}', product: 'StProbe', test: refused("/Invalid permissions for the user 'StProbe'/") }));
st.push(item('ST: a service without organization creation permission cannot create one', 'mutation CreateTenant { CreateTenant(input: { name: "st forbidden organization" }) { tenantId errorMessage } }', {
	auth: '{{stToken}}', product: 'StProbe',
	test: accepted('CreateTenant', ["pm.test('organization creation refused', function () { pm.expect(payload.tenantId || '').to.eql(''); pm.expect(payload.errorMessage).to.eql('Permission denied: /OrganizationManagement/Add'); });"])
}));
st.push(item('ST: an anonymous forwarded request has no service principal', readUsers, { server: '{{stToken}}', test: refused("/Invalid permissions for the user ''/") }));
st.push(item('ST: a server does not add clients', addClient('StBySrv'), { auth: '{{stToken}}', product: 'StProbe', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: a server validates a token without an extra scope', 'query ValidateToken { ValidateToken(input: { token: "{{stSecondToken}}" }) { valid clientProductId } }', {
	auth: '{{stSecondToken}}', product: 'StProbe',
	test: accepted('ValidateToken', ["pm.test('the token is validated for the connected product', function () { pm.expect(payload.valid).to.eql(true); pm.expect(payload.clientProductId).to.eql(pm.variables.replaceIn('StProbe{{stRun}}')); });"])
}));
st.push(item('ST: the token is refused under the header of another product', appInfo, { server: '{{stToken}}', product: 'Lisa', status: 403, test: refused("/The token of the server is not the token of the product 'Lisa'/") }));
st.push(item('ST: the token is refused under the header of Puma', appInfo, { server: '{{stToken}}', product: 'Puma', status: 403, test: refused("/The token of the server is not the token of the product 'Puma'/") }));
st.push(item('ST: a value that is no token is refused', appInfo, { server: 'not-a-token', status: 403, test: refused('/The token of the server is not valid/') }));
st.push(item('ST: an unknown token is refused', appInfo, { server: 'imt_srv_000000000000000000000000000000', status: 403, test: refused('/Invalid personal access token/') }));
st.push(item('ST: a server token cannot issue tokens', issue('StProbe', 'st by a server', '[]'), { auth: '{{stToken}}', bare: true, product: 'StProbe', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: a server token as the token of its own requests is refused for another product', appInfo, { auth: '{{stToken}}', bare: true, product: 'Lisa', status: 403, test: refused('/The personal access token is not valid for this product/') }));

// The token of a user travelling with the token of the server
// A PAT carries an explicit boundary; its scopes must include the receiver leaves used by both positive probes.
st.push(item('ST seed: personal token of su for StProbe', 'mutation CreateToken { CreateToken(input: { userId: "{{st_su_id}}", productId: "StProbe", name: "st su pat for StProbe", scopes: ["/Administration/Users/View", "/Administration/Roles/View"] }) { token success } }', {
	auth: '{{su_token}}',
	test: accepted('CreateToken', ["pm.test('created', function () { pm.expect(payload.success).to.eql(true); });", "pm.environment.set('stProbePat', payload.token);"])
}));
st.push(item('ST seed: personal token of su for Lisa', 'mutation CreateToken { CreateToken(input: { userId: "{{st_su_id}}", productId: "Lisa", name: "st su pat for Lisa", scopes: ["/Administration"] }) { token success } }', {
	auth: '{{su_token}}',
	test: accepted('CreateToken', ["pm.test('created', function () { pm.expect(payload.success).to.eql(true); });", "pm.environment.set('stLisaPat', payload.token);"])
}));
st.push(item('ST: a personal token comes through the server of its product', readUsers, {
	auth: '{{stProbePat}}', bare: true, server: '{{stToken}}',
	test: accepted('UsersList', ["pm.test('the ordinary user is read within both token boundaries', function () { pm.expect(payload.items.map(function (i) { return i.id; })).to.eql([pm.environment.get('pbAdmin')]); });"])
}));
st.push(item('ST: the permissions of the server bound even a superuser PAT', readRoles, { auth: '{{stProbePat}}', bare: true, server: '{{stToken}}', test: refused('/Invalid permissions for the user/') }));
st.push(item('ST seed: superuser session opened through the product server', 'query Authorization { Authorization(input: { login: "{{su_login}}", password: "{{su_password}}", productId: "StProbe" }) { token } }', {
	server: '{{stToken}}', product: 'StProbe',
	test: accepted('Authorization', ["pm.test('the product session is issued', function () { pm.expect(payload.token).to.be.a('string').and.not.empty; });", "pm.environment.set('stForwardedSuToken', payload.token);"])
}));
st.push(item('ST: a forwarded superuser session reads within the server boundary', readUsers, {
	auth: '{{stForwardedSuToken}}', server: '{{stToken}}', product: 'StProbe',
	test: accepted('UsersList', ["pm.test('the granted view permission works', function () { pm.expect(payload.items.map(function (i) { return i.id; })).to.eql([pm.environment.get('pbAdmin')]); });"])
}));
st.push(item('ST: a forwarded superuser session is not a global administrator', readRoles, { auth: '{{stForwardedSuToken}}', server: '{{stToken}}', product: 'StProbe', test: refused('/Invalid permissions for the user/') }));
st.push(item('ST: a forwarded superuser session cannot manage server tokens', issue('StProbe', 'st forwarded su', '[]'), { auth: '{{stForwardedSuToken}}', server: '{{stToken}}', product: 'StProbe', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: a personal token of another product does not come through this server', readUsers, { auth: '{{stLisaPat}}', bare: true, server: '{{stToken}}', status: 403, test: refused('/The personal access token is not for the product of the server/') }));
st.push(item('ST: a personal token of Puma does not come through the server of another product', readUsers, { auth: '{{stSuPat}}', server: '{{stToken}}', status: 403, test: refused('/The personal access token is not for the product of the server/') }));
st.push(item('ST: a personal token of another product is refused without the token of its server', readUsers, { auth: '{{stProbePat}}', bare: true, status: 403, test: refused("/The product 'StProbe' is not connected: its server sends no valid token/") }));
st.push(item('ST: the same with the header of its product', readUsers, { auth: '{{stProbePat}}', bare: true, product: 'StProbe', status: 403, test: refused("/The product 'StProbe' is not connected: its server sends no valid token/") }));

// Another product without the token of its server
st.push(item('ST: a superuser session works for another product', 'query RolesList { RolesList(input: { viewParams: { offset: 0, count: 1 } }) { items { id } } }', {
	auth: '{{su_token}}', product: 'StProbe',
	test: accepted('RolesList', ["pm.test('the console of Puma administers the product', function () { pm.expect(payload.items).to.be.an('array'); });"])
}));
st.push(item('ST: a personal token of su does not work for another product', readRoles, { auth: '{{stSuPat}}', product: 'StProbe', status: 403, test: refused('/The personal access token is not valid for this product/') }));
st.push(item('ST: an ordinary user does not work for another product', readRoles, { auth: '{{stUserToken}}', product: 'StProbe', status: 403, test: refused("/The product 'StProbe' is not connected: its server sends no valid token/") }));
st.push(item('ST: anonymous does not work for another product', appInfo, { product: 'StProbe', status: 403, test: refused("/The product 'StProbe' is not connected: its server sends no valid token/") }));

// The permissions change while the token is in use
st.push(item('ST: su gives the token the view of roles', update('{{stId}}', '["/Administration/Users/View", "/Administration/Roles/View"]'), {
	auth: '{{su_token}}',
	test: accepted('UpdateServerTokenScopes', ["pm.test('changed', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.scopes).to.have.members(['/Administration/Users/View', '/Administration/Roles/View']); });"])
}));
st.push(item('ST: the new permission takes effect at once', readRoles, {
	auth: '{{stProbePat}}', bare: true, server: '{{stToken}}',
	test: accepted('RolesList', ["pm.test('the roles are read', function () { pm.expect(payload.items).to.be.an('array'); });"])
}));
st.push(item('ST: su takes the view of users away', update('{{stId}}', '["/Administration/Roles/View"]'), {
	auth: '{{su_token}}',
	test: accepted('UpdateServerTokenScopes', ["pm.test('changed', function () { pm.expect(payload.scopes).to.have.members(['/Administration/Roles/View']); });"])
}));
st.push(item('ST: the removed permission is refused at once', readUsers, { auth: '{{stProbePat}}', bare: true, server: '{{stToken}}', test: refused('/Invalid permissions for the user/') }));
st.push(item('ST: an unknown permission is not given', update('{{stId}}', '["/StNoSuchPermission"]'), { auth: '{{su_token}}', test: refused("/The permission '\\/StNoSuchPermission' cannot be given to a server token/") }));
st.push(item('ST: the permissions of an unknown token are not changed', update('{{stCard2}}', '[]'), { auth: '{{su_token}}', test: refused('/The server token does not exist/') }));
st.push(item('ST: the permissions of a personal token are not changed here', 'mutation UpdateServerTokenScopes { UpdateServerTokenScopes(input: { id: "{{stSuPatId}}", scopes: [] }) { success } }', {
	auth: '{{su_token}}',
	pre: ["pm.sendRequest({ url: pm.environment.get('base_url'), method: 'POST', header: { 'Content-Type': 'application/json', 'x-authentication-token': pm.environment.get('su_token'), 'productid': 'Puma' }, body: { mode: 'raw', raw: JSON.stringify({ query: 'query GetTokenList { GetTokenList(input: { userId: \"' + pm.environment.get('st_su_id') + '\" }) { tokens { id name } } }' }) } }, function (err, res) { const tokens = (res && res.json().data && res.json().data.GetTokenList && res.json().data.GetTokenList.tokens) || []; const pat = tokens.find(function (t) { return t.name === pm.variables.replaceIn('st su pat {{stRun}}'); }); pm.environment.set('stSuPatId', pat ? pat.id : 'missing'); });"],
	test: ["pm.test('a personal token is named', function () { pm.expect(pm.environment.get('stSuPatId')).to.be.a('string').and.not.empty; });", ...refused('/The server token does not exist/')]
}));
st.push(item('ST: an ordinary user does not change the permissions', update('{{stId}}', '[]'), { auth: '{{stUserToken}}', test: refused('/Server tokens are managed by a superuser session/') }));

// The permissions of this server given to the products
st.push(item('ST: the server of a product reads the permissions of Puma for its roles', 'query GetServerTokenFeatures { GetServerTokenFeatures { features { featureId subFeatures { featureId } } } }', {
	auth: '{{stToken}}', bare: true,
	test: accepted('GetServerTokenFeatures', ["pm.test('the administration of Puma', function () { const ids = payload.features.map(function (f) { return f.featureId; }); pm.expect(ids).to.include('Administration'); });"])
}));
st.push(item('ST: a superuser session does not read them', 'query GetServerTokenFeatures { GetServerTokenFeatures { features { featureId } } }', { auth: '{{su_token}}', test: refused('/The permissions of this server are given to the servers of products/') }));
st.push(item('ST: anonymous does not read them', 'query GetServerTokenFeatures { GetServerTokenFeatures { features { featureId } } }', { test: refused('/The permissions of this server are given to the servers of products/') }));

// Publishing the permissions of a product
st.push(item('ST: the server of the product publishes its permissions with empty scopes', publish('StProbe', '7.1.0'), {
	auth: '{{stSecondToken}}', bare: true,
	test: accepted('PublishProductPermissions', ["pm.test('stored', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.changed).to.eql(true); });"])
}));
st.push(item('ST: the same tree again changes nothing', publish('StProbe', '7.1.0'), {
	auth: '{{stToken}}', bare: true,
	test: accepted('PublishProductPermissions', ["pm.test('unchanged', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.changed).to.eql(false); });"])
}));
st.push(item('ST: a server does not publish for another product', publish('StOther', '1'), { auth: '{{stToken}}', bare: true, test: refused("/The permissions of the product 'StOther' are published by its server only/") }));
st.push(item('ST: a forwarded request does not publish', publish('StProbe', '7.1.1'), { server: '{{stToken}}', test: refused("/The permissions of the product 'StProbe' are published by its server only/") }));
st.push(item('ST: su does not publish for a product', publish('StProbe', '7.1.1'), { auth: '{{su_token}}', test: refused("/The permissions of the product 'StProbe' are published by its server only/") }));
st.push(item('ST: the catalog shows the published version', catalogList, {
	auth: '{{su_token}}',
	test: [...catalogEntry('StProbe'), "pm.test('version 7.1.0', function () { pm.expect(entry.productVersion).to.eql('7.1.0'); });"]
}));
st.push(item('ST: su reads the published permissions with those of Puma', 'query GetProductPermissions { GetProductPermissions(input: { productId: "StProbe" }) { groups { groupId entries { permissionId } } } }', {
	auth: '{{su_token}}',
	test: accepted('GetProductPermissions', [
		'const ids = []; (payload.groups || []).forEach(function (g) { (g.entries || []).forEach(function (e) { ids.push(e.permissionId); }); });',
		"pm.test('the published tree', function () { pm.expect(ids).to.include('/StProbe/View'); });",
		"pm.test('the administration of Puma for the roles of the product', function () { pm.expect(ids.filter(function (id) { return id.indexOf('/Administration/') === 0; })).to.not.be.empty; });"
	])
}));

// Revoking
st.push(item('ST: an unknown token is not revoked', 'mutation RevokeServerToken { RevokeServerToken(input: { id: "{{stCard2}}" }) { success } }', { auth: '{{su_token}}', test: refused('/The server token does not exist/') }));
st.push(item('ST: an ordinary user does not revoke', 'mutation RevokeServerToken { RevokeServerToken(input: { id: "{{stId}}" }) { success } }', { auth: '{{stUserToken}}', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: su revokes', 'mutation RevokeServerToken { RevokeServerToken(input: { id: "{{stSecondId}}" }) { success } }', {
	auth: '{{su_token}}',
	test: accepted('RevokeServerToken', ["pm.test('revoked', function () { pm.expect(payload.success).to.eql(true); });"])
}));
st.push(item('ST: a revoked token is refused at once', appInfo, { server: '{{stSecondToken}}', status: 403, test: refused('/Invalid personal access token/') }));
st.push(item('ST: a revoked token is reported invalid', 'query ValidateToken { ValidateToken(input: { token: "{{stSecondToken}}" }) { valid } }', {
	auth: '{{su_token}}',
	test: accepted('ValidateToken', ["pm.test('invalid', function () { pm.expect(payload.valid).to.eql(false); });"])
}));
st.push(item('ST: the permissions of a revoked token are not changed', update('{{stSecondId}}', '[]'), { auth: '{{su_token}}', test: refused('/The server token is revoked/') }));
st.push(item('ST: the list marks the revoked token', 'query GetServerTokenList { GetServerTokenList(input: { clientProductId: "StProbe" }) { tokens { id revoked lastUsedAt } } }', {
	auth: '{{su_token}}',
	test: accepted('GetServerTokenList', [
		'const byId = {}; (payload.tokens || []).forEach(function (t) { byId[t.id] = t; });',
		"pm.test('revoked and in use', function () { pm.expect(byId[pm.environment.get('stSecondId')].revoked).to.eql(true); pm.expect(byId[pm.environment.get('stId')].revoked).to.eql(false); pm.expect(byId[pm.environment.get('stId')].lastUsedAt).to.not.eql(''); });"
	])
}));

// Deleting: a revoked token only
const deleteToken = (id) => `mutation DeleteServerToken { DeleteServerToken(input: { id: "${id}" }) { success } }`;
st.push(item('ST: a token in use is not deleted', deleteToken('{{stId}}'), { auth: '{{su_token}}', test: refused('/Revoke the server token before deleting it/') }));
st.push(item('ST: an ordinary user does not delete', deleteToken('{{stSecondId}}'), { auth: '{{stUserToken}}', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: su deletes a revoked token', deleteToken('{{stSecondId}}'), {
	auth: '{{su_token}}',
	test: accepted('DeleteServerToken', ["pm.test('deleted', function () { pm.expect(payload.success).to.eql(true); });"])
}));
st.push(item('ST: the list has no deleted token', 'query GetServerTokenList { GetServerTokenList(input: { clientProductId: "StProbe" }) { tokens { id } } }', {
	auth: '{{su_token}}',
	test: accepted('GetServerTokenList', [
		'const ids = (payload.tokens || []).map(function (t) { return t.id; });',
		"pm.test('gone, the others stay', function () { pm.expect(ids).to.not.include(pm.environment.get('stSecondId')); pm.expect(ids).to.include(pm.environment.get('stId')); });"
	])
}));
st.push(item('ST: a deleted token is refused', appInfo, { server: '{{stSecondToken}}', status: 403, test: refused('/Invalid personal access token/') }));
st.push(item('ST: a deleted token is not deleted again', deleteToken('{{stSecondId}}'), { auth: '{{su_token}}', test: refused('/The server token does not exist/') }));
st.push(item('ST: su revokes the remaining tokens', 'mutation RevokeServerToken { RevokeServerToken(input: { id: "{{stEditId}}" }) { success } }', {
	auth: '{{su_token}}',
	pre: ["pm.sendRequest({ url: pm.environment.get('base_url'), method: 'POST', header: { 'Content-Type': 'application/json', 'x-authentication-token': pm.environment.get('su_token'), 'productid': 'Puma' }, body: { mode: 'raw', raw: JSON.stringify({ query: 'mutation RevokeServerToken { RevokeServerToken(input: { id: \"' + pm.environment.get('stId') + '\" }) { success } }' }) } }, function (err, res) { pm.environment.set('stFirstRevoked', String(!err && res.json().data && res.json().data.RevokeServerToken && res.json().data.RevokeServerToken.success)); });"],
	test: accepted('RevokeServerToken', ["pm.test('both revoked', function () { pm.expect(payload.success).to.eql(true); pm.expect(pm.environment.get('stFirstRevoked')).to.eql('true'); });"])
}));
st.push(item('ST: a product with only revoked tokens has no token', catalogList, {
	auth: '{{su_token}}',
	test: [...catalogEntry('StProbe'), "pm.test('no token', function () { pm.expect(entry.status).to.eql('No token'); pm.expect(entry.productVersion).to.eql('7.1.0'); });"]
}));

// A configurator (Agentino) makes a product a client before it issues the token
st.push(item('ST: su adds a client product', addClient('StAdded'), {
	auth: '{{su_token}}',
	test: accepted('AddServerTokenClient', ["pm.test('added now', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.added).to.eql(true); });"])
}));
st.push(item('ST: adding a client again changes nothing', addClient('StAdded'), {
	auth: '{{su_token}}',
	test: accepted('AddServerTokenClient', ["pm.test('already a client', function () { pm.expect(payload.success).to.eql(true); pm.expect(payload.added).to.eql(false); });"])
}));
st.push(item('ST: the added client is in the catalog without a token', catalogList, {
	auth: '{{su_token}}',
	test: [...catalogEntry('StAdded'),
		"pm.test('once and without a token', function () { pm.expect(entry.status).to.eql('No token'); pm.expect(items.filter(function (i) { return i.productId === pm.variables.replaceIn('StAdded{{stRun}}'); }).length).to.eql(1); });",
		"pm.environment.set('stAddedCard', entry ? entry.id : 'missing');"]
}));
st.push(item('ST: the added client gets a token', issue('StAdded', 'st added', '[]'), {
	auth: '{{su_token}}',
	test: accepted('CreateServerToken', ["pm.test('issued', function () { pm.expect(payload.success).to.eql(true); });"])
}));
st.push(item('ST: this server is not its own client', addClient('Puma'), { auth: '{{su_token}}', test: refused('/A server token is issued to the server of another product/') }));
st.push(item('ST: a client needs an ID', addClient(''), { auth: '{{su_token}}', test: refused('/A server token is issued to the server of another product/') }));
st.push(item('ST: an ordinary user does not add clients', addClient('StByUser'), { auth: '{{stUserToken}}', test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: anonymous does not add clients', addClient('StByAnonymous'), { test: refused('/Server tokens are managed by a superuser session/') }));
st.push(item('ST: the refused requests added nothing', catalogList, {
	auth: '{{su_token}}',
	test: ['const json = pm.response.json();', 'const ids = ((json.data && json.data.ConnectedProductsList && json.data.ConnectedProductsList.items) || []).map(function (i) { return i.productId; });',
		"pm.test('no refused product is in the catalog', function () { ['StBySrv', 'StByUser', 'StByAnonymous'].forEach(function (id) { pm.expect(ids).to.not.include(id + pm.environment.get('stRun')); }); pm.expect(ids).to.not.include(''); });"]
}));
st.push(item('ST cleanup: the added client is removed', 'mutation RemoveElements { RemoveElements(input: { collectionId: "ConnectedProducts", elementIds: ["{{stAddedCard}}"] }) { success } }', {
	auth: '{{su_token}}',
	test: accepted('RemoveElements', ["pm.test('removed', function () { pm.expect(payload.success).to.eql(true); });"])
}));

// Removing the product
st.push(item('ST seed: a token of the product to remove', issue('StProbe', 'st before removal', '[]'), {
	auth: '{{su_token}}',
	test: accepted('CreateServerToken', ["pm.environment.set('stRemovedToken', payload.token);"])
}));
st.push(item('ST: removing the product revokes its tokens', 'mutation RemoveElements { RemoveElements(input: { collectionId: "ConnectedProducts", elementIds: ["{{stCard}}"] }) { success } }', {
	auth: '{{su_token}}',
	test: accepted('RemoveElements', ["pm.test('removed', function () { pm.expect(payload.success).to.eql(true); });"])
}));
st.push(item('ST: the token of a removed product is invalid', 'query ValidateToken { ValidateToken(input: { token: "{{stRemovedToken}}" }) { valid } }', {
	auth: '{{su_token}}',
	test: accepted('ValidateToken', ["pm.test('invalid', function () { pm.expect(payload.valid).to.eql(false); });"])
}));
st.push(item('ST: the server of a removed product is refused', appInfo, { server: '{{stRemovedToken}}', status: 403, test: refused('/Invalid personal access token/') }));

for (const product of ['ProLife', 'Lisa']){
	st.push(item(`ST cleanup: revoke only this suite's ${product} token`, `mutation RevokeServerToken { RevokeServerToken(input: { id: "{{srv${product}TestTokenId}}" }) { success } }`, {
		auth: '{{su_token}}',
		test: accepted('RevokeServerToken', ["pm.test('the suite token is revoked without removing its client', function () { pm.expect(payload.success).to.eql(true); });"])
	}));
}

// ---------------------------------------------------------------- apply

const collection = JSON.parse(fs.readFileSync(collectionPath, 'utf8'));
const folders = collection.item;

// Never reuse the fixed bootstrap accounts from a previous run on a persistent database.
const bootstrap = folders.find((f) => f.name === '00 System bootstrap').item.find((i) => i.name === 'Ensure superuser exists (su)');
const bootstrapPre = bootstrap.event.find((e) => e.listen === 'prerequest').script.exec;
for (const line of [
	"pm.environment.set('fixture_run', pm.variables.replaceIn('{{$guid}}').replace(/-/g, ''));",
	"['a_owner', 'b_owner', 'a_member', 'outsider'].forEach(function (name) { pm.environment.set(name + '_login', name + '_' + pm.environment.get('fixture_run')); });"
]){
	if (!bootstrapPre.includes(line)){
		bootstrapPre.push(line);
	}
}
for (const registration of folders.find((f) => f.name === '00B Register users').item){
	const body = registration.request.body.graphql;
	body.query = body.query
		.replace(/username: "(a_owner|b_owner|a_member|outsider)"/g, (_, name) => `username: "{{${name}_login}}"`)
		.replace(/email: "(a_owner|b_owner|a_member|outsider)@test\.local"/g, (_, name) => `email: "{{${name}_login}}@test.local"`);
}

const seedIndex = folders.findIndex((f) => f.name === seedFolder.name);
if (seedIndex >= 0){
	folders[seedIndex] = seedFolder;
}
else{
	folders.splice(folders.findIndex((f) => f.name.startsWith('00C')) + 1, 0, seedFolder);
}

const tokensFolder = folders.find((f) => f.name === '28 Server tokens');
tokensFolder.item = st;

// The ProLife user signs in through the server of ProLife
const signIn = folders.find((f) => f.name.startsWith('18 ')).item.find((i) => i.name === 'PI the ProLife-only user holds the ProLife role permissions in ProLife');
signIn.request.header = [{ key: 'x-server-token', value: '{{srvProLifeToken}}' }];

// A forged product header against the request through the server of that product
const boundary = folders.find((f) => f.name.startsWith('27 '));
const forgedIndex = boundary.item.findIndex((i) => i.name === 'PB role of another product: forged product header');
const forged = boundary.item[forgedIndex];
const forgedQuery = forged.request.body.graphql.query;
boundary.item[forgedIndex] = item('PB role of another product: forged product header', forgedQuery, {
	auth: '{{pbAdminToken}}', product: 'Lisa', status: 403,
	test: refused("/The product 'Lisa' is not connected: its server sends no valid token/")
});
const lisaSession = item('PB role of another product: authorize through its server',
	'query Authorization { Authorization(input: { login: "{{pbAdminLogin}}", password: "Bound_2026!", productId: "Lisa" }) { token } }', {
		server: '{{srvLisaToken}}', product: 'Lisa',
		test: accepted('Authorization', ["pm.test('the Lisa-origin session is issued', function () { pm.expect(payload.token).to.be.a('string').and.not.empty; });", "pm.environment.set('pbLisaToken', payload.token);"])
	});
const throughLisa = item('PB role of another product: through the server of that product', forgedQuery, {
	auth: '{{pbLisaToken}}', server: '{{srvLisaToken}}', product: 'Lisa',
	test: refused('/Invalid permissions for the user/')
});
boundary.item = boundary.item.filter((i) => i.name !== lisaSession.name && i.name !== throughLisa.name);
boundary.item.splice(forgedIndex + 1, 0, lisaSession, throughLisa);

fs.writeFileSync(collectionPath, JSON.stringify(collection, null, 4).replace(/\n/g, '\r\n'));
console.log(`00E: ${seedFolder.item.length} requests, 28: ${st.length} requests`);
