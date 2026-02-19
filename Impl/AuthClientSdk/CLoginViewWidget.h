// SPDX-License-Identifier: LicenseRef-Puma-Commercial
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

	bool SetConnectionParam(const QString& host, int httpPort, int wsPort) const;
	bool SetLoginParam(Login param) const;

private:
	CLoginViewWidgetImpl* m_implPtr;
};


} // namespace AuthClientSdk


