#include <PumaClientSdk/CAdministrationViewWidget.h>


// Qt includes
#include <QtWidgets/QVBoxLayout>
#include <QtQuickWidgets/QQuickWidget>

// ACF includes
#include <ipackage/CComponentAccessor.h>
#include <iqtgui/IGuiObject.h>
#include <iprm/IIdParam.h>

// ImtCore includes
#include <imtqml/IQuickObject.h>
#include <imtcom/IServerConnectionInterface.h>

// Local includes
#include <GeneratedFiles/PumaClientSdk/CAdministrationWidget.h>


namespace PumaClientSdk
{


class CAdministrationViewWidgetImpl
{
public:
	CAdministrationViewWidgetImpl()
	{
	}


	QWidget* GetWidget()
	{
		iqtgui::IGuiObject* guiObjectPtr = m_sdk.GetInterface<iqtgui::IGuiObject>("GuiQuickWrap");
		if (guiObjectPtr != nullptr){
			return guiObjectPtr->GetWidget();
		}

		return nullptr;
	}


	bool SetConnectionParam(const QString& host, int httpPort, int wsPort)
	{
		imtcom::IServerConnectionInterface* serverConnectionParamPtr = m_sdk.GetInterface<imtcom::IServerConnectionInterface>("ServerConnectionInterfaceParam");
		if (serverConnectionParamPtr != nullptr){
			serverConnectionParamPtr->SetHost(host);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_HTTP, httpPort);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_WEBSOCKET, wsPort);

			return true;
		}

		return false;
	}


	bool SetProductId(const QByteArray& productId)
	{
		iprm::IIdParam* guiObjectPtr = m_sdk.GetInterface<iprm::IIdParam>("ProductIdParam");
		if (guiObjectPtr != nullptr){
			guiObjectPtr->SetId(productId);
			return true;
		}

		return false;
	}

private:
	mutable CAdministrationWidget m_sdk;
};


// public methods

CAdministrationViewWidget::CAdministrationViewWidget()
	:m_implPtr(nullptr)
{
	m_implPtr = new CAdministrationViewWidgetImpl;
	QWidget* widgetPtr = m_implPtr->GetWidget();
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


QWidget* CAdministrationViewWidget::GetWidget() const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetWidget();
	}

	return nullptr;
}


bool CAdministrationViewWidget::SetConnectionParam(const QString& host, int httpPort, int wsPort)
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetConnectionParam(host, httpPort, wsPort);
	}

	return false;
}


bool CAdministrationViewWidget::SetProductId(const QByteArray& productId)
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetProductId(productId);
	}

	return false;
}


} // namespace PumaClientSdk


