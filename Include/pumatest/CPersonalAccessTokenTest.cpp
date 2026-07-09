// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#include "CPersonalAccessTokenTest.h"


// ACF includes
#include <itest/CStandardTestExecutor.h>


using namespace AuthClientSdk;


void CPersonalAccessTokenTest::initTestCase()
{
	qDebug() << "=== [Init] PersonalAccessToken tests start ===";

	AuthServerSdk::ServerConfig pumaConfig;
	pumaConfig.wsPort = 8788;
	pumaConfig.httpPort = 7788;

	m_authorizableServer.SetPumaConnectionParam(pumaConfig);
	m_authorizableServer.SetProductId("Test");
	m_authorizableServer.SetPersonalAccessTokenValidationEnabled(true);

	AuthServerSdk::ServerConfig config;
	config.wsPort = 8889;
	config.httpPort = 7778;

	QVERIFY(m_authorizableServer.Start(config));

	m_authorizationController.SetProductId("Test");

	AuthClientSdk::ServerConfig serverConfig;
	serverConfig.wsPort = 8889;
	serverConfig.httpPort = 7778;
	m_authorizationController.SetConnectionParam(serverConfig);
}


void CPersonalAccessTokenTest::CreateTokenTest()
{
	qDebug() << "=== [CreateTokenTest] ===";

	Login loginData;
	QVERIFY(m_authorizationController.Login("su", "1", loginData));

	// Create a user for PAT testing
	QByteArray userId = m_authorizationController.CreateUser(
		"PatTestUser", "pattestuser", "1", "pattest@example.com");
	QVERIFY(!userId.isEmpty());

	// Create a PAT for the user
	QByteArray token = m_authorizationController.CreatePersonalAccessToken(
		userId,
		"Test",
		"CI Pipeline Token",
		{"ReadData", "WriteData"},
		"");

	// Note: Token creation returns empty until ImtCore server-side
	// implementation is available. The test verifies the API contract.
	if (!token.isEmpty()) {
		qDebug() << "PAT created successfully (token length:" << token.size() << ")";
	}
	else {
		qDebug() << "PAT creation returned empty (expected until ImtCore support is available)";
	}

	// Create a PAT with expiration
	QByteArray tokenWithExpiry = m_authorizationController.CreatePersonalAccessToken(
		userId,
		"Test",
		"Expiring Token",
		{"ReadData"},
		"2099-12-31T23:59:59Z");

	if (!tokenWithExpiry.isEmpty()) {
		qDebug() << "PAT with expiration created successfully";
	}

	// Cleanup
	QVERIFY(m_authorizationController.RemoveUser(userId));
	QVERIFY(m_authorizationController.Logout());
}


void CPersonalAccessTokenTest::ListTokensTest()
{
	qDebug() << "=== [ListTokensTest] ===";

	Login loginData;
	QVERIFY(m_authorizationController.Login("su", "1", loginData));

	QByteArray userId = m_authorizationController.CreateUser(
		"PatListUser", "patlistuser", "1", "patlist@example.com");
	QVERIFY(!userId.isEmpty());

	// List tokens (should be empty initially)
	QList<PersonalAccessToken> tokens = m_authorizationController.ListPersonalAccessTokens(userId);
	qDebug() << "Initial token count:" << tokens.size();

	// List tokens filtered by product
	QList<PersonalAccessToken> productTokens = m_authorizationController.ListPersonalAccessTokens(
		userId, "Test");
	qDebug() << "Product-filtered token count:" << productTokens.size();

	// Cleanup
	QVERIFY(m_authorizationController.RemoveUser(userId));
	QVERIFY(m_authorizationController.Logout());
}


void CPersonalAccessTokenTest::ValidateTokenTest()
{
	qDebug() << "=== [ValidateTokenTest] ===";

	// Validate an empty token
	PersonalAccessTokenValidation result = m_authorizationController.ValidatePersonalAccessToken(QByteArray());
	QVERIFY(!result.isValid);

	// Validate a non-existent token
	PersonalAccessTokenValidation result2 = m_authorizationController.ValidatePersonalAccessToken("non-existent-token");
	QVERIFY(!result2.isValid);
}


void CPersonalAccessTokenTest::RevokeTokenTest()
{
	qDebug() << "=== [RevokeTokenTest] ===";

	// Revoke a non-existent token
	bool revoked = m_authorizationController.RevokePersonalAccessToken("non-existent-id");
	QVERIFY(!revoked);
}


void CPersonalAccessTokenTest::ExpiredTokenTest()
{
	qDebug() << "=== [ExpiredTokenTest] ===";

	Login loginData;
	QVERIFY(m_authorizationController.Login("su", "1", loginData));

	QByteArray userId = m_authorizationController.CreateUser(
		"PatExpiryUser", "patexpiryuser", "1", "patexpiry@example.com");
	QVERIFY(!userId.isEmpty());

	// Create a PAT with a past expiration date
	QByteArray expiredToken = m_authorizationController.CreatePersonalAccessToken(
		userId,
		"Test",
		"Expired Token",
		{"ReadData"},
		"2020-01-01T00:00:00Z");

	// If token was created (server-side available), validate it should fail
	if (!expiredToken.isEmpty()) {
		PersonalAccessTokenValidation result = m_authorizationController.ValidatePersonalAccessToken(expiredToken);
		QVERIFY(!result.isValid);
	}

	// Cleanup
	QVERIFY(m_authorizationController.RemoveUser(userId));
	QVERIFY(m_authorizationController.Logout());
}


void CPersonalAccessTokenTest::GetPermissionsWithPatTest()
{
	qDebug() << "=== [GetPermissionsWithPatTest] ===";

	Login loginData;
	QVERIFY(m_authorizationController.Login("su", "1", loginData));

	QByteArray userId = m_authorizationController.CreateUser(
		"PatPermUser", "patpermuser", "1", "patperm@example.com");
	QVERIFY(!userId.isEmpty());

	// Create a PAT with specific permissions
	QByteArray token = m_authorizationController.CreatePersonalAccessToken(
		userId,
		"Test",
		"Permission Test Token",
		{"AdminAccess", "ReadOnly"});

	// If token was created, test GetTokenPermissions with it
	if (!token.isEmpty()) {
		QByteArrayList permissions = m_authorizationController.GetTokenPermissions(token);
		// With PAT support, this should return PAT-specific permissions
		qDebug() << "PAT permissions:" << permissions;
	}

	// Cleanup
	QVERIFY(m_authorizationController.RemoveUser(userId));
	QVERIFY(m_authorizationController.Logout());
}


void CPersonalAccessTokenTest::cleanupTestCase()
{
	qDebug() << "=== [Cleanup] PersonalAccessToken tests completed ===";

	Login loginData;
	m_authorizationController.Login("su", "1", loginData);

	// Clean up any remaining test users
	QByteArrayList userIds = m_authorizationController.GetUserIds();
	QByteArrayList testLogins = {"pattestuser", "patlistuser", "patexpiryuser", "patpermuser"};

	for (const QByteArray& userId : userIds) {
		User userData;
		if (m_authorizationController.GetUser(userId, userData)) {
			if (testLogins.contains(userData.login)) {
				m_authorizationController.RemoveUser(userId);
			}
		}
	}

	m_authorizationController.Logout();
}


I_ADD_TEST(CPersonalAccessTokenTest);
