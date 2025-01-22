#pragma once


// Qt includes
#include <QtTest/QTest>

// ImtCore includes
#include <imtgql/Test/CGqlSdlRequestTest.h>
#include <imtcrypt/CMD5HashCalculator.h>
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Authorization.h>


class CUserCollectionControllerTest: public QObject, public imtgql::CGqlSdlRequestTest
{
	Q_OBJECT
private Q_SLOTS:
	void initTestCase();
	void AddUserTest();
	void AddUserFailedTest();
	void RemoveUserTest();
	void RemoveUserFailedTest();
	void UpdateUserTest();
	void UpdateUserFailedTest();
	void GetUserListTest();
	void cleanupTestCase();

private:
	QByteArray AddUser(const QString& login, const QString& password) const;
	bool RemoveUser(const QByteArray& userId) const;

private:
	imtcrypt::CMD5HashCalculator m_hashCalculator;
};


