#include "CUserControllerTest.h"


// ACF includes
#include <itest/CStandardTestExecutor.h>


static const QString s_usersTableName = "Users";


// private Q_SLOTS methods

void CUserControllerTest::ChangePasswordTest()
{
	namespace userssdl = sdl::imtauth::Users;
	userssdl::V1_0::ChangePasswordRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	bool ok = AddUser(userInfo);
	QVERIFY(ok);

	arguments.input.Login = userInfo.GetId();
	arguments.input.OldPassword = "1";
	arguments.input.NewPassword = "2";

	userssdl::CChangePasswordPayload::V1_0 response;

	ok = SendRequest<
				sdl::imtauth::Users::V1_0::CChangePasswordGqlRequest,
				sdl::imtauth::Users::V1_0::ChangePasswordRequestArguments,
				sdl::imtauth::Users::CChangePasswordPayload>(arguments, response);
	QVERIFY(ok);

	QVERIFY(*response.Success);

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
	userssdl::V1_0::ChangePasswordRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();

	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);

	bool ok = AddUser(userInfo);
	QVERIFY(ok);

	arguments.input.Login = "Test";
	arguments.input.OldPassword = "3";
	arguments.input.NewPassword = "2";

	userssdl::CChangePasswordPayload::V1_0 response;

	ok = SendRequest<
		sdl::imtauth::Users::V1_0::CChangePasswordGqlRequest,
		sdl::imtauth::Users::V1_0::ChangePasswordRequestArguments,
		sdl::imtauth::Users::CChangePasswordPayload>(arguments, response);
	QVERIFY(!ok);
	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::RegisterUserTest(){
	namespace userssdl = sdl::imtauth::Users;
	userssdl::V1_0::RegisterUserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Ivanov");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test12345");
	userInfo.SetPasswordHash(passwordHash);
	userInfo.SetMail("ivanov@mail.ru");
	userInfo.SetName("Ivanov");

	userssdl::CUserData::V1_0 userData;
	userData.Id = uuid;
	userData.Username = userInfo.GetId();
	userData.Password = "12345";
	userData.Name = userInfo.GetName();
	userData.Email = userInfo.GetMail();

	arguments.input.UserData = userData;
	arguments.input.ProductId = "Test";

	userssdl::CRegisterUserPayload::V1_0 response;

	bool ok = SendRequest<
		sdl::imtauth::Users::V1_0::CRegisterUserGqlRequest,
		sdl::imtauth::Users::V1_0::RegisterUserRequestArguments,
		sdl::imtauth::Users::CRegisterUserPayload>(arguments, response);
	QVERIFY(ok);
	QVERIFY(response.Id == uuid);

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
	userssdl::V1_0::RegisterUserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test1");
	userInfo.SetPasswordHash(passwordHash);
	userInfo.SetMail("test@mail.ru");
	userInfo.SetName("Test");

	userssdl::CUserData::V1_0 userData;
	userData.Id = uuid;
	userData.Username = "";
	userData.Password = "1";
	userData.Name = userInfo.GetName();
	userData.Email = userInfo.GetMail();

	arguments.input.UserData = userData;
	arguments.input.ProductId = "Test";

	userssdl::CRegisterUserPayload::V1_0 response;

	bool ok = SendRequest<
		userssdl::V1_0::CRegisterUserGqlRequest,
		userssdl::V1_0::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Check empty login
	QVERIFY(!ok);

	userData.Username = "Test";
	userData.Password = "";
	arguments.input.UserData = userData;

	ok = SendRequest<
		userssdl::V1_0::CRegisterUserGqlRequest,
		userssdl::V1_0::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Check empty password
	QVERIFY(!ok);

	userData.Password = "1";
	userData.Name = "";
	arguments.input.UserData = userData;

	ok = SendRequest<
		userssdl::V1_0::CRegisterUserGqlRequest,
		userssdl::V1_0::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Check empty name
	QVERIFY(!ok);

	QVERIFY(AddUser(userInfo));

	userData.Id = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userData.Username = "Test";
	userData.Password = "1";
	userData.Name = userInfo.GetName();
	userData.Email = userInfo.GetMail();

	arguments.input.UserData = userData;

	ok = SendRequest<
		userssdl::V1_0::CRegisterUserGqlRequest,
		userssdl::V1_0::RegisterUserRequestArguments,
		userssdl::CRegisterUserPayload>(arguments, response);

	// Already register user
	QVERIFY(!ok);

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::CheckEmailTest()
{
	namespace userssdl = sdl::imtauth::Users;
	userssdl::V1_0::CheckEmailRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("Test1"));
	userInfo.SetMail("test@mail.ru");
	userInfo.SetName("Test");

	QVERIFY(AddUser(userInfo));

	arguments.input.Email = userInfo.GetMail();

	userssdl::CCheckEmailPayload::V1_0 response;

	bool ok = SendRequest<
		userssdl::V1_0::CCheckEmailGqlRequest,
		userssdl::V1_0::CheckEmailRequestArguments,
		userssdl::CCheckEmailPayload>(arguments, response);

	QVERIFY(ok);

	QVERIFY(*response.Success);
	QVERIFY(*response.UserName == userInfo.GetName());
	QVERIFY(*response.Login == userInfo.GetId());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::CheckEmailFailedTest()
{
	namespace userssdl = sdl::imtauth::Users;
	userssdl::V1_0::CheckEmailRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Test");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("Test1"));
	userInfo.SetMail("test@mail.ru");
	userInfo.SetName("Test");

	QVERIFY(AddUser(userInfo));

	arguments.input.Email = "";

	userssdl::CCheckEmailPayload::V1_0 response;

	bool ok = SendRequest<
		userssdl::V1_0::CCheckEmailGqlRequest,
		userssdl::V1_0::CheckEmailRequestArguments,
		userssdl::CCheckEmailPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(!*response.Success);
	QVERIFY(!response.Message->isEmpty());

	arguments.input.Email = "test2@mail.ru";

	ok = SendRequest<
		userssdl::V1_0::CCheckEmailGqlRequest,
		userssdl::V1_0::CheckEmailRequestArguments,
		userssdl::CCheckEmailPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(!*response.Success);
	QVERIFY(!response.Message->isEmpty());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, uuid));
}


void CUserControllerTest::CreateSuperuserTest()
{
	namespace userssdl = sdl::imtauth::Users;
	userssdl::V1_0::CreateSuperuserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	userInfo.SetObjectUuid("su");
	userInfo.SetId("su");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("su1"));
	userInfo.SetMail("su@mail.ru");
	userInfo.SetName("superuser");

	arguments.input.Name = userInfo.GetName();
	arguments.input.Mail = userInfo.GetMail();
	arguments.input.Password = "1";

	userssdl::CCreateSuperuserPayload::V1_0 response;

	bool ok = SendRequest<
		userssdl::V1_0::CCreateSuperuserGqlRequest,
		userssdl::V1_0::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.Success.has_value() && *response.Success);

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
	userssdl::V1_0::CreateSuperuserRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	userInfo.SetObjectUuid("su");
	userInfo.SetId("su");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("su1"));
	userInfo.SetMail("su@mail.ru");
	userInfo.SetName("superuser");

	QVERIFY(AddUser(userInfo));

	arguments.input.Name = userInfo.GetName();
	arguments.input.Mail = "su@mail.ru";
	arguments.input.Password = "1";

	userssdl::CCreateSuperuserPayload::V1_0 response;

	bool ok = SendRequest<
		userssdl::V1_0::CCreateSuperuserGqlRequest,
		userssdl::V1_0::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	// Empty already exists
	QVERIFY(ok);
	QVERIFY(response.Success.has_value() && !*response.Success);
	QVERIFY(response.Message.has_value() && !response.Message->isEmpty());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, "su"));

	arguments.input.Name = userInfo.GetName();
	arguments.input.Mail = "";
	arguments.input.Password = "1";

	ok = SendRequest<
		userssdl::V1_0::CCreateSuperuserGqlRequest,
		userssdl::V1_0::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	// Empty email for su
	QVERIFY(ok);
	QVERIFY(response.Success.has_value() && !*response.Success);
	QVERIFY(response.Message.has_value() && !response.Message->isEmpty());

	arguments.input.Mail = "su@mail.ru";
	arguments.input.Password = "";

	ok = SendRequest<
		userssdl::V1_0::CCreateSuperuserGqlRequest,
		userssdl::V1_0::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	// Empty password for su
	QVERIFY(ok);
	QVERIFY(response.Success.has_value() && !*response.Success);
	QVERIFY(response.Message.has_value() && !response.Message->isEmpty());

	arguments.input.Name = userInfo.GetName();
	arguments.input.Mail = userInfo.GetMail();
	arguments.input.Password = "1";

	ok = SendRequest<
		userssdl::V1_0::CCreateSuperuserGqlRequest,
		userssdl::V1_0::CreateSuperuserRequestArguments,
		userssdl::CCreateSuperuserPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.Success.has_value() && *response.Success);

	QVERIFY(RemoveObjectFromTable(s_usersTableName, "su"));
}


void CUserControllerTest::CheckSuperuserTest()
{
	namespace userssdl = sdl::imtauth::Users;
	userssdl::V1_0::CheckSuperuserExistsRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;

	userInfo.SetObjectUuid("su");
	userInfo.SetId("su");
	userInfo.SetPasswordHash(m_hashCalculator.GenerateHash("su1"));
	userInfo.SetMail("su@mail.ru");
	userInfo.SetName("superuser");

	QVERIFY(AddUser(userInfo));

	userssdl::CCheckSuperuserPayload::V1_0 response;

	bool ok = SendRequest<
		userssdl::V1_0::CCheckSuperuserExistsGqlRequest,
		userssdl::V1_0::CheckSuperuserExistsRequestArguments,
		userssdl::CCheckSuperuserPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.Exists.has_value() && *response.Exists);
	QVERIFY(response.Message.has_value() && response.Message->isEmpty());
	QVERIFY(response.ErrorType.has_value() && response.ErrorType->isEmpty());

	imtauth::CIdentifiableUserInfo userInfo2;
	QVERIFY(GetObjectFromTable(s_usersTableName, "su", userInfo2));
	QVERIFY(userInfo.GetMail() == userInfo2.GetMail());
	QVERIFY(userInfo.GetName() == userInfo2.GetName());

	QVERIFY(RemoveObjectFromTable(s_usersTableName, "su"));

	ok = SendRequest<
		userssdl::V1_0::CCheckSuperuserExistsGqlRequest,
		userssdl::V1_0::CheckSuperuserExistsRequestArguments,
		userssdl::CCheckSuperuserPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.Exists.has_value() && !*response.Exists);
	QVERIFY(response.ErrorType.has_value() && !response.ErrorType->isEmpty());

	imtauth::CIdentifiableUserInfo userInfo3;
	QVERIFY(!GetObjectFromTable(s_usersTableName, "su", userInfo3));
}


void CUserControllerTest::CheckSuperuserFailedTest()
{
}


I_ADD_TEST(CUserControllerTest);
