// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#include <AuthClientSdk/CLoginViewWidget.h>


// Qt includes
#include <QtWidgets/QVBoxLayout>

// ACF includes
#include <iqtgui/IGuiObject.h>
#include <iauth/ILogin.h>

// ImtCore includes
#include <imtbase/Init.h>
#include <imtbase/IApplicationInfoController.h>
#include <imtcom/IServerConnectionInterface.h>
#include <imtauth/IUserPermissionsController.h>
#include <imtauth/IAccessTokenController.h>

// Local includes
#include <GeneratedFiles/AuthClientSdk/CLoginWidget.h>


namespace AuthClientSdk
{


class CLoginViewWidgetImpl
{
public:
	CLoginViewWidgetImpl()
	{
		m_sdk.EnsureAutoInitComponentsCreated();
	}


	QWidget* GetWidget(QWidget* parentPtr)
	{
		iqtgui::IGuiObject* guiObjectPtr = m_sdk.GetInterface<iqtgui::IGuiObject>();
		if (guiObjectPtr != nullptr){
			bool ok = guiObjectPtr->CreateGui(parentPtr);
			if (ok){
				return guiObjectPtr->GetWidget();
			}
		}

		return nullptr;
	}


	bool SetConnectionParam(const QString& host, int httpPort, int wsPort)
	{
		imtcom::IServerConnectionInterface* serverConnectionParamPtr = m_sdk.GetInterface<imtcom::IServerConnectionInterface>();
		if (serverConnectionParamPtr != nullptr){
			serverConnectionParamPtr->SetHost(host);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_HTTP, httpPort);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_WEBSOCKET, wsPort);

			return true;
		}

		return false;
	}


	bool SetLoginParam(Login param)
	{
		iauth::ILogin* loginPtr = m_sdk.GetInterface<iauth::ILogin>();
		if (loginPtr == nullptr){
			return false;
		}

		loginPtr->Login(param.userName, "");

		imtauth::IUserPermissionsController* permissionsController = m_sdk.GetInterface<imtauth::IUserPermissionsController>();
		if (permissionsController == nullptr){
			return false;
		}

		permissionsController->SetPermissions("", param.permissions);

		imtauth::IAccessTokenController* accessTokenControllerPtr = m_sdk.GetInterface<imtauth::IAccessTokenController>();
		if (accessTokenControllerPtr == nullptr){
			return false;
		}

		accessTokenControllerPtr->SetToken("", param.accessToken);

		ibase::IApplicationInfo* applicationInfoPtr = m_sdk.GetInterface<ibase::IApplicationInfo>();
		if (applicationInfoPtr == nullptr){
			return false;
		}

		imtbase::IApplicationInfoController* applicationInfoControllerPtr = m_sdk.GetInterface<imtbase::IApplicationInfoController>();
		if (applicationInfoControllerPtr == nullptr){
			return false;
		}

		applicationInfoControllerPtr->SetApplicationAttribute(imtbase::IApplicationInfoController::ApplicationAttribute::AA_APPLICATION_ID, param.productId);

		return true;
	}

private:
	mutable CLoginWidget m_sdk;
};


// public methods

CLoginViewWidget::CLoginViewWidget()
	:m_implPtr(nullptr)
{
	DefaultImtCoreQmlInitializer initializer;
	initializer.Init();

	m_implPtr = new CLoginViewWidgetImpl;
	QWidget* widgetPtr = m_implPtr->GetWidget(this);
	if (widgetPtr != nullptr){
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(widgetPtr);
		setLayout(layout);
	}
}


CLoginViewWidget::~CLoginViewWidget()
{
	if (m_implPtr != nullptr){
		delete m_implPtr;
	}
}


bool CLoginViewWidget::SetConnectionParam(const QString& host, int httpPort, int wsPort) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetConnectionParam(host, httpPort, wsPort);
	}

	return false;
}


bool CLoginViewWidget::SetLoginParam(Login param) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetLoginParam(param);
	}

	return false;
}


} // namespace AuthClientSdk


