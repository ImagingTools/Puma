#include "CUserCollectionControllerTest.h"


// ACF includes
#include <istd/TDelPtr.h>
#include <itest/CStandardTestExecutor.h>
#include <iser/CJsonMemWriteArchive.h>
#include <iser/CJsonMemReadArchive.h>

// ImtCore includes
#include <imtauth/CUserInfo.h>
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Users.h>


// private slots


void CUserCollectionControllerTest::initTestCase()
{
	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr1;
	userPtr1.SetCastedOrRemove(CreateUserInfo("Test1", "Test1", "Test1", "Test1@mail.ru"));
	AddUser(*userPtr1);

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr2;
	userPtr1.SetCastedOrRemove(CreateUserInfo("Test2", "Test2", "Test2", "Test2@mail.ru"));
	AddUser(*userPtr2);

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr3;
	userPtr1.SetCastedOrRemove(CreateUserInfo("Test3", "Test3", "Test3", "Test3@mail.ru"));
	AddUser(*userPtr3);

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr4;
	userPtr1.SetCastedOrRemove(CreateUserInfo("Test4", "Test4", "Test4", "Test4@mail.ru"));
	AddUser(*userPtr4);

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr5;
	userPtr1.SetCastedOrRemove(CreateUserInfo("Test5", "Test5", "Test5", "Test5@mail.ru"));
	AddUser(*userPtr5);
}


void CUserCollectionControllerTest::AddUserTest()
{
	namespace userssdl = sdl::imtauth::Users;

	userssdl::V1_0::UserAddRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;
	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Ivanov");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test12345");
	userInfo.SetPasswordHash(passwordHash);
	userInfo.SetMail("ivanov@mail.ru");
	userInfo.SetName("Ivanov");

	QVERIFY(AddUser(userInfo));

	istd::TDelPtr<sdl::imtauth::Users::CUserData::V1_0> userRepresentation = CreateUserDataFromUserInfo(userInfo, "Test");

	arguments.input.Id = uuid;
	arguments.input.Item = *userRepresentation.GetPtr();
	arguments.input.ProductId = "Test";

	sdl::imtbase::ImtCollection::CAddedNotificationPayload::V1_0 response;

	bool ok = SendRequest<
		sdl::imtauth::Users::V1_0::CUserAddGqlRequest,
		sdl::imtauth::Users::V1_0::UserAddRequestArguments,
		sdl::imtbase::ImtCollection::CAddedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.Id == uuid);

	imtauth::CIdentifiableUserInfo userInfo2;
	QVERIFY(GetObjectFromTable("Users", uuid, userInfo2));
	QVERIFY(userInfo2.GetObjectUuid() == uuid);
}


void CUserCollectionControllerTest::AddUserFailedTest()
{
	namespace userssdl = sdl::imtauth::Users;

	userssdl::V1_0::UserAddRequestArguments arguments;

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userInfo;
	userInfo.SetCastedOrRemove(CreateUserInfo("Test", "12345", "Ivanov", "ivanov@mail.ru"));

	istd::TDelPtr<sdl::imtauth::Users::CUserData::V1_0> userRepresentation = CreateUserDataFromUserInfo(*userInfo.GetPtr(), "Test");
	arguments.input.Id = userInfo->GetObjectUuid();
	arguments.input.Item = *userRepresentation.GetPtr();
	arguments.input.ProductId = "Test";

	sdl::imtbase::ImtCollection::CAddedNotificationPayload::V1_0 response;

	// Username already exists
	bool ok = SendRequest<
		sdl::imtauth::Users::V1_0::CUserAddGqlRequest,
		sdl::imtauth::Users::V1_0::UserAddRequestArguments,
		sdl::imtbase::ImtCollection::CAddedNotificationPayload>(arguments, response);

	QVERIFY(!ok);
	QVERIFY(response.Id->isEmpty());

	imtauth::CIdentifiableUserInfo userInfo2;
	QVERIFY(!GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));

	ok = SendRequest<
		sdl::imtauth::Users::V1_0::CUserAddGqlRequest,
		sdl::imtauth::Users::V1_0::UserAddRequestArguments,
		sdl::imtbase::ImtCollection::CAddedNotificationPayload>(arguments, response);

	QVERIFY(!ok);
	QVERIFY(response.Id->isEmpty());

	imtauth::CIdentifiableUserInfo userInfo3;
	QVERIFY(!GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));
}


void CUserCollectionControllerTest::RemoveUserTest()
{

}


void CUserCollectionControllerTest::RemoveUserFailedTest()
{

}


void CUserCollectionControllerTest::UpdateUserTest()
{

}


void CUserCollectionControllerTest::UpdateUserFailedTest()
{

}


void CUserCollectionControllerTest::GetUserListTest()
{

}


void CUserCollectionControllerTest::cleanupTestCase()
{

}


I_ADD_TEST(CUserCollectionControllerTest);

