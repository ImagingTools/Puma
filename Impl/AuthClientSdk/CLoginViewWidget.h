#pragma once


// Qt includes
#include <QtWidgets/QWidget>

// AuthClientSdk includes
#include <AuthClientSdk/AuthClientSdk.h>


namespace AuthClientSdk
{


class CLoginViewWidgetImpl;


class AUTH_CLIENT_SDK_EXPORT CLoginViewWidget: public QWidget
{
public:
	CLoginViewWidget();
	~CLoginViewWidget();

	bool SetConnectionParam(const QString& host, int httpPort, int wsPort);
	bool SetLoginParam(Login param);

private:
	CLoginViewWidgetImpl* m_implPtr;
};


} // namespace AuthClientSdk


