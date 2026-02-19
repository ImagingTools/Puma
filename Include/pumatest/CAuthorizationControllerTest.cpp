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
	namespace authsdl = sdl::imtauth::Authorization;
	authsdl::AuthorizationRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	bool ok = AddUser(userInfo);
	QVERIFY(ok);

	arguments.input.Version_1_0->login = "Test1";
	arguments.input.Version_1_0->password = "Test1";
	arguments.input.Version_1_0->productId = "Test";

	authsdl::CAuthorizationPayload::V1_0 response;

	ok = SendRequest<
		authsdl::CAuthorizationGqlRequest,
		authsdl::AuthorizationRequestArguments,
		authsdl::CAuthorizationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(userInfo.GetId() == "Test1");
	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CAuthorizationControllerTest::AuthorizationFailedTest()
{
	namespace authsdl = sdl::imtauth::Authorization;
	authsdl::AuthorizationRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	arguments.input.Version_1_0->login = "Test";
	arguments.input.Version_1_0->password = "2";
	arguments.input.Version_1_0->productId = "Test";

	authsdl::CAuthorizationPayload::V1_0 response;

	bool ok = SendRequest<
		authsdl::CAuthorizationGqlRequest,
		authsdl::AuthorizationRequestArguments,
		authsdl::CAuthorizationPayload>(arguments, response);

	QVERIFY(!ok);

	arguments.input.Version_1_0->login = "Test344";
	arguments.input.Version_1_0->password = "1";

	ok = SendRequest<
		authsdl::CAuthorizationGqlRequest,
		authsdl::AuthorizationRequestArguments,
		authsdl::CAuthorizationPayload>(arguments, response);

	QVERIFY(!ok);

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


// I_ADD_TEST(CAuthorizationControllerTest);


