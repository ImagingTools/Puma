// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-Puma-Commercial
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

	bool SetConnectionParam(const ServerConfig& config) const;
	bool SetLoginParam(Login param) const;

private:
	CAdministrationViewWidgetImpl* m_implPtr;
};


} // namespace AuthClientSdk


