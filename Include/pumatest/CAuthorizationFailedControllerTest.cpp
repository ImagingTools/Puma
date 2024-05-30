#include "CAuthorizationFailedControllerTest.h"


// ACF includes
#include <istd/TDelPtr.h>


// public methods

// reimplemented (imtgql::CGqlHandlerTest)

imtbase::CTreeItemModel* CAuthorizationFailedControllerTest::CreateExpectedModel() const
{
	istd::TDelPtr<imtbase::CTreeItemModel> expectedModelPtr;
	expectedModelPtr.SetPtr(new imtbase::CTreeItemModel);

	imtbase::CTreeItemModel* dataModelPtr = expectedModelPtr->AddTreeModel("data");
	Q_ASSERT(dataModelPtr != nullptr);

	imtbase::CTreeItemModel* errorsModelPtr = expectedModelPtr->AddTreeModel("errors");
	Q_ASSERT(errorsModelPtr != nullptr);

	errorsModelPtr->SetData("message", QString("Invalid login or password. Login: '%1'.").arg("test"));

	return expectedModelPtr.PopPtr();
}


imtgql::CGqlRequest* CAuthorizationFailedControllerTest::CreateGqlRequest() const
{
	istd::TDelPtr<imtgql::CGqlRequest> gqlRequestPtr;
	gqlRequestPtr.SetPtr(new imtgql::CGqlRequest);

	gqlRequestPtr->SetRequestType(imtgql::CGqlRequest::RT_QUERY);
	gqlRequestPtr->SetCommandId("UserToken");

	istd::TDelPtr<imtgql::CGqlObject> inputObjectPtr;
	inputObjectPtr.SetPtr(new imtgql::CGqlObject("input"));

	inputObjectPtr->InsertField("Login", "test");
	inputObjectPtr->InsertField("Password", "test");

	gqlRequestPtr->AddParam(*inputObjectPtr.PopPtr());

	return gqlRequestPtr.PopPtr();
}


bool CAuthorizationFailedControllerTest::VerifyResponse(const imtbase::CTreeItemModel& actualModel, const imtbase::CTreeItemModel& /*expectedModel*/) const
{
	if (!actualModel.ContainsKey("errors")){
		return false;
	}

	imtbase::CTreeItemModel* errorsModelPtr = actualModel.GetTreeItemModel("errors");
	if (errorsModelPtr == nullptr){
		return false;
	}

	if (!errorsModelPtr->ContainsKey("message")){
		return false;
	}

	return true;
}


I_ADD_TEST(CAuthorizationFailedControllerTest);


