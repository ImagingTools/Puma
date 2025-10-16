#pragma once


// Qt includes
#include <QtWidgets/QWidget>

// PumaClientSdk includes
#include <PumaClientSdk/PumaClientSdk.h>


namespace PumaClientSdk
{


class CAdministrationViewWidgetImpl;


class CAdministrationViewWidget: public QWidget
{
public:
	CAdministrationViewWidget();
	~CAdministrationViewWidget();

	bool SetConnectionParam(const QString& host, int httpPort, int wsPort);
	bool SetLoginParam(Login param);

private:
	CAdministrationViewWidgetImpl* m_implPtr;
};


} // namespace PumaClientSdk


