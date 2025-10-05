#include <PumaClientSdk/CLoginViewWidget.h>


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
#include <GeneratedFiles/PumaClientSdk/CLoginWidget.h>


namespace PumaClientSdk
{


class CLoginViewWidgetImpl
{
public:
	CLoginViewWidgetImpl()
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

private:
	mutable CLoginWidget m_sdk;
};


// public methods

CLoginViewWidget::CLoginViewWidget()
	:m_implPtr(nullptr)
{
	m_implPtr = new CLoginViewWidgetImpl;
	QWidget* widgetPtr = m_implPtr->GetWidget();
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


} // namespace PumaClientSdk


