// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#include "CAuthorizationControllerTest.h"


// ACF includes
#include <istd/TDelPtr.h>
#include <itest/CStandardTestExecutor.h>
#include <iser/CJsonMemWriteArchive.h>
#include <iser/CJsonMemReadArchive.h>

// ImtCore includes
#include <imtauth/CUserInfo.h>
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Authorization.h>


static const QString s_usersTableName = "Users";

// private slots

void CAuthorizationControllerTest::AuthorizationTest()
{
	namespace authsdl = sdl::V1_0::imtauth;
	authsdl::AuthorizationRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	bool ok = AddUser(userInfo);
	QVERIFY(ok);

	arguments.input.emplace();
	arguments.input->login = "Test1";
	arguments.input->password = "Test1";
	arguments.input->productId = "Test";

	authsdl::CAuthorizationPayload response;

	QVERIFY(ok);
	QVERIFY(userInfo.GetId() == "Test1");
	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CAuthorizationControllerTest::AuthorizationFailedTest()
{
	namespace authsdl = sdl::V1_0::imtauth;
	authsdl::AuthorizationRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	arguments.input.emplace();
	arguments.input->login = "Test";
	arguments.input->password = "2";
	arguments.input->productId = "Test";

	authsdl::CAuthorizationPayload response;

	bool ok = false;

	QVERIFY(!ok);

	arguments.input->login = "Test344";
	arguments.input->password = "1";

	QVERIFY(!ok);

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


// I_ADD_TEST(CAuthorizationControllerTest);


