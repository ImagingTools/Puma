// SPDX-License-Identifier: MIT
#include "CUserControllerTest.h"


// ACF includes
#include <itest/CStandardTestExecutor.h>


static const QString s_usersTableName = "Users";


// private Q_SLOTS methods

void CUserControllerTest::ChangePasswordTest()
{
	namespace userssdl = sdl::imtauth::Users;
	userssdl::ChangePasswordRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	bool ok = AddUser(userInfo);
	QVERIFY(ok);

	arguments.input.Version_1_0->login = userInfo.GetId();
	arguments.input.Version_1_0->oldPassword = "1";
	arguments.input.Version_1_0->newPassword = "2";

	userssdl::CChangePasswordPayload::V1_0 response;

	ok = SendRequest<
				sdl::imtauth::Users::CChangePasswordGqlRequest,
				sdl::imtauth::Users::ChangePasswordRequestArguments,
				sdl::imtauth::Users::CChangePasswordPayload>(arguments, response);
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
	namespace userssdl = sdl::imtauth::Users;
	userssdl::ChangePasswordRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	bool ok = AddUser(userInfo);
	QVERIFY(ok);

	arguments.input.Version_1_0->login = "Test";
	arguments.input.Version_1_0->oldPassword = "3";
	arguments.input.Version_1_0->newPassword = "2";

	userssdl::CChangePasswordPayload::V1_0 response;

	ok = SendRequest<
		sdl::imtauth::Users::CChangePasswordGqlRequest,
		sdl::imtauth::Users::ChangePasswordRequestArguments,
		sdl::imtauth::Users::CChangePasswordPayload>(arguments, response);
	QVERIFY(!ok);
	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::RegisterUserTest(){
	namespace userssdl = sdl::imtauth::Users;
	userssdl::RegisterUserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Ivanov");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test12345");
	userInfo.SetPasswordHash(passwordHash);
	userInfo.SetMail("ivanov@mail.ru");
	userInfo.SetName("Ivanov");

	userssdl::CUserData::V1_0 userData;
	userData.id = uuid;
	userData.username = userInfo.GetId();
	userData.password = "12345";
	userData.name = userInfo.GetName();
	userData.email = userInfo.GetMail();

	arguments.input.Version_1_0->userData = userData;
	arguments.input.Version_1_0->productId = "Test";

	userssdl::CRegisterUserPayload::V1_0 response;

	bool ok = SendRequest<
		sdl::imtauth::Users::CRegisterUserGqlRequest,
		sdl::imtauth::Users::RegisterUserRequestArguments,
		sdl::imtauth::Users::CRegisterUserPayload>(arguments, response);
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
	namespace userssdl = sdl::imtauth::Users;
	userssdl::RegisterUserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);
	userInfo.SetMail("test@mail.ru");
	userInfo.SetName("Test");

	userssdl::CUserData::V1_0 userData;
	userData.id = uuid;
	userData.username = "";
	userData.password = "1";
	userData.name = userInfo.GetName();
	userData.email = userInfo.GetMail();

	arguments.input.Version_1_0->userData = userData;
	arguments.input.Version_1_0->productId = "Test";

	userssdl::CRegisterUserPayload::V1_0 response;

	bool ok = SendRequest<
		userssdl::CRegisterUserGqlRequest,
		userssdl::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Check empty login
	QVERIFY(!ok);

	userData.username = "Test";
	userData.password = "";
	arguments.input.Version_1_0->userData = userData;

	ok = SendRequest<
		userssdl::CRegisterUserGqlRequest,
		userssdl::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Check empty password
	QVERIFY(!ok);

	userData.password = "1";
	userData.name = "";
	arguments.input.Version_1_0->userData = userData;

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

	arguments.input.Version_1_0->userData = userData;

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
	namespace userssdl = sdl::imtauth::Users;
	userssdl::CheckEmailRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("Test1"));
	userInfo.SetMail("test@mail.ru");
	userInfo.SetName("Test");

	QVERIFY(AddUser(userInfo));

	arguments.input.Version_1_0->email = userInfo.GetMail();

	userssdl::CCheckEmailPayload::V1_0 response;

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
	namespace userssdl = sdl::imtauth::Users;
	userssdl::CheckEmailRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("Test1"));
	userInfo.SetMail("test@mail.ru");
	userInfo.SetName("Test");

	QVERIFY(AddUser(userInfo));

	arguments.input.Version_1_0->email = "";

	userssdl::CCheckEmailPayload::V1_0 response;

	bool ok = SendRequest<
		userssdl::CCheckEmailGqlRequest,
		userssdl::CheckEmailRequestArguments,
		userssdl::CCheckEmailPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(!*response.success);
	QVERIFY(!response.message->isEmpty());

	arguments.input.Version_1_0->email = "test2@mail.ru";

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
	namespace userssdl = sdl::imtauth::Users;
	userssdl::CreateSuperuserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	userInfo.SetObjectUuid("su");
	userInfo.SetId("su");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("su1"));
	userInfo.SetMail("su@mail.ru");
	userInfo.SetName("superuser");

	arguments.input.Version_1_0->name = userInfo.GetName();
	arguments.input.Version_1_0->mail = userInfo.GetMail();
	arguments.input.Version_1_0->password = "1";

	userssdl::CCreateSuperuserPayload::V1_0 response;

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
	namespace userssdl = sdl::imtauth::Users;
	userssdl::CreateSuperuserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	userInfo.SetObjectUuid("su");
	userInfo.SetId("su");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("su1"));
	userInfo.SetMail("su@mail.ru");
	userInfo.SetName("superuser");

	QVERIFY(AddUser(userInfo));

	arguments.input.Version_1_0->name = userInfo.GetName();
	arguments.input.Version_1_0->mail = "su@mail.ru";
	arguments.input.Version_1_0->password = "1";

	userssdl::CCreateSuperuserPayload::V1_0 response;

	bool ok = SendRequest<
		userssdl::CCreateSuperuserGqlRequest,
		userssdl::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	// Empty already exists
	QVERIFY(ok);
	QVERIFY(response.success.has_value() && !*response.success);
	QVERIFY(response.message.has_value() && !response.message->isEmpty());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, "su"));

	arguments.input.Version_1_0->name = userInfo.GetName();
	arguments.input.Version_1_0->mail = "";
	arguments.input.Version_1_0->password = "1";

	ok = SendRequest<
		userssdl::CCreateSuperuserGqlRequest,
		userssdl::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	// Empty email for su
	QVERIFY(ok);
	QVERIFY(response.success.has_value() && !*response.success);
	QVERIFY(response.message.has_value() && !response.message->isEmpty());

	arguments.input.Version_1_0->mail = "su@mail.ru";
	arguments.input.Version_1_0->password = "";

	ok = SendRequest<
		userssdl::CCreateSuperuserGqlRequest,
		userssdl::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	// Empty password for su
	QVERIFY(ok);
	QVERIFY(response.success.has_value() && !*response.success);
	QVERIFY(response.message.has_value() && !response.message->isEmpty());

	arguments.input.Version_1_0->name = userInfo.GetName();
	arguments.input.Version_1_0->mail = userInfo.GetMail();
	arguments.input.Version_1_0->password = "1";

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
	namespace userssdl = sdl::imtauth::Users;
	userssdl::CheckSuperuserExistsRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	userInfo.SetObjectUuid("su");
	userInfo.SetId("su");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("su1"));
	userInfo.SetMail("su@mail.ru");
	userInfo.SetName("superuser");

	QVERIFY(AddUser(userInfo));

	userssdl::CCheckSuperuserPayload::V1_0 response;

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
