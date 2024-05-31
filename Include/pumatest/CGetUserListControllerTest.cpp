#include "CGetUserListControllerTest.h"


// ACF includes
#include <istd/TDelPtr.h>


// public methods

// reimplemented (imtgql::CGqlHandlerTest)

imtbase::CTreeItemModel* CGetUserListControllerTest::CreateExpectedModel() const
{
	istd::TDelPtr<imtbase::CTreeItemModel> expectedModelPtr;
	expectedModelPtr.SetPtr(new imtbase::CTreeItemModel);

	return expectedModelPtr.PopPtr();
}


imtgql::CGqlRequest* CGetUserListControllerTest::CreateGqlRequest() const
{
	istd::TDelPtr<imtgql::CGqlRequest> gqlRequestPtr;
	gqlRequestPtr.SetPtr(new imtgql::CGqlRequest);

	gqlRequestPtr->SetRequestType(imtgql::CGqlRequest::RT_QUERY);
	gqlRequestPtr->SetCommandId("UsersList");

	istd::TDelPtr<imtgql::CGqlObject> inputObjectPtr;
	inputObjectPtr.SetPtr(new imtgql::CGqlObject("input"));

	imtgql::CGqlObject* viewParamObjectPtr = inputObjectPtr->CreateFieldObject("viewParams");
	viewParamObjectPtr->InsertField("Count", -1);
	viewParamObjectPtr->InsertField("Offset", 0);

	gqlRequestPtr->AddParam(*inputObjectPtr.PopPtr());

	istd::TDelPtr<imtgql::CGqlObject> fieldObjectPtr;
	fieldObjectPtr.SetPtr(new imtgql::CGqlObject("items"));
	fieldObjectPtr->InsertField("Id");
	fieldObjectPtr->InsertField("Name");
	gqlRequestPtr->AddField(*fieldObjectPtr.PopPtr());

	return gqlRequestPtr.PopPtr();
}


bool CGetUserListControllerTest::VerifyResponse(const imtbase::CTreeItemModel& actualModel, const imtbase::CTreeItemModel& /*expectedModel*/) const
{
	ipackage::CComponentAccessor* accessorPtr = GetComponentAccessor();
	if (accessorPtr != nullptr){
		imtdb::IDatabaseEngine* databaseEnginePtr = accessorPtr->GetComponentInterface<imtdb::IDatabaseEngine>();
		if (databaseEnginePtr != nullptr){
			QSqlQuery sqlQuery = databaseEnginePtr->ExecSqlQuery(QByteArray("SELECT * FROM \"Users\";"));
			if (sqlQuery.size() != 10){
				return false;
			}
		}

		if (actualModel.ContainsKey("errors")){
			return false;
		}

		if (!actualModel.ContainsKey("data")){
			return false;
		}

		imtbase::CTreeItemModel* dataModelPtr = actualModel.GetTreeItemModel("data");
		if (!dataModelPtr->ContainsKey("items")){
			return false;
		}

		imtbase::CTreeItemModel* itemsModelPtr = dataModelPtr->GetTreeItemModel("items");
		if (itemsModelPtr == nullptr || itemsModelPtr->GetItemsCount() != 10){
			return false;
		}

		if (!dataModelPtr->ContainsKey("notification")){
			return false;
		}

		imtbase::CTreeItemModel* notificationModelPtr = dataModelPtr->GetTreeItemModel("notification");
		if (notificationModelPtr == nullptr){
			return false;
		}

		if (!notificationModelPtr->ContainsKey("PagesCount")){
			return false;
		}

		int pagesCount = notificationModelPtr->GetData("PagesCount").toInt();
		if (pagesCount != 1){
			return false;
		}

		if (!notificationModelPtr->ContainsKey("TotalCount")){
			return false;
		}

		int totalCount = notificationModelPtr->GetData("TotalCount").toInt();
		if (totalCount != 10){
			return false;
		}
	}

	return true;
}


I_ADD_TEST(CGetUserListControllerTest);


