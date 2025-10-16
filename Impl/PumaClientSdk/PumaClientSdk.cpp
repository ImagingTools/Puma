#include <PumaClientSdk/PumaClientSdk.h>


// ACF includes
#include <ibase/IApplicationInfo.h>
#include <iauth/ILogin.h>
#include <iauth/IRightsProvider.h>

// ImtCore includes
#include <imtbase/IApplicationInfoController.h>
#include <imtauth/IAccessTokenProvider.h>
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Authorization.h>

// Local includes
#include <GeneratedFiles/PumaClientSdk/CPumaClientSdk.h>


namespace PumaClientSdk
{


class CAuthorizationControllerImpl
{
public:
	CAuthorizationControllerImpl()
	{
	}


	bool Login(const QString& login, const QString& password, Login& out)
	{
		iauth::ILogin* loginPtr = m_sdk.GetInterface<iauth::ILogin>("SimpleLoginWrap");
		if (loginPtr == nullptr){
			return false;
		}

		bool ok = loginPtr->Login(login, password);
		if (!ok){
			return false;
		}

		iauth::CUser* userPtr = loginPtr->GetLoggedUser();
		if (userPtr == nullptr){
			return false;
		}

		imtauth::IAccessTokenProvider* tokenProviderPtr = m_sdk.GetInterface<imtauth::IAccessTokenProvider>("SimpleLoginWrap");
		if (tokenProviderPtr == nullptr){
			return false;
		}

		
		ibase::IApplicationInfo* applicationInfoPtr = m_sdk.GetInterface<ibase::IApplicationInfo>("ApplicationInfo");
		if (applicationInfoPtr == nullptr){
			return false;
		}

		out.productId = applicationInfoPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_APPLICATION_ID).toUtf8();
		out.userName = userPtr->GetUserName();
		out.accessToken = tokenProviderPtr->GetToken(QByteArray());

		return true;
	}


	bool Logout()
	{
		iauth::ILogin* loginPtr = m_sdk.GetInterface<iauth::ILogin>("SimpleLoginWrap");
		if (loginPtr == nullptr){
			return false;
		}

		return loginPtr->Logout();
	}


	bool HasPermission(const QByteArray& permissionId)
	{
		iauth::IRightsProvider* rightsProviderPtr = m_sdk.GetInterface<iauth::IRightsProvider>("SimpleLoginWrap");
		if (rightsProviderPtr == nullptr){
			return false;
		}

		return rightsProviderPtr->HasRight(permissionId);
	}


	QByteArray GetToken() const
	{
		imtauth::IAccessTokenProvider* accessTokenProviderPtr = m_sdk.GetInterface<imtauth::IAccessTokenProvider>("SimpleLoginWrap");
		if (accessTokenProviderPtr != nullptr){
			QByteArray userId;
			return accessTokenProviderPtr->GetToken(userId);
		}

		return QByteArray();
	}


	void SetProductId(const QByteArray& productId)
	{
		imtbase::IApplicationInfoController* applicationInfoControllerPtr = m_sdk.GetInterface<imtbase::IApplicationInfoController>("ApplicationInfoController");
		if (applicationInfoControllerPtr != nullptr){
			applicationInfoControllerPtr->SetApplicationAttribute(imtbase::IApplicationInfoController::ApplicationAttribute::AA_APPLICATION_ID, productId);
		}
	}

private:
	mutable CPumaClientSdk m_sdk;
};


// public methods

CAuthorizationController::CAuthorizationController()
	:m_implPtr(nullptr)
{
	m_implPtr = new CAuthorizationControllerImpl;
}


CAuthorizationController::~CAuthorizationController()
{
	if (m_implPtr != nullptr){
		delete m_implPtr;
	}
}


bool CAuthorizationController::Login(const QString& login, const QString& password, struct Login& out)
{
	if (m_implPtr != nullptr){
		return m_implPtr->Login(login, password, out);
	}

	return false;
}


bool CAuthorizationController::Logout()
{
	if (m_implPtr != nullptr){
		return m_implPtr->Logout();
	}

	return false;
}


bool CAuthorizationController::HasPermission(const QByteArray& permissionId)
{
	if (m_implPtr != nullptr){
		return m_implPtr->HasPermission(permissionId);
	}

	return false;
}


QByteArray CAuthorizationController::GetToken() const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetToken();
	}

	return QByteArray();
}


void CAuthorizationController::SetProductId(const QByteArray& productId)
{
	if (m_implPtr != nullptr){
		m_implPtr->SetProductId(productId);
	}
}


} // namespace PumaClientSdk


