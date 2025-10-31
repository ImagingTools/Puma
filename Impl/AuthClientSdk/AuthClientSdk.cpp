#include <AuthClientSdk/AuthClientSdk.h>


// Qt includes
#include <QDebug>

// ACF includes
#include <ibase/IApplicationInfo.h>
#include <iauth/ILogin.h>
#include <iauth/IRightsProvider.h>

// ImtCore includes
#include <imtbase/IApplicationInfoController.h>
#include <imtauth/IAccessTokenProvider.h>
#include <imtauth/IUserPermissionsController.h>
#include <imtauth/ISuperuserController.h>
#include <imtauth/ISuperuserProvider.h>
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Authorization.h>

// Local includes
#include <GeneratedFiles/AuthClientSdk/CAuthClientSdk.h>


namespace AuthClientSdk
{


class CAuthorizationControllerImpl
{
public:
	CAuthorizationControllerImpl()
	{
	}

	bool Login(const QString& login, const QString& password, Login& out)
	{
		iauth::ILogin* loginPtr = m_sdk.GetInterface<iauth::ILogin>();
		if (loginPtr == nullptr) {
			qWarning() << "[Login] Failed: iauth::ILogin interface not found";
			return false;
		}

		bool ok = loginPtr->Login(login, password);
		if (!ok) {
			qWarning() << "[Login] Failed: iauth::ILogin::Login() returned false";
			return false;
		}

		iauth::CUser* userPtr = loginPtr->GetLoggedUser();
		if (userPtr == nullptr) {
			qWarning() << "[Login] Failed: GetLoggedUser() returned nullptr";
			return false;
		}

		imtauth::IAccessTokenProvider* tokenProviderPtr = m_sdk.GetInterface<imtauth::IAccessTokenProvider>();
		if (tokenProviderPtr == nullptr) {
			qWarning() << "[Login] Failed: imtauth::IAccessTokenProvider interface not found";
			return false;
		}

		ibase::IApplicationInfo* applicationInfoPtr = m_sdk.GetInterface<ibase::IApplicationInfo>();
		if (applicationInfoPtr == nullptr) {
			qWarning() << "[Login] Failed: ibase::IApplicationInfo interface not found";
			return false;
		}

		imtauth::IUserPermissionsController* userPermissionsControllerPtr = m_sdk.GetInterface<imtauth::IUserPermissionsController>();
		if (userPermissionsControllerPtr == nullptr) {
			qWarning() << "[Login] Failed: imtauth::IUserPermissionsController interface not found";
			return false;
		}

		out.productId = applicationInfoPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_APPLICATION_ID).toUtf8();
		out.userName = userPtr->GetUserName();
		out.accessToken = tokenProviderPtr->GetToken(QByteArray());
		out.permissions = userPermissionsControllerPtr->GetPermissions(QByteArray());

		return true;
	}

	bool Logout()
	{
		iauth::ILogin* loginPtr = m_sdk.GetInterface<iauth::ILogin>();
		if (loginPtr == nullptr) {
			qWarning() << "[Logout] Failed: iauth::ILogin interface not found";
			return false;
		}

		return loginPtr->Logout();
	}

	bool HasPermission(const QByteArray& permissionId)
	{
		iauth::IRightsProvider* rightsProviderPtr = m_sdk.GetInterface<iauth::IRightsProvider>();
		if (rightsProviderPtr == nullptr) {
			qWarning() << "[HasPermission] Failed: iauth::IRightsProvider interface not found";
			return false;
		}

		return rightsProviderPtr->HasRight(permissionId);
	}

	QByteArray GetToken() const
	{
		imtauth::IAccessTokenProvider* accessTokenProviderPtr = m_sdk.GetInterface<imtauth::IAccessTokenProvider>();
		if (accessTokenProviderPtr != nullptr) {
			QByteArray userId;
			QByteArray token = accessTokenProviderPtr->GetToken(userId);
			return token;
		}

		return QByteArray();
	}

	void SetProductId(const QByteArray& productId)
	{
		imtbase::IApplicationInfoController* applicationInfoControllerPtr = m_sdk.GetInterface<imtbase::IApplicationInfoController>();
		if (applicationInfoControllerPtr != nullptr) {
			applicationInfoControllerPtr->SetApplicationAttribute(
						imtbase::IApplicationInfoController::ApplicationAttribute::AA_APPLICATION_ID,
						productId);
		}
		else{
			qWarning() << "[SetProductId] Failed: IApplicationInfoController not found";
		}
	}

	SuperuserStatus SuperuserExists(QString& errorMessage)
	{
		imtauth::ISuperuserProvider* superuserProviderPtr = m_sdk.GetInterface<imtauth::ISuperuserProvider>();
		if (superuserProviderPtr != nullptr) {
			imtauth::ISuperuserProvider::ExistsStatus status = superuserProviderPtr->SuperuserExists(errorMessage);
			switch (status){
			case imtauth::ISuperuserProvider::ES_EXISTS:
				return Exists;
			case imtauth::ISuperuserProvider::ES_NOT_EXISTS:
				return NotExists;
			case imtauth::ISuperuserProvider::ES_UNKNOWN:
				return Unknown;
			}
		}

		return Unknown;
	}

	bool CreateSuperuser(const QByteArray& password)
	{
		imtauth::ISuperuserController* superuserControllerPtr = m_sdk.GetInterface<imtauth::ISuperuserController>();
		if (superuserControllerPtr != nullptr) {
			return superuserControllerPtr->SetSuperuserPassword(password);
		}

		return false;
	}

private:
	mutable CAuthClientSdk m_sdk;
};


// public methods

CAuthorizationController::CAuthorizationController()
	: m_implPtr(nullptr)
{
	m_implPtr = new CAuthorizationControllerImpl;
}


CAuthorizationController::~CAuthorizationController()
{
	delete m_implPtr;
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


SuperuserStatus CAuthorizationController::SuperuserExists(QString& errorMessage)
{
	if (m_implPtr != nullptr){
		return m_implPtr->SuperuserExists(errorMessage);
	}

	return SuperuserStatus::Unknown;
}


bool CAuthorizationController::CreateSuperuser(const QByteArray& password)
{
	if (m_implPtr != nullptr){
		return m_implPtr->CreateSuperuser(password);
	}

	return false;
}


} // namespace AuthClientSdk
