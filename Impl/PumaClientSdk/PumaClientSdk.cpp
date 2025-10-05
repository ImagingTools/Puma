#include <PumaClientSdk/PumaClientSdk.h>


// ACF includes
#include <ibase/IApplicationInfo.h>
#include <iauth/ILogin.h>
#include <iauth/IRightsProvider.h>

// ImtCore includes
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


	bool Login(const QString& login, const QString& password)
	{
		iauth::ILogin* loginPtr = m_sdk.GetInterface<iauth::ILogin>("SimpleLoginWrap");
		if (loginPtr == nullptr){
			return false;
		}

		return loginPtr->Login(login, password);
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
		ibase::IApplicationInfo* applicationInfoPtr = m_sdk.GetInterface<ibase::IApplicationInfo>("ApplicationInfo");
		if (applicationInfoPtr != nullptr){
			// ???
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


bool CAuthorizationController::Login(const QString& login, const QString& password)
{
	if (m_implPtr != nullptr){
		return m_implPtr->Login(login, password);
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


