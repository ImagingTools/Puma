// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#pragma once


// Qt includes
#include <QtTest/QtTest>

// Puma includes
#include <AuthClientSdk/AuthClientSdk.h>
#include <AuthServerSdk/AuthServerSdk.h>


/**
	\brief Test class for Personal Access Token (PAT) management.

	Tests the PAT-related functionality of the AuthClientSdk,
	including creation, listing, validation, and revocation of
	Personal Access Tokens.

	\note These tests connect to a real Puma backend through
	      AuthServerSdk::SetPumaConnectionParam() (default host/port
	      7788/8788) and assert on real PAT create/list/validate/revoke
	      behavior; they are not satisfied by placeholder/no-op returns.

	\ingroup AuthClientSdk
*/
class CPersonalAccessTokenTest: public QObject
{
	Q_OBJECT
private slots:
	void initTestCase();

	void CreateTokenTest();
	void ListTokensTest();
	void ValidateTokenTest();
	void PatOwnershipAuthorizationTest();
	void RevokeTokenTest();
	void ExpiredTokenTest();
	void GetPermissionsWithPatTest();

	void cleanupTestCase();

private:
	AuthServerSdk::CAuthorizableServer m_authorizableServer;
	AuthClientSdk::CAuthorizationController m_authorizationController;
};
