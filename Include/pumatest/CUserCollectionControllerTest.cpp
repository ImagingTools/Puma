#include "CUserCollectionControllerTest.h"


// ACF includes
#include <itest/CStandardTestExecutor.h>


// private slots

void CUserCollectionControllerTest::AddUserTest()
{
	namespace userssdl = sdl::imtauth::Users;

	userssdl::UserAddRequestArguments arguments;

	imtauth::CIdentifiableUserInfo userInfo;
	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfo.SetObjectUuid(uuid);
	userInfo.SetId("Ivanov");

	QByteArray passwordHash = m_hashCalculator.GenerateHash("Test12345");
	userInfo.SetPasswordHash(passwordHash);
	userInfo.SetMail("ivanov@mail.ru");
	userInfo.SetName("Ivanov");

	istd::TDelPtr<sdl::imtauth::Users::CUserData::V1_0> userRepresentation = CreateUserDataFromUserInfo(userInfo, "Test");

	arguments.input.Version_1_0->Id = uuid;
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();
	arguments.input.Version_1_0->ProductId = "Test";

	sdl::imtbase::ImtCollection::CAddedNotificationPayload::V1_0 response;

	bool ok = SendRequest<
		sdl::imtauth::Users::CUserAddGqlRequest,
		sdl::imtauth::Users::UserAddRequestArguments,
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

	userssdl::UserAddRequestArguments arguments;

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userInfo;
	userInfo.SetCastedOrRemove(CreateUserInfo("Test", "12345", "Ivanov", "ivanov@mail.ru"));

	istd::TDelPtr<sdl::imtauth::Users::CUserData::V1_0> userRepresentation = CreateUserDataFromUserInfo(*userInfo.GetPtr(), "Test");
	arguments.input.Version_1_0->Id = userInfo->GetObjectUuid();
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();
	arguments.input.Version_1_0->ProductId = "Test";

	sdl::imtbase::ImtCollection::CAddedNotificationPayload::V1_0 response;

	// Username already exists
	bool ok = SendRequest<
		sdl::imtauth::Users::CUserAddGqlRequest,
		sdl::imtauth::Users::UserAddRequestArguments,
		sdl::imtbase::ImtCollection::CAddedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	imtauth::CIdentifiableUserInfo userInfo2;
	QVERIFY(!GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));

	ok = SendRequest<
		sdl::imtauth::Users::CUserAddGqlRequest,
		sdl::imtauth::Users::UserAddRequestArguments,
		sdl::imtbase::ImtCollection::CAddedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	imtauth::CIdentifiableUserInfo userInfo3;
	QVERIFY(!GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo3));
}


void CUserCollectionControllerTest::RemoveUserTest()
{
	namespace userssdl = sdl::imtauth::Users;

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userInfo;
	userInfo.SetCastedOrRemove(CreateUserInfo("RemovedUser", "12345", "Ivanov", "ivanov@mail.ru"));

	QVERIFY(AddUser(*userInfo.GetPtr()));

	userssdl::UsersRemoveRequestArguments arguments;
	arguments.input.Version_1_0->Id = userInfo->GetObjectUuid();

	userssdl::CRemoveUserPayload::V1_0 response;
	bool ok = SendRequest<
		userssdl::CUsersRemoveGqlRequest,
		userssdl::UsersRemoveRequestArguments,
		userssdl::CRemoveUserPayload>(arguments, response);

	// QVERIFY(ok);

	imtauth::CIdentifiableUserInfo userInfo2;
	// QVERIFY(!GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));
}


void CUserCollectionControllerTest::RemoveUserFailedTest()
{
	namespace userssdl = sdl::imtauth::Users;

	userssdl::UsersRemoveRequestArguments arguments;
	arguments.input.Version_1_0->Id = "UnexistsUser";

	userssdl::CRemoveUserPayload::V1_0 response;
	bool ok = SendRequest<
		userssdl::CUsersRemoveGqlRequest,
		userssdl::UsersRemoveRequestArguments,
		userssdl::CRemoveUserPayload>(arguments, response);

	QVERIFY(!ok);
}


void CUserCollectionControllerTest::UpdateUserTest()
{
	namespace userssdl = sdl::imtauth::Users;

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userInfo;
	userInfo.SetCastedOrRemove(CreateUserInfo("RemovedUser", "12345", "Ivanov", "ivanov@mail.ru"));

	QVERIFY(AddUser(*userInfo.GetPtr()));

	userssdl::UserUpdateRequestArguments arguments;
	arguments.input.Version_1_0->Id = userInfo->GetObjectUuid();
	arguments.input.Version_1_0->ProductId = "Test";

	istd::TDelPtr<sdl::imtauth::Users::CUserData::V1_0> userRepresentation = CreateUserDataFromUserInfo(*userInfo, "Test");
	userRepresentation->Name = "NewName";
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	sdl::imtbase::ImtCollection::CUpdatedNotificationPayload::V1_0 response;
	bool ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.Id.has_value() && response.Id == userInfo->GetObjectUuid());

	imtauth::CIdentifiableUserInfo userInfo2;
	QVERIFY(GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));

	QVERIFY(userInfo2.GetObjectUuid() == userInfo->GetObjectUuid());
	QVERIFY(userInfo2.GetMail() == userInfo->GetMail());
	QVERIFY(userInfo2.GetRoles("Test") == userInfo->GetRoles("Test"));
	QVERIFY(userInfo2.GetGroups() == userInfo->GetGroups());
	QVERIFY(userInfo2.GetName() != userInfo->GetName());
	QVERIFY(userInfo2.GetName() == *userRepresentation->Name);

	userRepresentation->Name = "NewName2";
	userRepresentation->Email = "ivanov2@mail.ru";
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.Id.has_value() && response.Id == userInfo->GetObjectUuid());

	QVERIFY(GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));
	QVERIFY(userInfo2.GetObjectUuid() == userInfo->GetObjectUuid());
	QVERIFY(userInfo2.GetMail() != userInfo->GetMail());
	QVERIFY(userInfo2.GetMail() == *userRepresentation->Email);
	QVERIFY(userInfo2.GetName() != userInfo->GetName());
	QVERIFY(userInfo2.GetName() == *userRepresentation->Name);

	userRepresentation->Username = "RemovedUser2";
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.Id.has_value() && response.Id == userInfo->GetObjectUuid());

	QVERIFY(GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));
	QVERIFY(userInfo2.GetObjectUuid() == userInfo->GetObjectUuid());
	QVERIFY(userInfo2.GetId() != userInfo->GetId());
	QVERIFY(userInfo2.GetId() == *userRepresentation->Username);

	// Adding roles to user
	istd::TDelPtr<imtauth::CIdentifiableRoleInfo> roleInfo1;
	roleInfo1.SetCastedOrRemove(CreateRoleInfo("Developer", "Developer", "Test", QByteArrayList()));

	istd::TDelPtr<imtauth::CIdentifiableRoleInfo> roleInfo2;
	roleInfo2.SetCastedOrRemove(CreateRoleInfo("Tester", "Tester", "Test", QByteArrayList()));

	istd::TDelPtr<imtauth::CIdentifiableRoleInfo> roleInfo3;
	roleInfo2.SetCastedOrRemove(CreateRoleInfo("Tester2", "Tester2", "Test2", QByteArrayList()));

	QVERIFY(AddRole(*roleInfo1.GetPtr()));
	QVERIFY(AddRole(*roleInfo2.GetPtr()));

	QByteArrayList roles;
	roles << roleInfo1->GetObjectUuid();
	roles << roleInfo2->GetObjectUuid();

	userRepresentation->Roles = roles.join(';');

	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));
	QVERIFY(userInfo2.GetObjectUuid() == userInfo->GetObjectUuid());
	QVERIFY(userInfo2.GetId() != userInfo->GetId());
	QVERIFY(userInfo2.GetId() == *userRepresentation->Username);

	QByteArrayList userRoles = userInfo2.GetRoles("Test");
	QVERIFY(!userRoles.isEmpty());
	QVERIFY(userRoles.size() == 2);
	QVERIFY(userRoles.contains(roleInfo1->GetObjectUuid()));
	QVERIFY(userRoles.contains(roleInfo2->GetObjectUuid()));

	// Remove one role
	roles.removeAll(roleInfo1->GetObjectUuid());
	userRepresentation->Roles = roles.join(';');
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(ok);

	userRoles = userInfo2.GetRoles("Test");
	QVERIFY(!userRoles.isEmpty());
	QVERIFY(userRoles.size() == 1);
	QVERIFY(!userRoles.contains(roleInfo1->GetObjectUuid()));
	QVERIFY(userRoles.contains(roleInfo2->GetObjectUuid()));

	QVERIFY(RemoveObjectFromTable("Users", userInfo->GetObjectUuid()));
	QVERIFY(RemoveObjectFromTable("Roles", roleInfo1->GetObjectUuid()));
	QVERIFY(RemoveObjectFromTable("Roles", roleInfo2->GetObjectUuid()));
}


void CUserCollectionControllerTest::UpdateUserFailedTest()
{
	namespace userssdl = sdl::imtauth::Users;

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userInfo;
	userInfo.SetCastedOrRemove(CreateUserInfo("Ivanov", "12345", "Ivanov", "ivanov@mail.ru"));

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userInfo2;
	userInfo2.SetCastedOrRemove(CreateUserInfo("Sidorov", "12345", "Sidorov", "sidorov@gmail.ru"));

	QVERIFY(AddUser(*userInfo.GetPtr()));
	QVERIFY(AddUser(*userInfo2.GetPtr()));

	userssdl::UserUpdateRequestArguments arguments;
	arguments.input.Version_1_0->Id = userInfo->GetObjectUuid();
	arguments.input.Version_1_0->ProductId = "Test";

	// Empty name
	istd::TDelPtr<sdl::imtauth::Users::CUserData::V1_0> userRepresentation = CreateUserDataFromUserInfo(*userInfo, "Test");
	userRepresentation->Name = "";
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	sdl::imtbase::ImtCollection::CUpdatedNotificationPayload::V1_0 response;
	bool ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	// Empty username
	userRepresentation->Name = "Ivanov";
	userRepresentation->Username = "";
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	// Username alreasy exists
	userRepresentation->Name = "Ivanov";
	userRepresentation->Username = "Sidorov";
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	// Email empty
	userRepresentation->Name = "Ivanov";
	userRepresentation->Username = "Ivanov";
	userRepresentation->Email = "sidorov@gmail.ru";
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	// Email alreay exists
	userRepresentation->Name = "Ivanov";
	userRepresentation->Username = "Ivanov";
	userRepresentation->Email = "";
	arguments.input.Version_1_0->Item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	QVERIFY(RemoveObjectFromTable("Users", userInfo->GetObjectUuid()));
	QVERIFY(RemoveObjectFromTable("Users", userInfo2->GetObjectUuid()));
}


void CUserCollectionControllerTest::GetUserListTest()
{
	namespace userssdl = sdl::imtauth::Users;

	userssdl::UsersListRequestArguments arguments;

	sdl::imtbase::ImtCollection::CCollectionViewParams::V1_0 viewParams;

	viewParams.Count = -1;
	viewParams.Offset = 0;
	arguments.input.Version_1_0->ProductId = "Test";
	arguments.input.Version_1_0->viewParams = viewParams;

	userssdl::CUsersListPayload::V1_0 response;
	bool ok = SendRequest<
		userssdl::CUsersListGqlRequest,
		userssdl::UsersListRequestArguments,
		userssdl::CUsersListPayload>(arguments, response);

	QVERIFY(ok);

	QVERIFY(response.notification.has_value() && response.notification->PagesCount == 1);
	QVERIFY(response.notification.has_value() && response.notification->TotalCount == 5);
	QVERIFY(response.items.has_value());
	QVERIFY(response.items->size() == 5);

	QList<userssdl::CUserItem::V1_0> items = *response.items;

	QByteArrayList ids;
	QStringList names;
	for (const userssdl::CUserItem::V1_0& item : items){
		ids << *item.Id;
		names << *item.Name;
	}

	QVERIFY(names.contains("Test1") &&
			names.contains("Test2") &&
			names.contains("Test3") &&
			names.contains("Test4") &&
			names.contains("Test5"));

	QVERIFY(ids.contains("Test1") &&
			ids.contains("Test2") &&
			ids.contains("Test3") &&
			ids.contains("Test4") &&
			ids.contains("Test5"));
}

// protected methods

void CUserCollectionControllerTest::SetData() const
{
	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr1;
	userPtr1.SetCastedOrRemove(CreateUserInfo("Test1", "Test1", "Test1", "Test1@mail.ru"));
	AddUser(*userPtr1);

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr2;
	userPtr2.SetCastedOrRemove(CreateUserInfo("Test2", "Test2", "Test2", "Test2@mail.ru"));
	AddUser(*userPtr2);

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr3;
	userPtr3.SetCastedOrRemove(CreateUserInfo("Test3", "Test3", "Test3", "Test3@mail.ru"));
	AddUser(*userPtr3);

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr4;
	userPtr4.SetCastedOrRemove(CreateUserInfo("Test4", "Test4", "Test4", "Test4@mail.ru"));
	AddUser(*userPtr4);

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userPtr5;
	userPtr5.SetCastedOrRemove(CreateUserInfo("Test5", "Test5", "Test5", "Test5@mail.ru"));
	AddUser(*userPtr5);
}


I_ADD_TEST(CUserCollectionControllerTest);

