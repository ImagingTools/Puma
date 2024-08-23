#include "CAuthorizationControllerTest.h"


// ACF includes
#include <istd/TDelPtr.h>


// public methods

// reimplemented (imtgql::CGqlHandlerTest)

imtbase::CTreeItemModel* CAuthorizationControllerTest::CreateExpectedModel() const
{
	istd::TDelPtr<imtbase::CTreeItemModel> expectedModelPtr;
	expectedModelPtr.SetPtr(new imtbase::CTreeItemModel);

	imtbase::CTreeItemModel* dataModelPtr = expectedModelPtr->AddTreeModel("data");
	Q_ASSERT(dataModelPtr != nullptr);

	imtbase::CTreeItemModel* userTokenModelPtr = dataModelPtr->AddTreeModel("UserToken");
	Q_ASSERT(userTokenModelPtr != nullptr);

	userTokenModelPtr->SetData("Token", "");
	userTokenModelPtr->SetData("Login", "");
	userTokenModelPtr->SetData("UserId", "");
	userTokenModelPtr->SetData("PasswordHash", "");
	userTokenModelPtr->SetData("Permissions", "");

	return expectedModelPtr.PopPtr();
}


imtgql::CGqlRequest* CAuthorizationControllerTest::CreateGqlRequest() const
{
	istd::TDelPtr<imtgql::CGqlRequest> gqlRequestPtr;
	gqlRequestPtr.SetPtr(new imtgql::CGqlRequest);

	gqlRequestPtr->SetRequestType(imtgql::CGqlRequest::RT_QUERY);
	gqlRequestPtr->SetCommandId("UserToken");

	istd::TDelPtr<imtgql::CGqlObject> inputObjectPtr;
	inputObjectPtr.SetPtr(new imtgql::CGqlObject());

	inputObjectPtr->InsertField("Login", "test");
	inputObjectPtr->InsertField("Password", "1");

	gqlRequestPtr->AddParam("input", *inputObjectPtr.PopPtr());

	return gqlRequestPtr.PopPtr();
}


bool CAuthorizationControllerTest::VerifyResponse(const imtbase::CTreeItemModel& actualModel, const imtbase::CTreeItemModel& /*expectedModel*/) const
{
	if (!actualModel.ContainsKey("data")){
		return false;
	}

	if (actualModel.ContainsKey("errors")){
		return false;
	}

	imtbase::CTreeItemModel* dataModelPtr = actualModel.GetTreeItemModel("data");
	if (dataModelPtr == nullptr){
		return false;
	}

	if (!dataModelPtr->ContainsKey("Token")){
		return false;
	}

	QUuid uuid = dataModelPtr->GetData("Token").toUuid();
	if (uuid.isNull()){
		return false;
	}

	if (!dataModelPtr->ContainsKey("Login")){
		return false;
	}

	if (!dataModelPtr->ContainsKey("UserId")){
		return false;
	}

	if (!dataModelPtr->ContainsKey("PasswordHash")){
		return false;
	}

	if (!dataModelPtr->ContainsKey("Permissions")){
		return false;
	}

	return true;
}


I_ADD_TEST(CAuthorizationControllerTest);


