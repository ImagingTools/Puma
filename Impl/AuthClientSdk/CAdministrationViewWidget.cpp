// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#include <AuthClientSdk/CAdministrationViewWidget.h>


// Qt includes
#include <QtWidgets/QVBoxLayout>

// ACF includes
#include <iqtgui/IGuiObject.h>
#include <istd/CChangeGroup.h>

// ImtCore includes
#include <imtcore/CImtCoreAuthInitializer.h>
#include <imtcore/CImtCoreBaseInitializer.h>
#include <imtcore/CImtCoreLocalizationInitializer.h>
#include <imtcore/CImtCoreStyleInitializer.h>
#include <imtbase/IApplicationInfoController.h>
#include <imtcom/IServerConnectionInterface.h>
#include <imtqml/IQuickObject.h>

// Local includes
#include <GeneratedFiles/AuthClientSdk/CAdministrationWidget.h>


static void InitializeAdministrationWidgetResources()
{
	ImtCoreInitLocalizationResources();
	ImtCoreInitBaseResources();

	ImtCoreInitStyleResources();
	ImtCoreInitAuthStyleResources();

	ImtCoreInitQmlApplicationCoreResources();
	ImtCoreInitQmlDocumentManagementResources();

	Q_INIT_RESOURCE(imtauthguiqml);
	Q_INIT_RESOURCE(imtauthGroupsSdl);
	Q_INIT_RESOURCE(imtauthRolesSdl);
	Q_INIT_RESOURCE(imtauthUsersSdl);
	Q_INIT_RESOURCE(imtauthAuthorizationSdl);
	Q_INIT_RESOURCE(imtauthPermissionsSdl);
	Q_INIT_RESOURCE(imtauthSessionsSdl);
	Q_INIT_RESOURCE(imtauthTenantMembershipsSdl);
	Q_INIT_RESOURCE(imtauthRoleCollectionDocumentServiceSdl);
	Q_INIT_RESOURCE(imtauthGroupCollectionDocumentServiceSdl);
	Q_INIT_RESOURCE(imtauthUserCollectionDocumentServiceSdl);

	InitializeImtCoreStyle();
}


namespace AuthClientSdk
{


class CAdministrationViewWidgetImpl
{
public:
	CAdministrationViewWidgetImpl()
	{
		m_sdk.EnsureAutoInitComponentsCreated();
	}


	QWidget* GetWidget(QWidget* parentPtr)
	{
		auto* guiObjectPtr = m_sdk.GetInterface<iqtgui::IGuiObject>();
		if (guiObjectPtr != nullptr){
			if (!guiObjectPtr->IsGuiCreated()){
				guiObjectPtr->CreateGui(parentPtr);
			}

			return guiObjectPtr->GetWidget();
		}

		return nullptr;
	}


	bool SetConnectionParam(const ServerConfig& config)
	{
		auto* serverConnectionParamPtr = m_sdk.GetInterface<imtcom::IServerConnectionInterface>();
		if (serverConnectionParamPtr != nullptr){
			istd::CChangeGroup changeGroup(serverConnectionParamPtr);

			serverConnectionParamPtr->SetHost(config.host);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_HTTP, config.httpPort);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_WEBSOCKET, config.wsPort);

			if (config.sslConfig.has_value()){
				serverConnectionParamPtr->SetConnectionFlags(imtcom::IServerConnectionInterface::CF_SECURE);
			}

			return true;
		}

		return false;
	}


	bool SetLoginParam(Login param)
	{
		auto* applicationInfoControllerPtr = m_sdk.GetInterface<imtbase::IApplicationInfoController>();
		if (applicationInfoControllerPtr == nullptr){
			return false;
		}

		applicationInfoControllerPtr->SetApplicationAttribute(imtbase::IApplicationInfoController::ApplicationAttribute::AA_APPLICATION_ID, param.productId);

		auto* quickObjectPtr = m_sdk.GetInterface<imtqml::IQuickObject>();
		if (quickObjectPtr == nullptr){
			return false;
		}

		QQuickItem* quickItem = quickObjectPtr->GetQuickItem();
		if (quickItem == nullptr){
			return false;
		}

		quickItem->setProperty("productId", param.productId);
		QMetaObject::invokeMethod(
					quickItem,
					"setLoginData",
					Q_ARG(QVariant, QVariant::fromValue(param.accessToken)),
					Q_ARG(QVariant, QVariant::fromValue(param.userName)),
					Q_ARG(QVariant, QVariant::fromValue(param.permissions)));

		return true;
	}

private:
	mutable CAdministrationWidget m_sdk;
};


// public methods

CAdministrationViewWidget::CAdministrationViewWidget()
	: m_implPtr(nullptr)
{
	InitializeAdministrationWidgetResources();

	m_implPtr = new CAdministrationViewWidgetImpl;
	QWidget* widgetPtr = m_implPtr->GetWidget(this);
	if (widgetPtr != nullptr){
		auto* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(widgetPtr);
		setLayout(layout);
	}
}


CAdministrationViewWidget::~CAdministrationViewWidget()
{
	delete m_implPtr;
}


bool CAdministrationViewWidget::SetConnectionParam(const ServerConfig& config) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetConnectionParam(config);
	}

	return false;
}


bool CAdministrationViewWidget::SetLoginParam(Login param) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetLoginParam(param);
	}

	return false;
}


} // namespace AuthClientSdk
