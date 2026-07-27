// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#include "CPersonalAccessTokenTest.h"


// ACF includes
#include <itest/CStandardTestExecutor.h>


using namespace AuthClientSdk;


static const QByteArrayList s_testLogins = {"pattestuser", "patlistuser", "patvalidateuser", "patauthusera", "patauthuserb", "patrevokeuser", "patexpiryuser", "patpermuser"};


void CPersonalAccessTokenTest::initTestCase()
{
	qDebug() << "=== [Init] PersonalAccessToken tests start ===";

	// Ports match PumaServerSlTest.acc (isolated, disposable-DB test server),
	// not the plain PumaServerSl.exe (17789/18789 vs. 7788/8788) - this suite
	// must never touch a developer's persistent local Puma database.
	AuthServerSdk::ServerConfig pumaConfig;
	pumaConfig.wsPort = 18789;
	pumaConfig.httpPort = 17789;

	m_authorizableServer.SetPumaConnectionParam(pumaConfig);
	m_authorizableServer.SetProductId("Test");

	AuthServerSdk::ServerConfig config;
	config.wsPort = 8889;
	config.httpPort = 7778;

	QVERIFY(m_authorizableServer.Start(config));

	m_authorizationController.SetProductId("Test");

	AuthClientSdk::ServerConfig serverConfig;
	serverConfig.wsPort = 8889;
	serverConfig.httpPort = 7778;
	m_authorizationController.SetConnectionParam(serverConfig);

	// Idempotent superuser bootstrap: this test class must be runnable on its
	// own, without depending on CAuthClientSdkTest having run first in the
	// same process and already created "su"/"1" in the shared backend.
	Login loginData;
	if (m_authorizationController.Login("su", "1", loginData)) {
		QVERIFY(m_authorizationController.Logout());
	}
	else {
		QString err;
		SuperuserStatus status = m_authorizationController.SuperuserExists(err);
		QVERIFY2(err.isEmpty(), qPrintable(err));
		QCOMPARE(status, SuperuserStatus::NotExists);
		QVERIFY(m_authorizationController.CreateSuperuser("1"));
	}
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
	QVERIFY2(!token.isEmpty(), "CreatePersonalAccessToken returned an empty raw token");

	// A second token for the same user/product must be a distinct secret,
	// even with the same name (PAT names are not unique identifiers).
	QByteArray token2 = m_authorizationController.CreatePersonalAccessToken(
		userId,
		"Test",
		"CI Pipeline Token",
		{"ReadData", "WriteData"},
		"");
	QVERIFY(!token2.isEmpty());
	QVERIFY2(token != token2, "Two CreatePersonalAccessToken calls produced the same raw token");

	// Create a PAT with a (future) expiration date
	QByteArray tokenWithExpiry = m_authorizationController.CreatePersonalAccessToken(
		userId,
		"Test",
		"Expiring Token",
		{"ReadData"},
		"2099-12-31T23:59:59Z");
	QVERIFY(!tokenWithExpiry.isEmpty());

	// CreatePersonalAccessToken() without prior Login() must fail. See
	// PatOwnershipAuthorizationTest() for the fuller anonymous/self/cross-user/
	// admin authorization matrix (imtauthgql::CPersonalAccessTokenControllerComp::
	// IsCallerAuthorizedForUser()).
	QVERIFY(m_authorizationController.Logout());
	QByteArray anonymousToken = m_authorizationController.CreatePersonalAccessToken(
		userId, "Test", "Anonymous Token", {"ReadData"}, "");
	QVERIFY2(anonymousToken.isEmpty(), "PAT was created without authorization");

	// Cleanup
	QVERIFY(m_authorizationController.Login("su", "1", loginData));
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

	// No tokens exist yet for a freshly created user.
	QVERIFY(m_authorizationController.ListPersonalAccessTokens(userId).isEmpty());

	QByteArray tokenTest = m_authorizationController.CreatePersonalAccessToken(
		userId, "Test", "Token For Test Product", {"ReadData", "WriteData"}, "");
	QVERIFY(!tokenTest.isEmpty());

	QByteArray tokenOther = m_authorizationController.CreatePersonalAccessToken(
		userId, "Other", "Token For Other Product", {"AdminAccess"}, "");
	QVERIFY(!tokenOther.isEmpty());

	// Unfiltered list must contain both tokens.
	QList<PersonalAccessToken> allTokens = m_authorizationController.ListPersonalAccessTokens(userId);
	QCOMPARE(allTokens.size(), 2);

	// Product-filtered list must contain only the matching token, and its
	// fields must reflect exactly what was passed to CreatePersonalAccessToken().
	QList<PersonalAccessToken> testTokens = m_authorizationController.ListPersonalAccessTokens(userId, "Test");
	QCOMPARE(testTokens.size(), 1);

	const PersonalAccessToken& listedToken = testTokens.first();
	QVERIFY(!listedToken.id.isEmpty());
	QCOMPARE(listedToken.userId, userId);
	QCOMPARE(listedToken.productId, QByteArray("Test"));
	QCOMPARE(listedToken.name, QString("Token For Test Product"));
	QVERIFY(listedToken.description.isEmpty());
	QCOMPARE(listedToken.permissions.size(), 2);
	QVERIFY(listedToken.permissions.contains("ReadData"));
	QVERIFY(listedToken.permissions.contains("WriteData"));
	QVERIFY(!listedToken.creationDate.isEmpty());
	QVERIFY2(listedToken.expirationDate.isEmpty(), "Token created without expiration must report an empty expirationDate");
	QVERIFY(listedToken.isActive);

	QList<PersonalAccessToken> otherTokens = m_authorizationController.ListPersonalAccessTokens(userId, "Other");
	QCOMPARE(otherTokens.size(), 1);
	QCOMPARE(otherTokens.first().productId, QByteArray("Other"));

	// A product filter that matches nothing must return an empty list, not
	// fall back to the unfiltered set.
	QVERIFY(m_authorizationController.ListPersonalAccessTokens(userId, "NoSuchProduct").isEmpty());

	// Cleanup
	QVERIFY(m_authorizationController.RemoveUser(userId));
	QVERIFY(m_authorizationController.Logout());
}


void CPersonalAccessTokenTest::ValidateTokenTest()
{
	qDebug() << "=== [ValidateTokenTest] ===";

	// Validate an empty token: must fail without contacting the server.
	PersonalAccessTokenValidation emptyResult = m_authorizationController.ValidatePersonalAccessToken(QByteArray());
	QVERIFY(!emptyResult.isValid);
	QVERIFY(emptyResult.userId.isEmpty());
	QVERIFY(emptyResult.permissions.isEmpty());

	// Validate a well-formed but unknown token.
	PersonalAccessTokenValidation unknownResult = m_authorizationController.ValidatePersonalAccessToken("non-existent-token");
	QVERIFY(!unknownResult.isValid);

	Login loginData;
	QVERIFY(m_authorizationController.Login("su", "1", loginData));

	QByteArray userId = m_authorizationController.CreateUser(
		"PatValidateUser", "patvalidateuser", "1", "patvalidate@example.com");
	QVERIFY(!userId.isEmpty());

	QByteArray token = m_authorizationController.CreatePersonalAccessToken(
		userId, "Test", "Validate Test Token", {"ReadData", "WriteData"}, "");
	QVERIFY(!token.isEmpty());

	PersonalAccessTokenValidation result = m_authorizationController.ValidatePersonalAccessToken(token);
	QVERIFY2(result.isValid, "A freshly created, non-expired, non-revoked PAT must validate");
	QCOMPARE(result.userId, userId);
	QCOMPARE(result.permissions.size(), 2);
	QVERIFY(result.permissions.contains("ReadData"));
	QVERIFY(result.permissions.contains("WriteData"));
	// Known ImtCore limitation: the ValidateToken GraphQL query does not
	// return the token id, so the client cannot look up and report the
	// token's product scope here. See Docs/AuthClientSdk.md.
	QVERIFY(result.productId.isEmpty());

	// Validating garbage that merely happens to share a prefix must still fail.
	PersonalAccessTokenValidation tamperedResult = m_authorizationController.ValidatePersonalAccessToken(token + "x");
	QVERIFY(!tamperedResult.isValid);

	// Cleanup
	QVERIFY(m_authorizationController.RemoveUser(userId));
	QVERIFY(m_authorizationController.Logout());
}


void CPersonalAccessTokenTest::PatOwnershipAuthorizationTest()
{
	qDebug() << "=== [PatOwnershipAuthorizationTest] ===";

	// Exercises imtauthgql::CPersonalAccessTokenControllerComp::
	// IsCallerAuthorizedForUser(): anonymous callers and non-owning,
	// non-admin users must be rejected for every PAT mutation/query, while
	// the token owner (self-service) and admins must still be allowed.

	Login loginData;
	QVERIFY(m_authorizationController.Login("su", "1", loginData));

	QByteArray userAId = m_authorizationController.CreateUser(
		"PatAuthUserA", "patauthusera", "1", "patauthusera@example.com");
	QVERIFY(!userAId.isEmpty());

	QByteArray userBId = m_authorizationController.CreateUser(
		"PatAuthUserB", "patauthuserb", "1", "patauthuserb@example.com");
	QVERIFY(!userBId.isEmpty());

	// su (admin) mints a token for User B up front, to later probe cross-user
	// and admin access to an *existing* token id.
	QByteArray userBToken = m_authorizationController.CreatePersonalAccessToken(
		userBId, "Test", "User B Token", {"ReadData"}, "");
	QVERIFY(!userBToken.isEmpty());
	QList<PersonalAccessToken> userBTokens = m_authorizationController.ListPersonalAccessTokens(userBId);
	QCOMPARE(userBTokens.size(), 1);
	QByteArray userBTokenId = userBTokens.first().id;

	QVERIFY(m_authorizationController.Logout());

	// Anonymous (no login at all) must not be able to create, list, or
	// revoke PATs for anyone.
	QVERIFY2(m_authorizationController.CreatePersonalAccessToken(userAId, "Test", "Anonymous Attempt", {"ReadData"}, "").isEmpty(),
		"Unauthenticated caller was able to create a PAT");
	QVERIFY2(m_authorizationController.ListPersonalAccessTokens(userAId).isEmpty(),
		"Unauthenticated caller was able to list another user's PATs");
	QVERIFY2(!m_authorizationController.RevokePersonalAccessToken(userBTokenId),
		"Unauthenticated caller was able to revoke a PAT");
	QVERIFY(m_authorizationController.ValidatePersonalAccessToken(userBToken).isValid);

	// User A (non-admin) creates a PAT for themself - self-service must work.
	QVERIFY(m_authorizationController.Login("patauthusera", "1", loginData));
	QByteArray selfToken = m_authorizationController.CreatePersonalAccessToken(
		userAId, "Test", "Self Service Token", {"ReadData"}, "");
	QVERIFY2(!selfToken.isEmpty(), "A regular user could not create a PAT for themself");

	QList<PersonalAccessToken> ownTokens = m_authorizationController.ListPersonalAccessTokens(userAId);
	QCOMPARE(ownTokens.size(), 1);
	QByteArray ownTokenId = ownTokens.first().id;

	// ...but User A must not be able to create, list, or revoke User B's PATs.
	QVERIFY2(m_authorizationController.CreatePersonalAccessToken(userBId, "Test", "Cross User Attempt", {"ReadData"}, "").isEmpty(),
		"A regular user was able to create a PAT for a different user");
	QVERIFY2(m_authorizationController.ListPersonalAccessTokens(userBId).isEmpty(),
		"A regular user was able to list a different user's PATs");
	QVERIFY2(!m_authorizationController.RevokePersonalAccessToken(userBTokenId),
		"A regular user was able to revoke a different user's PAT");
	QVERIFY2(m_authorizationController.ValidatePersonalAccessToken(userBToken).isValid,
		"A different user's PAT was revoked by a non-owner, non-admin caller");

	// User A can still revoke their own token.
	QVERIFY2(m_authorizationController.RevokePersonalAccessToken(ownTokenId),
		"A regular user could not revoke their own PAT");
	QVERIFY(!m_authorizationController.ValidatePersonalAccessToken(selfToken).isValid);

	QVERIFY(m_authorizationController.Logout());

	// Admin (su) can still manage User B's tokens.
	QVERIFY(m_authorizationController.Login("su", "1", loginData));
	QVERIFY2(m_authorizationController.RevokePersonalAccessToken(userBTokenId),
		"An admin could not revoke another user's PAT");

	// Cleanup
	QVERIFY(m_authorizationController.RemoveUser(userAId));
	QVERIFY(m_authorizationController.RemoveUser(userBId));
	QVERIFY(m_authorizationController.Logout());
}


void CPersonalAccessTokenTest::RevokeTokenTest()
{
	qDebug() << "=== [RevokeTokenTest] ===";

	// Revoke a non-existent token id.
	QVERIFY(!m_authorizationController.RevokePersonalAccessToken("non-existent-id"));

	Login loginData;
	QVERIFY(m_authorizationController.Login("su", "1", loginData));

	QByteArray userId = m_authorizationController.CreateUser(
		"PatRevokeUser", "patrevokeuser", "1", "patrevoke@example.com");
	QVERIFY(!userId.isEmpty());

	QByteArray token = m_authorizationController.CreatePersonalAccessToken(
		userId, "Test", "Revoke Test Token", {"ReadData"}, "");
	QVERIFY(!token.isEmpty());

	QList<PersonalAccessToken> tokens = m_authorizationController.ListPersonalAccessTokens(userId);
	QCOMPARE(tokens.size(), 1);
	QByteArray tokenId = tokens.first().id;
	QVERIFY(!tokenId.isEmpty());
	QVERIFY(tokens.first().isActive);

	// The token must still validate before it is revoked.
	QVERIFY(m_authorizationController.ValidatePersonalAccessToken(token).isValid);

	QVERIFY2(m_authorizationController.RevokePersonalAccessToken(tokenId), "RevokePersonalAccessToken failed for an existing token");

	// After revocation: validation must fail and the listing must report
	// the token as inactive (the record itself is retained for audit).
	QVERIFY(!m_authorizationController.ValidatePersonalAccessToken(token).isValid);

	QList<PersonalAccessToken> tokensAfterRevoke = m_authorizationController.ListPersonalAccessTokens(userId);
	QCOMPARE(tokensAfterRevoke.size(), 1);
	QCOMPARE(tokensAfterRevoke.first().id, tokenId);
	QVERIFY(!tokensAfterRevoke.first().isActive);

	// Revoking an already-revoked (but still existing) token is idempotent
	// and reports success again.
	QVERIFY(m_authorizationController.RevokePersonalAccessToken(tokenId));

	// Cleanup
	QVERIFY(m_authorizationController.RemoveUser(userId));
	QVERIFY(m_authorizationController.Logout());
}


void CPersonalAccessTokenTest::ExpiredTokenTest()
{
	qDebug() << "=== [ExpiredTokenTest] ===";

	Login loginData;
	QVERIFY(m_authorizationController.Login("su", "1", loginData));

	QByteArray userId = m_authorizationController.CreateUser(
		"PatExpiryUser", "patexpiryuser", "1", "patexpiry@example.com");
	QVERIFY(!userId.isEmpty());

	// A PAT with a past expiration date is still created successfully -
	// CreatePersonalAccessToken() does not reject dates in the past - but
	// must never validate as usable.
	QByteArray expiredToken = m_authorizationController.CreatePersonalAccessToken(
		userId,
		"Test",
		"Expired Token",
		{"ReadData"},
		"2020-01-01T00:00:00Z");
	QVERIFY(!expiredToken.isEmpty());

	PersonalAccessTokenValidation result = m_authorizationController.ValidatePersonalAccessToken(expiredToken);
	QVERIFY2(!result.isValid, "An already-expired PAT must not validate");

	// isActive must be false for an expired-but-not-revoked token: it
	// reflects usability (not revoked AND not expired), not just revocation.
	QList<PersonalAccessToken> tokens = m_authorizationController.ListPersonalAccessTokens(userId, "Test");
	QCOMPARE(tokens.size(), 1);
	QVERIFY(!tokens.first().expirationDate.isEmpty());
	QVERIFY2(!tokens.first().isActive, "Expired token must be reported inactive even though it was never revoked");

	// A token created in the same call with a future expiration must
	// contrast correctly: still valid and still active.
	QByteArray liveToken = m_authorizationController.CreatePersonalAccessToken(
		userId, "Test", "Still Valid Token", {"ReadData"}, "2099-01-01T00:00:00Z");
	QVERIFY(!liveToken.isEmpty());
	QVERIFY(m_authorizationController.ValidatePersonalAccessToken(liveToken).isValid);

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

	QByteArray roleName = QByteArrayLiteral("PatPermissionRole_") + userId;
	QByteArray roleId = m_authorizationController.CreateRole(
		roleName, "", {"AdminAccess", "ReadOnly", "NotInTokenScope"});
	QVERIFY(!roleId.isEmpty());
	QVERIFY(m_authorizationController.AddRolesToUser(userId, {roleId}));

	QByteArray token = m_authorizationController.CreatePersonalAccessToken(
		userId, "Test", "Permission Test Token", {"AdminAccess", "ReadOnly"});
	QVERIFY(!token.isEmpty());

	// The PAT itself validates correctly and reports its configured scopes.
	PersonalAccessTokenValidation validation = m_authorizationController.ValidatePersonalAccessToken(token);
	QVERIFY(validation.isValid);
	QCOMPARE(validation.permissions.size(), 2);

	// GetPermissions returns the intersection of the user's permissions and
	// the PAT scopes, excluding permissions granted only through the role.
	QByteArrayList sessionPermissions = m_authorizationController.GetTokenPermissions(token);
	QCOMPARE(sessionPermissions.size(), 2);
	QVERIFY(sessionPermissions.contains("AdminAccess"));
	QVERIFY(sessionPermissions.contains("ReadOnly"));
	QVERIFY(!sessionPermissions.contains("NotInTokenScope"));

	// Cleanup
	QVERIFY(m_authorizationController.RemoveUser(userId));
	QVERIFY(m_authorizationController.RemoveRole(roleId));
	QVERIFY(m_authorizationController.Logout());
}


void CPersonalAccessTokenTest::cleanupTestCase()
{
	qDebug() << "=== [Cleanup] PersonalAccessToken tests completed ===";

	Login loginData;
	QVERIFY(m_authorizationController.Login("su", "1", loginData));

	// Clean up any remaining test users (defensive: covers the case where
	// an earlier assertion failed before a test's own cleanup ran).
	QByteArrayList userIds = m_authorizationController.GetUserIds();
	for (const QByteArray& userId : userIds) {
		User userData;
		if (m_authorizationController.GetUser(userId, userData)) {
			if (s_testLogins.contains(userData.login)) {
				QVERIFY(m_authorizationController.RemoveUser(userId));
			}
		}
	}

	QVERIFY(m_authorizationController.Logout());
}


I_ADD_TEST(CPersonalAccessTokenTest);
