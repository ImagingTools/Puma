#include "CRemoveUserControllerTest.h"


// ACF includes
#include <istd/TDelPtr.h>


// public methods

// reimplemented (imtgql::CGqlHandlerTest)

imtbase::CTreeItemModel* CRemoveUserControllerTest::CreateExpectedModel() const
{
	istd::TDelPtr<imtbase::CTreeItemModel> expectedModelPtr;
	expectedModelPtr.SetPtr(new imtbase::CTreeItemModel);

	imtbase::CTreeItemModel* dataModelPtr = expectedModelPtr->AddTreeModel("data");
	Q_ASSERT(dataModelPtr != nullptr);

	imtbase::CTreeItemModel* removedModelPtr = dataModelPtr->AddTreeModel("removedNotification");
	Q_ASSERT(removedModelPtr != nullptr);

	removedModelPtr->SetData("Id", "Test");
	removedModelPtr->SetData("Name", "Test");

	return expectedModelPtr.PopPtr();
}


imtgql::CGqlRequest* CRemoveUserControllerTest::CreateGqlRequest() const
{
	istd::TDelPtr<imtgql::CGqlRequest> gqlRequestPtr;
	gqlRequestPtr.SetPtr(new imtgql::CGqlRequest);

	gqlRequestPtr->SetRequestType(imtgql::CGqlRequest::RT_MUTATION);
	gqlRequestPtr->SetCommandId("UsersRemove");

	istd::TDelPtr<imtgql::CGqlObject> inputObjectPtr;
	inputObjectPtr.SetPtr(new imtgql::CGqlObject("input"));
	inputObjectPtr->InsertField("Id", "Test");
	gqlRequestPtr->AddParam(*inputObjectPtr.PopPtr());

	istd::TDelPtr<imtgql::CGqlObject> fieldObjectPtr;
	fieldObjectPtr.SetPtr(new imtgql::CGqlObject("removedNotification"));
	fieldObjectPtr->InsertField("Id");
	gqlRequestPtr->AddField(*fieldObjectPtr.PopPtr());

	return gqlRequestPtr.PopPtr();
}


bool CRemoveUserControllerTest::VerifyResponse(const imtbase::CTreeItemModel& actualModel, const imtbase::CTreeItemModel& expectedModel) const
{
	ipackage::CComponentAccessor* accessorPtr = GetComponentAccessor();
	if (accessorPtr != nullptr){
		imtdb::IDatabaseEngine* databaseEnginePtr = accessorPtr->GetComponentInterface<imtdb::IDatabaseEngine>();
		if (databaseEnginePtr != nullptr){
			QSqlQuery sqlQuery = databaseEnginePtr->ExecSqlQuery(QByteArray("SELECT * FROM \"Users\";"));
			if (sqlQuery.size() != 0){
				return false;
			}
		}
	}

	return BaseClass::VerifyResponse(actualModel, expectedModel);
}


I_ADD_TEST(CRemoveUserControllerTest);


