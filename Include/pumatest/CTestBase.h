#pragma once


// Qt includes
#include <QtTest/QTest>

// ImtCore includes
#include <imtgqltest/CGqlSdlRequestTest.h>
#include <imtauth/CUserInfo.h>
#include <imtauth/CRole.h>
#include <imtauth/CUserGroupInfo.h>
#include <imtcrypt/CMD5HashCalculator.h>
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Users.h>
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Roles.h>


namespace pumatest
{


class CTestBase: public imtgqltest::CGqlSdlRequestTest
{
	Q_OBJECT
public slots:
	void initTestCase();
	void cleanupTestCase();

protected:
	virtual void SetData() const;
	void ClearData();
	bool GetObjectFromTable(const QString& tableName, const QByteArray& userId, iser::ISerializable& object) const;
	bool InsertObjectToTable(const QString& tableName, iser::ISerializable& object) const;
	bool RemoveObjectFromTable(const QString& tableName, const QByteArray& userId) const;
	bool AddUser(const imtauth::CIdentifiableUserInfo& userData) const;
	bool AddRole(const imtauth::CIdentifiableRoleInfo& roleData) const;
	bool AddGroup(const imtauth::CIdentifiableUserGroupInfo& groupData) const;
	imtauth::IUserInfo* CreateUserInfo(const QString& username, const QString& password, const QString& name, const QString& email) const;
	sdl::imtauth::Users::CUserData::V1_0* CreateUserDataFromUserInfo(const imtauth::CIdentifiableUserInfo& userInfo, const QByteArray& productId = QByteArray()) const;
	imtauth::IRole* CreateRoleInfo(const QString& roleId, const QString& roleName, const QString& productId, QByteArrayList features) const;
	sdl::imtauth::Roles::CRoleData::V1_0* CreateRoleDataFromUserInfo(const imtauth::CIdentifiableRoleInfo& roleInfo) const;

protected:
	imtcrypt::CMD5HashCalculator m_hashCalculator;
};


} // namespace pumatest
