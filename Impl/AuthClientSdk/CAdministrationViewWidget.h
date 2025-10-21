#pragma once


// Qt includes
#include <QtWidgets/QWidget>

// AuthClientSdk includes
#include <AuthClientSdk/AuthClientSdk.h>


namespace AuthClientSdk
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


} // namespace AuthClientSdk


