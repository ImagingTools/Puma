// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
//
// The push of the notification center (OnNotificationEvent) over WebSocket.
//
// Every participant holds live sessions subscribed the way NotificationCenter.qml does. Each case changes an inbox
// and requires that exactly the sessions that see that inbox received exactly one event of the expected kind, and
// that every other session received nothing:
//   Created / Updated -> the sessions of the recipient that see the scope of the notification
//                        (a personal one every session of the user, a tenant one the sessions in that tenant)
//   Read / AllRead / Removed -> the same, for the change of the own inbox; AllRead carries createdUntil, no ids,
//                        and only for a tenant inbox: a personal one is read by ids
//   Withdraw          -> Removed to every inbox of the group
//   a product of another product, a delivered acknowledgement -> nothing at all
// The unread count of an event is the count of its inbox, or -1 for a personal notification: it reaches sessions of
// several inboxes, each of which counts its own.

'use strict';

const crypto = require('crypto');
const {
	wsUrl, suLogin, suPassword, productId,
	gqlString, gql, login,
	SubscriptionSession, notificationOf,
	assert, test, requires, runSuite
} = require('./ws-lib');

const COMMAND = 'OnNotificationEvent';
const QUERY = `subscription ${COMMAND} { ${COMMAND} { kind items { id typeId groupCount subject { id } } ids createdUntil unreadCount } }`;

// How long a misrouted event gets to show up after the expected ones arrived.
const quietMs = 1500;

const runTag = Date.now().toString(36);
const password = 'WsCenter_2026!';

async function act(label, query, token, product, headers) {
	const res = await gql(query, token, product, headers);
	if (res && res.errors) {
		throw new Error(`${label} returned GraphQL errors: ${JSON.stringify(res.errors).slice(0, 400)}`);
	}
	const payload = res && res.data && res.data[label];
	if (!payload) {
		throw new Error(`${label} returned no data: ${JSON.stringify(res).slice(0, 400)}`);
	}
	if (payload.errorMessage || payload.success === false) {
		throw new Error(`${label} was refused: ${JSON.stringify(payload).slice(0, 400)}`);
	}
	return payload;
}

// Puma takes notifications from the server of a product only; its users watch and read them through that server
const sender = { product: `WsncSender${runTag}`, token: null };
const otherSender = { product: `WsncOther${runTag}`, token: null };
const throughSender = () => ({ 'x-server-token': sender.token });

function send(fields) {
	return act('SendNotification',
		`mutation SendNotification { SendNotification(input: { productId: ${gqlString(sender.product)}, ${fields} }) { success } }`, sender.token, sender.product);
}

// An action of a user on the notifications of the sender
function actThroughSender(label, query, token) {
	return act(label, query, token, sender.product, throughSender());
}

async function loginThroughSender(username) {
	const auth = await actThroughSender('Authorization',
		`query Authorization { Authorization(input: { login: ${gqlString(username)}, password: ${gqlString(password)},` +
		` productId: ${gqlString(sender.product)} }) { userId token } }`);
	assert(auth.userId && auth.token, 'The sender did not issue a user session');
	return auth;
}

// --- the sessions

const sessions = [];

async function openSession(role, token) {
	const s = { role, token, session: new SubscriptionSession(role, wsUrl, token, Object.assign({ productId: sender.product }, throughSender())) };
	await s.session.connect();
	await s.session.init();
	s.subId = s.session.beginSubscribe(COMMAND, QUERY);
	sessions.push(s);
	return s;
}

function eventsSince(s, mark) {
	return s.session.frames.slice(mark).filter(f => f.type === 'data' && f.id === s.subId).map(f => notificationOf(f, COMMAND));
}

// Performs the action and requires every listed session to receive exactly the listed events - one predicate, or an
// array of them when the change reaches the session from several inboxes, each matching an event of its own - and
// every other session nothing.
async function expectEvents(expected, action) {
	const marks = new Map(sessions.map(s => [s, s.session.mark()]));
	const result = await action();

	await new Promise(r => setTimeout(r, quietMs));

	const problems = [];
	for (const s of sessions) {
		const events = eventsSince(s, marks.get(s));
		if (expected.has(s)) {
			const predicates = [].concat(expected.get(s));
			const unmatched = events.slice();
			for (const matches of predicates) {
				const index = unmatched.findIndex(e => matches(e, result));
				if (index < 0) {
					problems.push(`${s.role} did not receive an expected event; got ${JSON.stringify(events).slice(0, 400)}`);
				}
				else {
					unmatched.splice(index, 1);
				}
			}
			if (events.length !== predicates.length) {
				problems.push(`${s.role} received ${events.length} events instead of ${predicates.length}: ${JSON.stringify(events).slice(0, 400)}`);
			}
		}
		else if (events.length > 0) {
			problems.push(`${s.role} must not receive anything but received ${JSON.stringify(events).slice(0, 300)}`);
		}
	}
	assert(problems.length === 0, problems.join('\n        '));
	return result;
}

async function unreadCount(token) {
	const res = await gql('query NotificationsUnreadCount { NotificationsUnreadCount { unreadCount } }', token, sender.product, throughSender());
	return res.data.NotificationsUnreadCount.unreadCount;
}

async function inboxItems(token, typeId) {
	const res = await gql('query NotificationsList { NotificationsList(input: { pageSize: 100 }) { items { id typeId groupCount } } }', token, sender.product, throughSender());
	const items = (res && res.data && res.data.NotificationsList && res.data.NotificationsList.items) || [];
	return items.filter(i => i.typeId === typeId);
}

const created = (typeId) => (e) => e && e.kind === 'Created' && (e.items || []).length === 1 && e.items[0].typeId === typeId;

// --- main

runSuite({ name: 'Notification center WebSocket suite', className: 'NotificationCenterWs', junitFile: 'junit-report-ws-notification-center.xml' }, async () => {
	const su = await login(suLogin, suPassword);

	for (const product of [sender, otherSender]) {
		const card = crypto.randomUUID();
		await act('ConnectedProductAdd',
			`mutation ConnectedProductAdd { ConnectedProductAdd(input: { id: ${gqlString(card)}, typeId: "ConnectedProduct",` +
			` item: { id: ${gqlString(card)}, productId: ${gqlString(product.product)}, name: ${gqlString(product.product)} } }) { id } }`, su.token);
		product.token = (await act('CreateServerToken',
			`mutation CreateServerToken { CreateServerToken(input: { clientProductId: ${gqlString(product.product)}, name: "ws notification center",` +
			' scopes: [] }) { token } }', su.token)).token;
	}

	const createUser = async (role) => {
		const loginName = `wsnc_${role}_${runTag}`;
		const registered = await act('RegisterUser',
			`mutation RegisterUser { RegisterUser(input: { userData: { name: ${gqlString('WS NC ' + role)}, username: ${gqlString(loginName)},` +
			` password: ${gqlString(password)}, email: ${gqlString(loginName + '@ws.test')} }, productId: ${gqlString(productId)} }) { id } }`, su.token);
		assert(registered.id, 'The notification fixture user was not created');
		return { role, login: loginName, userId: registered.id };
	};

	const alice = await createUser('alice');
	const bob = await createUser('bob');

	const tenantA = (await act('CreateTenant',
		`mutation CreateTenant { CreateTenant(input: { name: ${gqlString('WSNC_A_' + runTag)}, description: "ws notification center",` +
		` ownerId: ${gqlString(alice.userId)} }) { tenantId errorMessage } }`, su.token)).tenantId;

	// SelectTenant ends the session of the token it is called with: the tenant session signs in on its own
	const aliceTenantLogin = await loginThroughSender(alice.login);
	const selected = await actThroughSender('SelectTenant',
		`mutation SelectTenant { SelectTenant(input: { tenantId: ${gqlString(tenantA)} }) { ok userSession { tenantId accessToken } errorMessage } }`,
		aliceTenantLogin.token);

	const aliceHome = await openSession('alice without an organization', (await loginThroughSender(alice.login)).token);
	const aliceInA = await openSession('alice in organization A', selected.userSession.accessToken);
	const aliceSecond = await openSession('a second session of alice', (await loginThroughSender(alice.login)).token);
	const bobHome = await openSession('bob', (await loginThroughSender(bob.login)).token);

	await Promise.all(sessions.map(s => s.session.settle(s.subId)));
	for (const s of sessions) {
		s.subId = s.session.confirmAccepted(s.subId);
	}
	console.log(`Sessions: ${sessions.map(s => s.role).join(', ')}\n`);

	const personalOfAlice = (e) => e.unreadCount === -1;

	await test('a personal notification reaches every session of the recipient, with the count to read again', async () => {
		await expectEvents(new Map([
			[aliceHome, (e) => created('wsnc.Personal')(e) && personalOfAlice(e)],
			[aliceInA, (e) => created('wsnc.Personal')(e) && personalOfAlice(e)],
			[aliceSecond, (e) => created('wsnc.Personal')(e) && personalOfAlice(e)]
		]), () => send(`userIds: [${gqlString(alice.userId)}], typeId: "wsnc.Personal"`));
	});

	let tenantItemId = null;

	await test('a tenant notification reaches the sessions in that tenant only, with its unread count', async () => {
		await expectEvents(new Map([
			[aliceInA, (e) => created('wsnc.Tenant')(e) && e.unreadCount >= 1]
		]), () => send(`tenantId: ${gqlString(tenantA)}, userIds: [${gqlString(alice.userId)}], typeId: "wsnc.Tenant", groupKey: "wsnc-group"`));
		tenantItemId = requires((await inboxItems(aliceInA.token, 'wsnc.Tenant'))[0], 'the tenant notification').id;
	});

	await test('a grouped event updates the notification for the same sessions', async () => {
		requires(tenantItemId, 'the tenant notification');
		await expectEvents(new Map([
			[aliceInA, (e) => e.kind === 'Updated' && (e.items || []).length === 1 && e.items[0].id === tenantItemId && e.items[0].groupCount === 2]
		]), () => send(`tenantId: ${gqlString(tenantA)}, userIds: [${gqlString(alice.userId)}], typeId: "wsnc.Tenant", groupKey: "wsnc-group"`));
	});

	await test('reading it is told to the sessions of that inbox only, with its new count', async () => {
		requires(tenantItemId, 'the tenant notification');
		await expectEvents(new Map([
			[aliceInA, (e, count) => e.kind === 'Read' && JSON.stringify(e.ids) === JSON.stringify([tenantItemId]) && e.unreadCount === count]
		]), async () => {
			await actThroughSender('MarkNotificationsRead',
				`mutation MarkNotificationsRead { MarkNotificationsRead(input: { ids: [${gqlString(tenantItemId)}] }) { changedIds } }`, aliceInA.token);
			return unreadCount(aliceInA.token);
		});
	});

	await test('marking all read in a tenant is told per inbox: a bound to the tenant sessions, the ids of the personal ones everywhere', async () => {
		await send(`tenantId: ${gqlString(tenantA)}, userIds: [${gqlString(alice.userId)}], typeId: "wsnc.Unread"`);
		await new Promise(r => setTimeout(r, quietMs));
		const personalId = requires((await inboxItems(aliceHome.token, 'wsnc.Personal'))[0], 'the personal notification').id;
		// The other sessions hold notifications of other inboxes: a bound would mark them read too
		const personalRead = (e) => e.kind === 'Read' && JSON.stringify(e.ids) === JSON.stringify([personalId]) && e.unreadCount === -1;
		const tenantAllRead = (e) => e.kind === 'AllRead' && !!e.createdUntil && !isNaN(Date.parse(e.createdUntil)) &&
			(e.ids || []).length === 0 && e.unreadCount === 0;
		await expectEvents(new Map([
			[aliceHome, personalRead],
			[aliceSecond, personalRead],
			[aliceInA, [tenantAllRead, personalRead]]
		]), () => actThroughSender('MarkAllNotificationsRead',
			'mutation MarkAllNotificationsRead { MarkAllNotificationsRead(input: {}) { unreadCount } }', aliceInA.token));
	});

	await test('removing it is told to the sessions of that inbox only', async () => {
		requires(tenantItemId, 'the tenant notification');
		await expectEvents(new Map([
			[aliceInA, (e) => e.kind === 'Removed' && JSON.stringify(e.ids) === JSON.stringify([tenantItemId])]
		]), () => actThroughSender('RemoveNotifications',
			`mutation RemoveNotifications { RemoveNotifications(input: { ids: [${gqlString(tenantItemId)}] }) { changedIds } }`, aliceInA.token));
	});

	await test('a withdrawn group is removed from every inbox it reached', async () => {
		await send(`userIds: [${gqlString(alice.userId)}, ${gqlString(bob.userId)}], typeId: "wsnc.Withdraw", groupKey: "wsnc-withdraw"`);
		await new Promise(r => setTimeout(r, quietMs));
		const aliceId = requires((await inboxItems(aliceHome.token, 'wsnc.Withdraw'))[0], 'the notification of alice').id;
		const bobId = requires((await inboxItems(bobHome.token, 'wsnc.Withdraw'))[0], 'the notification of bob').id;
		const removed = (id) => (e) => e.kind === 'Removed' && JSON.stringify(e.ids) === JSON.stringify([id]);
		await expectEvents(new Map([
			[aliceHome, removed(aliceId)],
			[aliceInA, removed(aliceId)],
			[aliceSecond, removed(aliceId)],
			[bobHome, removed(bobId)]
		]), () => act('WithdrawNotifications',
			`mutation WithdrawNotifications { WithdrawNotifications(input: { productId: ${gqlString(sender.product)}, groupKey: "wsnc-withdraw" }) { success } }`,
			sender.token, sender.product));
	});

	await test('a notification of another product reaches no session of this product', async () => {
		await expectEvents(new Map(), async () => {
			const res = await gql(`mutation SendNotification { SendNotification(input: { productId: ${gqlString(otherSender.product)},` +
				` userIds: [${gqlString(alice.userId)}], typeId: "wsnc.Other" }) { success } }`, otherSender.token, otherSender.product);
			assert(res && res.data && res.data.SendNotification && res.data.SendNotification.success === true,
				`the notification of ${otherSender.product} was not sent: ${JSON.stringify(res).slice(0, 300)}`);
		});
	});

	await test('acknowledging a delivery is no change the sessions are told', async () => {
		await send(`userIds: [${gqlString(bob.userId)}], typeId: "wsnc.Delivered"`);
		await new Promise(r => setTimeout(r, quietMs));
		const itemId = requires((await inboxItems(bobHome.token, 'wsnc.Delivered'))[0], 'the notification of bob').id;
		await expectEvents(new Map(), () => actThroughSender('AcknowledgeNotificationsDelivered',
			`mutation AcknowledgeNotificationsDelivered { AcknowledgeNotificationsDelivered(input: { ids: [${gqlString(itemId)}] }) { changedIds } }`,
			bobHome.token));
	});

	await test('no unaccounted frames or protocol violations on any session', async () => {
		const violations = sessions.flatMap(s => s.session.violations);
		assert(violations.length === 0, violations.join('\n        '));
	});

	for (const s of sessions) {
		s.session.close();
	}
});
