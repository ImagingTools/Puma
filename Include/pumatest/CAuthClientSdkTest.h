// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#pragma once


// Qt includes
#include <QtTest/QtTest>

// Puma includes
#include <AuthClientSdk/AuthClientSdk.h>
#include <AuthServerSdk/AuthServerSdk.h>


class CAuthClientSdkTest: public QObject
{
	Q_OBJECT
private slots:
	void initTestCase();

	void SuperuserExistsTest();
	void LoginLogoutTest();
	void GetTokenPermissionsTest();
	void UserCrudTest();
	void RoleCrudTest();
	void GroupCrudTest();

	void cleanupTestCase();

private:
	AuthServerSdk::CAuthorizableServer m_authorizableServer;
	AuthClientSdk::CAuthorizationController m_authorizationController;
};


