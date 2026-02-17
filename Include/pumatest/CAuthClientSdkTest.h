// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-Puma-Commercial
#pragma once


// Qt includes
#include <QtTest/QtTest>

// ImtCore includes
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Users.h>

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
	void UserCrudTest();
	void RoleCrudTest();
	void GroupCrudTest();

	void cleanupTestCase();

private:
	AuthServerSdk::CAuthorizableServer m_authorizableServer;
	AuthClientSdk::CAuthorizationController m_authorizationController;
};


