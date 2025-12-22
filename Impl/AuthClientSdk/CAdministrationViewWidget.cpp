#include <AuthClientSdk/CAdministrationViewWidget.h>


// Qt includes
#include <QtWidgets/QVBoxLayout>

// ACF includes
#include <iqtgui/IGuiObject.h>
#include <istd/CChangeGroup.h>

// ImtCore includes
#include <imtbase/Init.h>
#include <imtbase/IApplicationInfoController.h>
#include <imtcom/IServerConnectionInterface.h>
#include <imtqml/IQuickObject.h>

// Local includes
#include <GeneratedFiles/AuthClientSdk/CAdministrationWidget.h>


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
		iqtgui::IGuiObject* guiObjectPtr = m_sdk.GetInterface<iqtgui::IGuiObject>();
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
		imtcom::IServerConnectionInterface* serverConnectionParamPtr = m_sdk.GetInterface<imtcom::IServerConnectionInterface>();
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
		imtbase::IApplicationInfoController* applicationInfoControllerPtr = m_sdk.GetInterface<imtbase::IApplicationInfoController>();
		if (applicationInfoControllerPtr == nullptr){
			return false;
		}

		applicationInfoControllerPtr->SetApplicationAttribute(imtbase::IApplicationInfoController::ApplicationAttribute::AA_APPLICATION_ID, param.productId);

		imtqml::IQuickObject* quickObjectPtr = m_sdk.GetInterface<imtqml::IQuickObject>();
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
	:m_implPtr(nullptr)
{
	DefaultImtCoreQmlInitializer initializer;
	initializer.Init();

	m_implPtr = new CAdministrationViewWidgetImpl;
	QWidget* widgetPtr = m_implPtr->GetWidget(this);
	if (widgetPtr != nullptr){
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(widgetPtr);
		setLayout(layout);
	}
}


CAdministrationViewWidget::~CAdministrationViewWidget()
{
	if (m_implPtr != nullptr){
		delete m_implPtr;
	}
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


