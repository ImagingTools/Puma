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

	arguments.input.Version_1_0->id = uuid;
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();
	arguments.input.Version_1_0->productId = "Test";
	
	sdl::imtbase::ImtCollection::CAddedNotificationPayload::V1_0 response;

	bool ok = SendRequest<
		sdl::imtauth::Users::CUserAddGqlRequest,
		sdl::imtauth::Users::UserAddRequestArguments,
		sdl::imtbase::ImtCollection::CAddedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.id == uuid);

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
	arguments.input.Version_1_0->id = userInfo->GetObjectUuid();
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();
	arguments.input.Version_1_0->productId = "Test";

	sdl::imtbase::ImtCollection::CAddedNotificationPayload::V1_0 response;

	// username already exists
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
}


void CUserCollectionControllerTest::RemoveUserFailedTest()
{
	namespace userssdl = sdl::imtauth::Users;
}


void CUserCollectionControllerTest::UpdateUserTest()
{
	namespace userssdl = sdl::imtauth::Users;

	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userInfo;
	userInfo.SetCastedOrRemove(CreateUserInfo("RemovedUser", "12345", "Ivanov", "ivanov@mail.ru"));

	QVERIFY(AddUser(*userInfo.GetPtr()));

	userssdl::UserUpdateRequestArguments arguments;
	arguments.input.Version_1_0->id = userInfo->GetObjectUuid();
	arguments.input.Version_1_0->productId = "Test";

	istd::TDelPtr<sdl::imtauth::Users::CUserData::V1_0> userRepresentation = CreateUserDataFromUserInfo(*userInfo, "Test");
	userRepresentation->name = "NewName";
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

	sdl::imtbase::ImtCollection::CUpdatedNotificationPayload::V1_0 response;
	bool ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.id.has_value() && response.id == userInfo->GetObjectUuid());

	imtauth::CIdentifiableUserInfo userInfo2;
	QVERIFY(GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));

	QVERIFY(userInfo2.GetObjectUuid() == userInfo->GetObjectUuid());
	QVERIFY(userInfo2.GetMail() == userInfo->GetMail());
	QVERIFY(userInfo2.GetRoles("Test") == userInfo->GetRoles("Test"));
	QVERIFY(userInfo2.GetGroups() == userInfo->GetGroups());
	QVERIFY(userInfo2.GetName() != userInfo->GetName());
	QVERIFY(userInfo2.GetName() == *userRepresentation->name);

	userRepresentation->name = "NewName2";
	userRepresentation->email = "ivanov2@mail.ru";
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.id.has_value() && response.id == userInfo->GetObjectUuid());

	QVERIFY(GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));
	QVERIFY(userInfo2.GetObjectUuid() == userInfo->GetObjectUuid());
	QVERIFY(userInfo2.GetMail() != userInfo->GetMail());
	QVERIFY(userInfo2.GetMail() == *userRepresentation->email);
	QVERIFY(userInfo2.GetName() != userInfo->GetName());
	QVERIFY(userInfo2.GetName() == *userRepresentation->name);

	userRepresentation->username = "RemovedUser2";
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(response.id.has_value() && response.id == userInfo->GetObjectUuid());

	QVERIFY(GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));
	QVERIFY(userInfo2.GetObjectUuid() == userInfo->GetObjectUuid());
	QVERIFY(userInfo2.GetId() != userInfo->GetId());
	QVERIFY(userInfo2.GetId() == *userRepresentation->username);

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

	userRepresentation->roles.Emplace();
	userRepresentation->roles->FromList(roles);

	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(ok);
	QVERIFY(GetObjectFromTable("Users", userInfo->GetObjectUuid(), userInfo2));
	QVERIFY(userInfo2.GetObjectUuid() == userInfo->GetObjectUuid());
	QVERIFY(userInfo2.GetId() != userInfo->GetId());
	QVERIFY(userInfo2.GetId() == *userRepresentation->username);

	QByteArrayList userRoles = userInfo2.GetRoles("Test");
	QVERIFY(!userRoles.isEmpty());
	QVERIFY(userRoles.size() == 2);
	QVERIFY(userRoles.contains(roleInfo1->GetObjectUuid()));
	QVERIFY(userRoles.contains(roleInfo2->GetObjectUuid()));

	// Remove one role
	roles.removeAll(roleInfo1->GetObjectUuid());
	userRepresentation->roles.Emplace();
	userRepresentation->roles->FromList(roles);
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

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
	arguments.input.Version_1_0->id = userInfo->GetObjectUuid();
	arguments.input.Version_1_0->productId = "Test";

	// Empty name
	istd::TDelPtr<sdl::imtauth::Users::CUserData::V1_0> userRepresentation = CreateUserDataFromUserInfo(*userInfo, "Test");
	userRepresentation->name = "";
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

	sdl::imtbase::ImtCollection::CUpdatedNotificationPayload::V1_0 response;
	bool ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	// Empty username
	userRepresentation->name = "Ivanov";
	userRepresentation->username = "";
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	// username alreasy exists
	userRepresentation->name = "Ivanov";
	userRepresentation->username = "Sidorov";
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	// email empty
	userRepresentation->name = "Ivanov";
	userRepresentation->username = "Ivanov";
	userRepresentation->email = "sidorov@gmail.ru";
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

	ok = SendRequest<
		userssdl::CUserUpdateGqlRequest,
		userssdl::UserUpdateRequestArguments,
		sdl::imtbase::ImtCollection::CUpdatedNotificationPayload>(arguments, response);

	QVERIFY(!ok);

	// email alreay exists
	userRepresentation->name = "Ivanov";
	userRepresentation->username = "Ivanov";
	userRepresentation->email = "";
	arguments.input.Version_1_0->item = *userRepresentation.GetPtr();

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

	viewParams.count = -1;
	viewParams.offset = 0;
	arguments.input.Version_1_0->productId = "Test";
	arguments.input.Version_1_0->viewParams = viewParams;

	userssdl::CUsersListPayload::V1_0 response;
	bool ok = SendRequest<
		userssdl::CUsersListGqlRequest,
		userssdl::UsersListRequestArguments,
		userssdl::CUsersListPayload>(arguments, response);

	QVERIFY(ok);

	QVERIFY(response.notification.has_value() && response.notification->pagesCount == 1);
	QVERIFY(response.notification.has_value() && response.notification->totalCount == 5);
	QVERIFY(response.items.has_value());
	QVERIFY(response.items->size() == 5);

	QList<istd::TSharedNullable<sdl::imtauth::Users::CUserItemData::V1_0>> items = *response.items;

	QByteArrayList ids;
	QStringList names;
	for (const istd::TSharedNullable<sdl::imtauth::Users::CUserItemData::V1_0>& item : items){
		if (item.HasValue()){
			ids << *item->id;
			names << *item->name;
		}
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


// I_ADD_TEST(CUserCollectionControllerTest);

