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
	authsdl::V1_0::AuthorizationRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	bool ok = AddUser(userInfo);
	QVERIFY(ok);

	arguments.input.Login = "Test1";
	arguments.input.Password = "Test1";
	arguments.input.ProductId = "Test";

	authsdl::CAuthorizationPayload::V1_0 response;

	ok = SendRequest<
		authsdl::V1_0::CAuthorizationGqlRequest,
		authsdl::V1_0::AuthorizationRequestArguments,
		authsdl::CAuthorizationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(userInfo.GetId() == "Test1");
	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CAuthorizationControllerTest::AuthorizationFailedTest()
{
	namespace authsdl = sdl::imtauth::Authorization;
	authsdl::V1_0::AuthorizationRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	arguments.input.Login = "Test";
	arguments.input.Password = "2";
	arguments.input.ProductId = "Test";

	authsdl::CAuthorizationPayload::V1_0 response;

	bool ok = SendRequest<
		authsdl::V1_0::CAuthorizationGqlRequest,
		authsdl::V1_0::AuthorizationRequestArguments,
		authsdl::CAuthorizationPayload>(arguments, response);

	QVERIFY(!ok);

	arguments.input.Login = "Test344";
	arguments.input.Password = "1";

	ok = SendRequest<
		authsdl::V1_0::CAuthorizationGqlRequest,
		authsdl::V1_0::AuthorizationRequestArguments,
		authsdl::CAuthorizationPayload>(arguments, response);

	QVERIFY(!ok);

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


// I_ADD_TEST(CAuthorizationControllerTest);


