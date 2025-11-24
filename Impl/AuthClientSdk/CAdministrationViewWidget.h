#pragma once


// Qt includes
#include <QtWidgets/QWidget>

// AuthClientSdk includes
#include <AuthClientSdk/AuthClientSdk.h>


namespace AuthClientSdk
{


class CAdministrationViewWidgetImpl;


class AUTH_CLIENT_SDK_EXPORT CAdministrationViewWidget: public QWidget
{
public:
	CAdministrationViewWidget();
	~CAdministrationViewWidget();

	bool SetConnectionParam(const QString& host, int httpPort, int wsPort) const;
	bool SetLoginParam(Login param) const;

private:
	CAdministrationViewWidgetImpl* m_implPtr;
};


} // namespace AuthClientSdk


