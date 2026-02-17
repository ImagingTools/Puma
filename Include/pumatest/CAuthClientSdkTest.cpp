// SPDX-License-Identifier: MIT
#include "CAuthClientSdkTest.h"


// ACF includes
#include <itest/CStandardTestExecutor.h>


using namespace AuthClientSdk;


static const QByteArrayList s_roleNames = {"Test1", "Test2", "Test3", "Test4", "Test5"};
static const QByteArrayList s_userNames = {"Test1", "Test2", "Test3", "Test4", "Test5"};
static const QByteArrayList s_groupNames = {"Test1", "Test2", "Test3", "Test4", "Test5"};


void CAuthClientSdkTest::initTestCase()
{
	qDebug() << "=== [Init] AuthClientSdk tests start ===";

	AuthServerSdk::ServerConfig pumaConfig;
	pumaConfig.wsPort = 8788;
	pumaConfig.httpPort = 7788;
	
	m_authorizableServer.SetPumaConnectionParam(pumaConfig);
	m_authorizableServer.SetProductId("Test");

	AuthServerSdk::ServerConfig config;
	config.wsPort = 8888;
	config.httpPort = 7777;

	QVERIFY(m_authorizableServer.Start(config));

	m_authorizationController.SetProductId("Test");

	AuthClientSdk::ServerConfig serverConfig;
	serverConfig.wsPort = 8888;
	serverConfig.httpPort = 7777;
	m_authorizationController.SetConnectionParam(serverConfig);
	QVERIFY(m_authorizationController.GetToken().isEmpty());
}


void CAuthClientSdkTest::SuperuserExistsTest()
{
	qDebug() << "=== [SuperuserExistsTest] ===";
	QString err;
	SuperuserStatus status = m_authorizationController.SuperuserExists(err);

	QVERIFY2(err.isEmpty(), qPrintable(err));
	QVERIFY(status == SuperuserStatus::NotExists ||
			status == SuperuserStatus::Exists ||
			status == SuperuserStatus::Unknown);

	if (status == SuperuserStatus::NotExists){
		qDebug() << "Superuser not found — creating new superuser...";
		bool created = m_authorizationController.CreateSuperuser("1");
		QVERIFY2(created, "Failed to create superuser");

		QString err2;
		auto status2 = m_authorizationController.SuperuserExists(err2);
		QCOMPARE(status2, SuperuserStatus::Exists);
	}
	else if (status == SuperuserStatus::Exists){
		qDebug() << "Superuser already exists — trying to recreate (should fail)";
		bool created = m_authorizationController.CreateSuperuser("1");
		QVERIFY(!created);
	}
	else{
		qWarning("Superuser status is Unknown — skipping create test");
	}
}


void CAuthClientSdkTest::LoginLogoutTest()
{
	qDebug() << "=== [LoginLogoutTest] ===";

	Login loginData;

	bool ok = m_authorizationController.Login("su", "2", loginData);
	QVERIFY2(!ok, "Login succesful");
	QVERIFY(loginData.accessToken.isEmpty());
	QVERIFY(loginData.productId.isEmpty());
	QVERIFY(loginData.userName.isEmpty());

	bool ok2 = m_authorizationController.Login("su", "1", loginData);
	QVERIFY2(ok2, "Login failed");

	qDebug() << "Logged in as:" << loginData.userName;
	QVERIFY(!loginData.accessToken.isEmpty());
	QVERIFY(!loginData.productId.isEmpty());
	QVERIFY(!loginData.userName.isEmpty());
	QVERIFY(loginData.permissions.isEmpty());

	bool logoutOk = m_authorizationController.Logout();
	QVERIFY2(logoutOk, "Logout failed");
	QVERIFY(m_authorizationController.GetToken().isEmpty());
	QVERIFY(!m_authorizationController.HasPermission("*"));

	// Try to create without authorization
	QVERIFY(m_authorizationController.CreateUser(s_userNames[0], s_userNames[0], "1", "test@example.com").isEmpty());

	QVERIFY(m_authorizationController.Login("su", "1", loginData));
	QByteArray userId = m_authorizationController.CreateUser(s_userNames[0], s_userNames[0], "1", "test@example.com");
	QVERIFY(!userId.isEmpty());
	QByteArray roleId = m_authorizationController.CreateRole(s_roleNames[0], "", {"PermissionA", "PermissionB"});
	QVERIFY(!roleId.isEmpty());
	QVERIFY(m_authorizationController.AddRolesToUser(userId, {roleId}));
	QVERIFY(m_authorizationController.Logout());

	QVERIFY(!m_authorizationController.Login(s_userNames[0], "2", loginData));
	QVERIFY(loginData.accessToken.isEmpty());

	QVERIFY(m_authorizationController.Login(s_userNames[0], "1", loginData));
	QVERIFY(!loginData.accessToken.isEmpty());
	QVERIFY(loginData.permissions.size() == 2);
	QVERIFY(loginData.permissions.contains("PermissionA"));
	QVERIFY(loginData.permissions.contains("PermissionB"));
	QVERIFY(m_authorizationController.Logout());

	QVERIFY(m_authorizationController.GetToken().isEmpty());
	QVERIFY(!m_authorizationController.HasPermission("PermissionA"));
	QVERIFY(!m_authorizationController.HasPermission("PermissionB"));

	QVERIFY(m_authorizationController.Login("su", "1", loginData));
	QVERIFY(m_authorizationController.RemoveUser(userId));
	QVERIFY(m_authorizationController.RemoveRole(roleId));
	QVERIFY(m_authorizationController.Logout());
}


void CAuthClientSdkTest::UserCrudTest()
{
	qDebug() << "=== [UserCrudTest] ===";

	QByteArray userId = m_authorizationController.CreateUser(s_userNames[0], s_userNames[0], "1", "test@example.com");
	QVERIFY2(userId.isEmpty(), "The user was created without authorization");

	QByteArrayList userIds = m_authorizationController.GetUserIds();
	QVERIFY2(userIds.isEmpty(), "The User-IDs received without authorization");

	Login loginData;

	bool ok = m_authorizationController.Login("su", "1", loginData);
	QVERIFY(ok);

	userId = m_authorizationController.CreateUser(s_userNames[0], s_userNames[0], "1", "test@example.com");
	QVERIFY2(!userId.isEmpty(), "User creation failed");

	auto allUsers = m_authorizationController.GetUserIds();
	QVERIFY(allUsers.contains(userId));

	QVERIFY2(m_authorizationController.CreateUser(s_userNames[0], s_userNames[0], "1", "test2@example.com").isEmpty(), "User with this login already exists");
	QVERIFY2(m_authorizationController.CreateUser(s_userNames[1], s_userNames[1], "1", "test@example.com").isEmpty(), "User with this email already exists");

	User u;
	bool got = m_authorizationController.GetUser(userId, u);
	QVERIFY(got);
	QCOMPARE(u.login, s_userNames[0]);
	QCOMPARE(u.name, s_userNames[0]);
	QCOMPARE(u.email, QString("test@example.com"));
	QVERIFY(u.roleIds.isEmpty());
	QVERIFY(u.groupIds.isEmpty());

	QByteArray roleId = m_authorizationController.CreateRole(s_roleNames[0], "", {"A", "B", "C"});
	QVERIFY2(!roleId.isEmpty(), "Role was not created");

	QVERIFY(m_authorizationController.AddRolesToUser(userId, {roleId}));

	User u2;
	bool got2 = m_authorizationController.GetUser(userId, u2);
	QVERIFY(got2);
	QCOMPARE(u2.login, s_userNames[0]);
	QCOMPARE(u2.name, s_userNames[0]);
	QCOMPARE(u2.email, QString("test@example.com"));
	QVERIFY(u2.roleIds.size() == 1);
	QVERIFY(u2.roleIds.contains(roleId));
	QVERIFY(u2.groupIds.isEmpty());

	QByteArrayList userPermissions = m_authorizationController.GetUserPermissions(userId);
	QVERIFY(!userPermissions.isEmpty());
	QVERIFY(userPermissions.size() == 3);
	QVERIFY(userPermissions.contains("A"));
	QVERIFY(userPermissions.contains("B"));
	QVERIFY(userPermissions.contains("C"));

	QVERIFY(!m_authorizationController.RemoveRolesFromUser(userId, {}));
	QVERIFY(m_authorizationController.GetUserPermissions(userId).size() == 3);

	QVERIFY(m_authorizationController.RemoveRolesFromUser(userId, {"12345"}));
	QVERIFY(m_authorizationController.GetUserPermissions(userId).size() == 3);

	QVERIFY(m_authorizationController.RemoveRolesFromUser(userId, {roleId}));
	QVERIFY(m_authorizationController.GetUserPermissions(userId).size() == 0);

	QVERIFY(m_authorizationController.Logout());

	Login loginData2;
	QVERIFY(!m_authorizationController.Login(s_userNames[0], "2", loginData2));
	QVERIFY(m_authorizationController.Login(s_userNames[0], "1", loginData2));
	QVERIFY(!loginData2.accessToken.isEmpty());
	QVERIFY(m_authorizationController.Logout());

	QVERIFY(m_authorizationController.Login("su", "1", loginData2));
	QVERIFY(m_authorizationController.ChangeUserPassword(s_userNames[0], "1", "2"));
	QVERIFY(m_authorizationController.Logout());

	QVERIFY(!m_authorizationController.Login(s_userNames[0], "1", loginData2));
	QVERIFY(m_authorizationController.Login(s_userNames[0], "2", loginData2));
	QVERIFY(m_authorizationController.Logout());
}


void CAuthClientSdkTest::RoleCrudTest()
{
	qDebug() << "=== [RoleCrudTest] ===";

	Login loginData;

	bool ok = m_authorizationController.Login("su", "1", loginData);
	QVERIFY(ok);

	QByteArray roleId = m_authorizationController.CreateRole(s_roleNames[1], "Test description");
	QVERIFY2(!roleId.isEmpty(), "Failed to create role");

	auto roles = m_authorizationController.GetRoleIds();
	QVERIFY(roles.contains(roleId));

	Role role;
	bool got = m_authorizationController.GetRole(roleId, role);
	QVERIFY(got);
	QCOMPARE(role.name, QString(s_roleNames[1]));
	QCOMPARE(role.description, QString("Test description"));
	QVERIFY(role.permissionIds.isEmpty());

	QVERIFY(m_authorizationController.AddPermissionsToRole(roleId, {"AddOrder", "RemoveOrder"}));

	Role role2;
	QVERIFY(m_authorizationController.GetRole(roleId, role2));
	QCOMPARE(role2.name, s_roleNames[1]);
	QCOMPARE(role2.description, QString("Test description"));
	QVERIFY(role2.permissionIds.size() == 2);
	QVERIFY(role2.permissionIds.contains("AddOrder"));
	QVERIFY(role2.permissionIds.contains("RemoveOrder"));

	QVERIFY(!m_authorizationController.RemovePermissionsFromRole(roleId, {}));
	QVERIFY(m_authorizationController.RemovePermissionsFromRole(roleId, {"A"}));
	QVERIFY(m_authorizationController.GetRole(roleId, role2));
	QVERIFY(role2.permissionIds.size() == 2);

	QVERIFY(m_authorizationController.RemovePermissionsFromRole(roleId, {"AddOrder"}));

	Role role3;
	QVERIFY(m_authorizationController.GetRole(roleId, role3));
	QVERIFY(role3.permissionIds.size() == 1);
	QVERIFY(!role3.permissionIds.contains("AddOrder"));
	QVERIFY(role3.permissionIds.contains("RemoveOrder"));

	QVERIFY(m_authorizationController.RemovePermissionsFromRole(roleId, {"RemoveOrder"}));
	QVERIFY(m_authorizationController.GetRole(roleId, role3));
	QVERIFY(role3.permissionIds.isEmpty());
	QVERIFY(m_authorizationController.RemoveRole(roleId));
	QVERIFY(!m_authorizationController.GetRoleIds().contains(roleId));

	m_authorizationController.Logout();
}


void CAuthClientSdkTest::GroupCrudTest()
{
	qDebug() << "=== [GroupCrudTest] ===";

	Login loginData;

	bool ok = m_authorizationController.Login("su", "1", loginData);
	QVERIFY(ok);

	QByteArray groupId = m_authorizationController.CreateGroup(s_groupNames[0], "Test group description");
	QVERIFY2(!groupId.isEmpty(), "Failed to create group");

	auto groups = m_authorizationController.GetGroupIds();
	QVERIFY(groups.contains(groupId));

	Group g;
	QVERIFY(m_authorizationController.GetGroup(groupId, g));
	QCOMPARE(g.name, s_groupNames[0]);
	QCOMPARE(g.description, QString("Test group description"));
	QVERIFY(g.userIds.isEmpty());
	QVERIFY(g.roleIds.isEmpty());

	QVERIFY(!m_authorizationController.AddUsersToGroup(groupId, {}));
	QVERIFY(!m_authorizationController.AddRolesToGroup(groupId, {}));
	QVERIFY(!m_authorizationController.RemoveUsersFromGroup(groupId, {}));
	QVERIFY(!m_authorizationController.RemoveRolesFromGroup(groupId, {}));

	QByteArray userId1 = m_authorizationController.CreateUser(s_userNames[2], s_userNames[2], "1", "test1@mail.ru");
	QVERIFY(!userId1.isEmpty());

	QByteArray userId2 = m_authorizationController.CreateUser(s_userNames[3], s_userNames[3], "1", "test2@mail.ru");
	QVERIFY(!userId2.isEmpty());

	QVERIFY(m_authorizationController.AddUsersToGroup(groupId, {userId1, userId2}));
	QVERIFY(m_authorizationController.GetGroup(groupId, g));
	QVERIFY(g.userIds.size() == 2);
	QVERIFY(g.userIds.contains(userId1));
	QVERIFY(g.userIds.contains(userId2));
	QVERIFY(g.roleIds.isEmpty());
	QCOMPARE(g.name, s_groupNames[0]);
	QCOMPARE(g.description, QString("Test group description"));

	QVERIFY(m_authorizationController.RemoveUsersFromGroup(groupId, {userId1, userId2}));
	QVERIFY(m_authorizationController.GetGroup(groupId, g));
	QVERIFY(g.userIds.isEmpty());
	QVERIFY(!g.userIds.contains(userId1));
	QVERIFY(!g.userIds.contains(userId2));
	QVERIFY(g.roleIds.isEmpty());
	QCOMPARE(g.name, s_groupNames[0]);
	QCOMPARE(g.description, QString("Test group description"));

	QByteArray roleId1 = m_authorizationController.CreateRole(s_roleNames[2], "", {"A", "B", "C"});
	QVERIFY(!roleId1.isEmpty());

	QByteArray roleId2 = m_authorizationController.CreateRole(s_roleNames[3], "", {"D", "E"});
	QVERIFY(!roleId2.isEmpty());

	QVERIFY(m_authorizationController.AddRolesToGroup(groupId, {roleId1, roleId2}));
	QVERIFY(m_authorizationController.GetGroup(groupId, g));
	QVERIFY(g.userIds.isEmpty());
	QVERIFY(g.roleIds.size() == 2);
	QVERIFY(g.roleIds.contains(roleId1));
	QVERIFY(g.roleIds.contains(roleId2));

	QVERIFY(m_authorizationController.RemoveRolesFromGroup(groupId, {roleId1}));
	QVERIFY(m_authorizationController.GetGroup(groupId, g));
	QVERIFY(g.userIds.isEmpty());
	QVERIFY(g.roleIds.size() == 1);
	QVERIFY(!g.roleIds.contains(roleId1));
	QVERIFY(g.roleIds.contains(roleId2));

	QVERIFY(m_authorizationController.RemoveRole(roleId2));

	m_authorizationController.Logout();
}


void CAuthClientSdkTest::cleanupTestCase()
{
	qDebug() << "=== [Cleanup] All tests completed ===";

	Login loginData;
	m_authorizationController.Login("su", "1", loginData);

	QByteArrayList userIds = m_authorizationController.GetUserIds();
	for (const QByteArray& userId : userIds){
		User userData;
		if (m_authorizationController.GetUser(userId, userData)){
			if (s_userNames.contains(userData.login)){
				QVERIFY(m_authorizationController.RemoveUser(userId));
			}
		}
	}

	QByteArrayList roleIds = m_authorizationController.GetRoleIds();
	for (const QByteArray& roleId : roleIds){
		Role roleData;
		if (m_authorizationController.GetRole(roleId, roleData)){
			if (s_roleNames.contains(roleData.name)){
				QVERIFY(m_authorizationController.RemoveRole(roleId));
			}
		}
	}

	QByteArrayList groupIds = m_authorizationController.GetGroupIds();
	for (const QByteArray& groupId : groupIds){
		Group groupData;
		if (m_authorizationController.GetGroup(groupId, groupData)){
			if (s_groupNames.contains(groupData.name)){
				QVERIFY(m_authorizationController.RemoveGroup(groupId));
			}
		}
	}

	m_authorizationController.Logout();
}


I_ADD_TEST(CAuthClientSdkTest);
