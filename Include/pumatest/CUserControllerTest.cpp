// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#include "CUserControllerTest.h"


// ACF includes
#include <itest/CStandardTestExecutor.h>

// ImtCore includes
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Users.h>


static const QString s_usersTableName = "Users";


// private Q_SLOTS methods

void CUserControllerTest::ChangePasswordTest()
{
	namespace userssdl = sdl::V1_0::imtauth;
	userssdl::ChangePasswordRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	bool ok = AddUser(userInfo);
	QVERIFY(ok);

	arguments.input.emplace();
	arguments.input->login = userInfo.GetId();
	arguments.input->oldPassword = "1";
	arguments.input->newPassword = "2";

	userssdl::CChangePasswordPayload response;

	ok = SendRequest<
				sdl::V1_0::imtauth::CChangePasswordGqlRequest,
				sdl::V1_0::imtauth::ChangePasswordRequestArguments,
				sdl::V1_0::imtauth::CChangePasswordPayload>(arguments, response);
	QVERIFY(ok);

	QVERIFY(*response.success);

	QByteArray newPasswordHash = m_hashCalculator.GenerateHash("Test2");

	imtauth::CIdentifiableUserInfo userInfo2;

	QVERIFY(GetObjectFromTable(s_usersTableName, uuid, userInfo2));
	QVERIFY(userInfo2.GetObjectUuid() == userInfo.GetObjectUuid());
	QVERIFY(userInfo2.GetName() == userInfo.GetName());
	QVERIFY(userInfo2.GetMail() == userInfo.GetMail());
	QVERIFY(userInfo2.GetPasswordHash() != userInfo.GetPasswordHash());
	QVERIFY(userInfo2.GetPasswordHash() == newPasswordHash);
	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::ChangePasswordFailedTest()
{
	namespace userssdl = sdl::V1_0::imtauth;
	userssdl::ChangePasswordRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	bool ok = AddUser(userInfo);
	QVERIFY(ok);

	arguments.input.emplace();
	arguments.input->login = "Test";
	arguments.input->oldPassword = "3";
	arguments.input->newPassword = "2";

	userssdl::CChangePasswordPayload response;

	ok = SendRequest<
		sdl::V1_0::imtauth::CChangePasswordGqlRequest,
		sdl::V1_0::imtauth::ChangePasswordRequestArguments,
		sdl::V1_0::imtauth::CChangePasswordPayload>(arguments, response);
	QVERIFY(!ok);
	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::RegisterUserTest(){
	namespace userssdl = sdl::V1_0::imtauth;
	userssdl::RegisterUserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Ivanov");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test12345");
	userInfo.SetPasswordHash(passwordHash);
	userInfo.SetMail("ivanov@mail.ru");
	userInfo.SetName("Ivanov");

	userssdl::CUserData userData;
	userData.id = uuid;
	userData.username = userInfo.GetId();
	userData.password = "12345";
	userData.name = userInfo.GetName();
	userData.email = userInfo.GetMail();

	arguments.input.emplace();
	arguments.input->userData = userData;
	arguments.input->productId = "Test";

	userssdl::CRegisterUserPayload response;

	bool ok = SendRequest<
		sdl::V1_0::imtauth::CRegisterUserGqlRequest,
		sdl::V1_0::imtauth::RegisterUserRequestArguments,
		sdl::V1_0::imtauth::CRegisterUserPayload>(arguments, response);
	QVERIFY(ok);
	QVERIFY(response.id == uuid);

	imtauth::CIdentifiableUserInfo userInfo2;
	QVERIFY(GetObjectFromTable(s_usersTableName, uuid, userInfo2));
	QVERIFY(userInfo2.GetId() == userInfo.GetId());
	QVERIFY(userInfo2.GetName() == userInfo.GetName());
	QVERIFY(userInfo2.GetMail() == userInfo.GetMail());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::RegisterUserFailedTest()
{
	namespace userssdl = sdl::V1_0::imtauth;
	userssdl::RegisterUserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);
	userInfo.SetMail("test@mail.ru");
	userInfo.SetName("Test");

	userssdl::CUserData userData;
	userData.id = uuid;
	userData.username = "";
	userData.password = "1";
	userData.name = userInfo.GetName();
	userData.email = userInfo.GetMail();

	arguments.input.emplace();
	arguments.input->userData = userData;
	arguments.input->productId = "Test";

	userssdl::CRegisterUserPayload response;

	bool ok = SendRequest<
		userssdl::CRegisterUserGqlRequest,
		userssdl::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Check empty login
	QVERIFY(!ok);

	userData.username = "Test";
	userData.password = "";
	arguments.input->userData = userData;

	ok = SendRequest<
		userssdl::CRegisterUserGqlRequest,
		userssdl::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Check empty password
	QVERIFY(!ok);

	userData.password = "1";
	userData.name = "";
	arguments.input->userData = userData;

	ok = SendRequest<
		userssdl::CRegisterUserGqlRequest,
		userssdl::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Check empty name
	QVERIFY(!ok);

	QVERIFY(AddUser(userInfo));

	userData.id = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userData.username = "Test";
	userData.password = "1";
	userData.name = userInfo.GetName();
	userData.email = userInfo.GetMail();

	arguments.input->userData = userData;

	ok = SendRequest<
		userssdl::CRegisterUserGqlRequest,
		userssdl::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Already register user
	QVERIFY(!ok);

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::CheckEmailTest()
{
	namespace userssdl = sdl::V1_0::imtauth;
	userssdl::CheckEmailRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("Test1"));
	userInfo.SetMail("test@mail.ru");
	userInfo.SetName("Test");

	QVERIFY(AddUser(userInfo));

	arguments.input.emplace();
	arguments.input->email = userInfo.GetMail();

	userssdl::CCheckEmailPayload response;

	bool ok = SendRequest<
		userssdl::CCheckEmailGqlRequest,
		userssdl::CheckEmailRequestArguments,
		userssdl::CCheckEmailPayload>(arguments, response);

	QVERIFY(ok);

	QVERIFY(*response.success);
	QVERIFY(*response.userName == userInfo.GetName());
	QVERIFY(*response.login == userInfo.GetId());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::CheckEmailFailedTest()
{
	namespace userssdl = sdl::V1_0::imtauth;
	userssdl::CheckEmailRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("Test1"));
	userInfo.SetMail("test@mail.ru");
	userInfo.SetName("Test");

	QVERIFY(AddUser(userInfo));

	arguments.input.emplace();
	arguments.input->email = "";

	userssdl::CCheckEmailPayload response;

	bool ok = SendRequest<
		userssdl::CCheckEmailGqlRequest,
		userssdl::CheckEmailRequestArguments,
		userssdl::CCheckEmailPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(!*response.success);
	QVERIFY(!response.message->isEmpty());

	arguments.input->email = "test2@mail.ru";

	ok = SendRequest<
		userssdl::CCheckEmailGqlRequest,
		userssdl::CheckEmailRequestArguments,
		userssdl::CCheckEmailPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(!*response.success);
	QVERIFY(!response.message->isEmpty());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::CreateSuperuserTest()
{
	namespace userssdl = sdl::V1_0::imtauth;
	userssdl::CreateSuperuserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	userInfo.SetObjectUuid("su");
	userInfo.SetId("su");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("su1"));
	userInfo.SetMail("su@mail.ru");
	userInfo.SetName("superuser");

	arguments.input.emplace();
	arguments.input->name = userInfo.GetName();
	arguments.input->mail = userInfo.GetMail();
	arguments.input->password = "1";

	userssdl::CCreateSuperuserPayload response;

	bool ok = SendRequest<
		userssdl::CCreateSuperuserGqlRequest,
		userssdl::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.success.has_value() && *response.success);

	QByteArray passwordHash = m_hashCalculator.GenerateHash("su1");

	imtauth::CIdentifiableUserInfo userInfo2;
	QVERIFY(GetObjectFromTable(s_usersTableName, "su", userInfo2));

	QVERIFY(userInfo.GetMail() == userInfo2.GetMail());
	QVERIFY(userInfo.GetName() == userInfo2.GetName());
	QVERIFY(userInfo.GetPasswordHash() == userInfo2.GetPasswordHash());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, "su"));
}


void CUserControllerTest::CreateSuperuserFailedTest()
{
	namespace userssdl = sdl::V1_0::imtauth;
	userssdl::CreateSuperuserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	userInfo.SetObjectUuid("su");
	userInfo.SetId("su");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("su1"));
	userInfo.SetMail("su@mail.ru");
	userInfo.SetName("superuser");

	QVERIFY(AddUser(userInfo));

	arguments.input.emplace();
	arguments.input->name = userInfo.GetName();
	arguments.input->mail = "su@mail.ru";
	arguments.input->password = "1";

	userssdl::CCreateSuperuserPayload response;

	bool ok = SendRequest<
		userssdl::CCreateSuperuserGqlRequest,
		userssdl::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	// Empty already exists
	QVERIFY(ok);
	QVERIFY(response.success.has_value() && !*response.success);
	QVERIFY(response.message.has_value() && !response.message->isEmpty());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, "su"));

	arguments.input->name = userInfo.GetName();
	arguments.input->mail = "";
	arguments.input->password = "1";

	ok = SendRequest<
		userssdl::CCreateSuperuserGqlRequest,
		userssdl::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	// Empty email for su
	QVERIFY(ok);
	QVERIFY(response.success.has_value() && !*response.success);
	QVERIFY(response.message.has_value() && !response.message->isEmpty());

	arguments.input->mail = "su@mail.ru";
	arguments.input->password = "";

	ok = SendRequest<
		userssdl::CCreateSuperuserGqlRequest,
		userssdl::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	// Empty password for su
	QVERIFY(ok);
	QVERIFY(response.success.has_value() && !*response.success);
	QVERIFY(response.message.has_value() && !response.message->isEmpty());

	arguments.input->name = userInfo.GetName();
	arguments.input->mail = userInfo.GetMail();
	arguments.input->password = "1";

	ok = SendRequest<
		userssdl::CCreateSuperuserGqlRequest,
		userssdl::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.success.has_value() && *response.success);

	QVERIFY(RemoveObjectFromTable(s_usersTableName, "su"));
}


void CUserControllerTest::CheckSuperuserTest()
{
	namespace userssdl = sdl::V1_0::imtauth;
	userssdl::CheckSuperuserExistsRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	userInfo.SetObjectUuid("su");
	userInfo.SetId("su");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("su1"));
	userInfo.SetMail("su@mail.ru");
	userInfo.SetName("superuser");

	QVERIFY(AddUser(userInfo));

	userssdl::CCheckSuperuserPayload response;

	bool ok = SendRequest<
		userssdl::CCheckSuperuserExistsGqlRequest,
		userssdl::CheckSuperuserExistsRequestArguments,
		userssdl::CCheckSuperuserPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.message.has_value() && response.message->isEmpty());

	imtauth::CIdentifiableUserInfo userInfo2;
	QVERIFY(GetObjectFromTable(s_usersTableName, "su", userInfo2));
	QVERIFY(userInfo.GetMail() == userInfo2.GetMail());
	QVERIFY(userInfo.GetName() == userInfo2.GetName());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, "su"));

	ok = SendRequest<
		userssdl::CCheckSuperuserExistsGqlRequest,
		userssdl::CheckSuperuserExistsRequestArguments,
		userssdl::CCheckSuperuserPayload>(arguments, response);

	QVERIFY(ok);

	imtauth::CIdentifiableUserInfo userInfo3;
	QVERIFY(!GetObjectFromTable(s_usersTableName, "su", userInfo3));
}


void CUserControllerTest::CheckSuperuserFailedTest()
{
}


I_ADD_TEST(CUserControllerTest);
