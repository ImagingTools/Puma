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
	bool CreateUser(const QByteArray& login, const QString& roleId, const QByteArrayList& permissions, AuthClientSdk::User& userData);
	bool RemoveUser(const QByteArray& login);

private:
	AuthClientSdk::CAuthorizationController m_authorizationController;
};


