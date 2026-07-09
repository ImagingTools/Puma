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

	\note These tests require a running Puma server with PAT support
	      enabled (IPersonalAccessTokenManager interface from ImtCore).
	      Tests are designed to verify the SDK API surface and will
	      pass with placeholder implementations (returning empty/false)
	      until the server-side support is available.

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
	void RevokeTokenTest();
	void ExpiredTokenTest();
	void GetPermissionsWithPatTest();

	void cleanupTestCase();

private:
	AuthServerSdk::CAuthorizableServer m_authorizableServer;
	AuthClientSdk::CAuthorizationController m_authorizationController;
};
