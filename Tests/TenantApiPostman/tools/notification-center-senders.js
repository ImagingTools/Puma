// Moves Notification_Center.postman_collection.json to the senders Puma accepts: the server of a product with its own
// token (ImtCore#1083). The notifications of a product are read through the server of that product, as its clients do.
// Run once after editing: "node tools/notification-center-senders.js"; a second run changes nothing.
const fs = require('fs');
const path = require('path');

const collectionPath = path.join(__dirname, '..', 'Notification_Center.postman_collection.json');
const collection = JSON.parse(fs.readFileSync(collectionPath, 'utf8'));
const folder = (prefix) => collection.item.find((f) => f.name.startsWith(prefix));
const prelude = folder('00').item[0].event.find((e) => e.listen === 'test').script.exec.join('\n').split('\n');
const preludeLength = prelude.indexOf("function argumentOf(item, id) { return ((item && item.arguments) || []).filter(function (a) { return a.id === id; })[0]; }") + 1;
const shared = prelude.slice(0, preludeLength);

const inboxCommands = ['NotificationsList', 'NotificationsSync', 'NotificationsUnreadCount', 'MarkNotificationsRead', 'MarkAllNotificationsRead',
	'RemoveNotifications', 'AcknowledgeNotificationsDelivered', 'AcknowledgeAllNotificationsDelivered', 'UserNotificationsList', 'RemoveElements'];

function commandOf(item){
	const query = item.request.body && item.request.body.graphql ? item.request.body.graphql.query : '';
	return (query.match(/^\s*(?:query|mutation)\s+(\w+)/) || [])[1] || '';
}

function setHeader(item, key, value){
	const headers = item.request.header || (item.request.header = []);
	const found = headers.find((h) => h.key === key);
	if (found){
		found.value = value;
	}
	else{
		headers.push({ key, value });
	}
}

function setProduct(item, product){
	let pre = (item.event || []).find((e) => e.listen === 'prerequest');
	if (!pre){
		pre = { listen: 'prerequest', script: { type: 'text/javascript', exec: [] } };
		item.event.unshift(pre);
	}
	const line = `pm.request.headers.upsert({ key: 'productid', value: '${product}' });`;
	pre.script.exec = pre.script.exec.filter((l) => !l.includes("key: 'productid'"));
	pre.script.exec.unshift(line);
}

function addPre(item, line){
	const pre = (item.event || []).find((e) => e.listen === 'prerequest');
	if (!pre.script.exec.includes(line)){
		pre.script.exec.push(line);
	}
}

function setTest(item, own){
	const test = item.event.find((e) => e.listen === 'test');
	test.script.exec = [shared.concat(own).join('\n')];
}

function setQuery(item, query){
	item.request.body.graphql.query = query;
}

function newItem(name, query, headers, own, product){
	const item = {
		name,
		event: [{ listen: 'test', script: { type: 'text/javascript', exec: [shared.concat(own).join('\n')] } }],
		request: { method: 'POST', header: headers, body: { mode: 'graphql', graphql: { query, variables: '' } }, url: { raw: '{{base_url}}', host: ['{{base_url}}'] } },
		response: []
	};
	if (product){
		setProduct(item, product);
	}
	return item;
}

// The sender sends: its token as the token of the request, its product named in the header and the input
function sentByServer(item, token = '{{nc_server_token}}', product = '{{nc_product}}'){
	setHeader(item, 'x-authentication-token', token);
	setProduct(item, product);
	setQuery(item, item.request.body.graphql.query.replace(/productId: "(Puma|ProLife|\{\{nc_product\}\}|\{\{nc_other\}\})"/, `productId: "${product}"`));
}

// A user reads the notifications of the sender through the server of the sender
function readThroughServer(item, token = '{{nc_server_token}}', product = '{{nc_product}}'){
	setHeader(item, 'x-server-token', token);
	setProduct(item, product);
}

// ---------------------------------------------------------------- 00 Bootstrap: the senders

const bootstrap = folder('00');
bootstrap.item = bootstrap.item.filter((i) => i.name !== 'Authorization alice in product ProLife');
const senderItems = [
	newItem('The product that sends the notifications',
		'mutation ConnectedProductAdd { ConnectedProductAdd(input: { id: "{{nc_product_card}}", typeId: "ConnectedProduct", item: { id: "{{nc_product_card}}", productId: "{{nc_product}}", name: "Notification sender" } }) { id } }',
		[{ key: 'x-authentication-token', value: '{{nc_su_token}}' }],
		["const p = payload('ConnectedProductAdd');", "pm.test('connected', function () { pm.expect(p.id).to.eql(env('nc_product_card')); });"]),
	newItem('A second product that sends notifications',
		'mutation ConnectedProductAdd { ConnectedProductAdd(input: { id: "{{nc_other_card}}", typeId: "ConnectedProduct", item: { id: "{{nc_other_card}}", productId: "{{nc_other}}", name: "Second sender" } }) { id } }',
		[{ key: 'x-authentication-token', value: '{{nc_su_token}}' }],
		["const p = payload('ConnectedProductAdd');", "pm.test('connected', function () { pm.expect(p.id).to.eql(env('nc_other_card')); });"]),
	newItem('The token of the server of the sender',
		'mutation CreateServerToken { CreateServerToken(input: { clientProductId: "{{nc_product}}", name: "nc sender", scopes: ["/ProductServer/Notifications/View", "/ProductServer/Notifications/Send"] }) { id token scopes } }',
		[{ key: 'x-authentication-token', value: '{{nc_su_token}}' }],
		["const p = payload('CreateServerToken');", "pm.test('it may send and read notifications', function () { pm.expect(p.scopes).to.include.members(['/ProductServer/Notifications/View', '/ProductServer/Notifications/Send']); });",
			"pm.environment.set('nc_server_token', p.token);", "pm.environment.set('nc_server_token_id', p.id);"]),
	newItem('A token of the sender that may not send',
		'mutation CreateServerToken { CreateServerToken(input: { clientProductId: "{{nc_product}}", name: "nc reader", scopes: ["/ProductServer/Notifications/View"] }) { id token scopes } }',
		[{ key: 'x-authentication-token', value: '{{nc_su_token}}' }],
		["const p = payload('CreateServerToken');", "pm.test('it may not send', function () { pm.expect(p.scopes).to.not.include('/ProductServer/Notifications/Send'); });",
			"pm.environment.set('nc_reader_token', p.token);"]),
	newItem('The token of the server of the second product',
		'mutation CreateServerToken { CreateServerToken(input: { clientProductId: "{{nc_other}}", name: "nc second sender", scopes: ["/ProductServer/Notifications/View", "/ProductServer/Notifications/Send"] }) { id token scopes } }',
		[{ key: 'x-authentication-token', value: '{{nc_su_token}}' }],
		["const p = payload('CreateServerToken');", "pm.test('it may send', function () { pm.expect(p.scopes).to.include('/ProductServer/Notifications/Send'); });",
			"pm.environment.set('nc_other_token', p.token);"])
];
if (!bootstrap.item.some((i) => i.name === senderItems[0].name)){
	const suIndex = bootstrap.item.findIndex((i) => i.name === 'Authorization SU');
	bootstrap.item.splice(suIndex + 1, 0, ...senderItems);
}
// The products of the run
const runTag = bootstrap.item.find((i) => i.name === 'Run tag');
const runPre = runTag.event.find((e) => e.listen === 'prerequest').script.exec;
if (!runPre.some((l) => l.includes('nc_product'))){
	runPre.push(
		"pm.environment.set('nc_product', 'NcSender' + pm.environment.get('nc_run'));",
		"pm.environment.set('nc_other', 'NcOther' + pm.environment.get('nc_run'));",
		"pm.environment.set('nc_product_card', pm.variables.replaceIn('{{$guid}}'));",
		"pm.environment.set('nc_other_card', pm.variables.replaceIn('{{$guid}}'));");
}

// ---------------------------------------------------------------- 01 Access: who sends

const access = folder('01');
const byName = (f, name) => f.item.find((i) => i.name === name);
const sendQuery = (product, typeId) => `mutation SendNotification { SendNotification(input: { productId: "${product}", userIds: ["{{nc_bob_id}}"], typeId: "${typeId}" }) { success } }`;

const anonymous = byName(access, 'An anonymous request may not send');
if (anonymous){
	setQuery(anonymous, sendQuery('{{nc_product}}', 'nc.Access'));
	setTest(anonymous, ["refused('SendNotification', /Notifications are sent only by the server of a product/);"]);
}
const user = byName(access, 'A signed-in user may send for the product of the request (until ImtCore#1059)');
if (user){
	user.name = 'A signed-in user may not send';
	setQuery(user, sendQuery('{{nc_product}}', 'nc.Access'));
	setTest(user, ["refused('SendNotification', /Notifications are sent only by the server of a product/);"]);
	const index = access.item.indexOf(user);
	access.item.splice(index + 1, 0,
		newItem('A superuser session may not send', sendQuery('{{nc_product}}', 'nc.Access'), [{ key: 'x-authentication-token', value: '{{nc_su_token}}' }],
			["refused('SendNotification', /Notifications are sent only by the server of a product/);"]),
		newItem('A request forwarded by the server of the sender does not send for the user', sendQuery('{{nc_product}}', 'nc.Access'),
			[{ key: 'x-authentication-token', value: '{{nc_alice_token}}' }, { key: 'x-server-token', value: '{{nc_server_token}}' }],
			["refused('SendNotification', /Notifications are sent only by the server of a product/);"], '{{nc_product}}'),
		newItem('A token without the permission to send is refused', sendQuery('{{nc_product}}', 'nc.Access'), [{ key: 'x-authentication-token', value: '{{nc_reader_token}}' }],
			["refused('SendNotification', /The server of 'NcSender[0-9a-z]+' may not send 'SendNotification': its token lacks '\\/ProductServer\\/Notifications\\/Send'/);"], '{{nc_product}}'));
	addPre(access.item[index + 3], "pm.variables.set('expectedHttpStatus', 403);");
}
const otherProduct = byName(access, 'A token sends only to the product of its request');
if (otherProduct){
	otherProduct.name = 'The server of a product sends only for its product';
	setHeader(otherProduct, 'x-authentication-token', '{{nc_server_token}}');
	setProduct(otherProduct, '{{nc_product}}');
	setQuery(otherProduct, sendQuery('{{nc_other}}', 'nc.Access'));
	setTest(otherProduct, ["refused('SendNotification', /Notifications are sent only to the product of the sender/);"]);
}
const userWithdraw = byName(access, 'A signed-in user may withdraw for the product of the request (until ImtCore#1059)');
if (userWithdraw){
	userWithdraw.name = 'A signed-in user may not withdraw';
	setQuery(userWithdraw, userWithdraw.request.body.graphql.query.replace(/productId: "[^"]+"/, 'productId: "{{nc_product}}"'));
	setTest(userWithdraw, ["refused('WithdrawNotifications', /Notifications are sent only by the server of a product/);"]);
}
for (const name of ['More than 100 recipients are refused', 'A group is withdrawn from at most 100 users']){
	const item = byName(access, name);
	if (item){
		sentByServer(item);
	}
}
const nothing = byName(access, 'Nothing refused reached an inbox');
if (nothing){
	readThroughServer(nothing);
}

// ---------------------------------------------------------------- 02-07: the sender sends, the users read through it

for (const prefix of ['02', '03', '04', '05', '07']){
	for (const item of folder(prefix).item){
		const command = commandOf(item);
		const auth = ((item.request.header || []).find((h) => h.key === 'x-authentication-token') || {}).value || '';
		if ((command === 'SendNotification' || command === 'WithdrawNotifications') && (auth === '{{nc_su_token}}' || auth === '{{nc_server_token}}')){
			sentByServer(item);
		}
		else if (inboxCommands.includes(command) && auth && auth !== '{{nc_su_token}}'){
			readThroughServer(item);
		}
	}
}

// Two senders: a notification of one product is not seen through the other
const inboxes = folder('03');
const second = byName(inboxes, 'Product ProLife notification to alice') || byName(inboxes, 'A notification of the second product to alice');
if (second){
	second.name = 'A notification of the second product to alice';
	sentByServer(second, '{{nc_other_token}}', '{{nc_other}}');
}
const notSeen = byName(inboxes, 'Alice in Puma does not see a ProLife one') || byName(inboxes, 'Through the first product alice does not see the second one');
if (notSeen){
	notSeen.name = 'Through the first product alice does not see the second one';
	readThroughServer(notSeen);
}
const seen = byName(inboxes, 'Alice in ProLife sees it') || byName(inboxes, 'Through the second product alice sees it');
if (seen){
	seen.name = 'Through the second product alice sees it';
	setHeader(seen, 'x-authentication-token', '{{nc_alice_token}}');
	readThroughServer(seen, '{{nc_other_token}}', '{{nc_other}}');
}

// ---------------------------------------------------------------- 08: the one notification sent through the API

const organizations = folder('08');
const toCarol = byName(organizations, 'A tenant notification to the member carol');
if (toCarol){
	sentByServer(toCarol);
}
for (const name of ['Carol in A has it', 'A removed member lost the notifications of the organization']){
	const item = byName(organizations, name);
	if (item){
		readThroughServer(item);
	}
}

// ---------------------------------------------------------------- the tokens of the senders are revoked at the end

const cleanupName = 'The tokens of the senders are revoked';
if (!organizations.item.some((i) => i.name === cleanupName)){
	organizations.item.push(newItem(cleanupName,
		'mutation RemoveElements { RemoveElements(input: { collectionId: "ConnectedProducts", elementIds: ["{{nc_product_card}}", "{{nc_other_card}}"] }) { success } }',
		[{ key: 'x-authentication-token', value: '{{nc_su_token}}' }],
		["const p = payload('RemoveElements');", "pm.test('both senders removed with their tokens', function () { pm.expect(p.success).to.eql(true); });"]));
}

// The format of the file: tabs, the line ends of the checkout and its final line end
const original = fs.readFileSync(collectionPath, 'utf8');
const newline = original.includes('\r\n') ? '\r\n' : '\n';
fs.writeFileSync(collectionPath, JSON.stringify(collection, null, '\t').replace(/\n/g, newline) + (original.endsWith(newline) ? newline : ''));
console.log('done');
