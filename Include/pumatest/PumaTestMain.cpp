// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-Puma-Commercial
#include <itest/CStandardTestExecutor.h>


// Qt includes
#include <QtWidgets/QApplication>


#include <AuthClientSdk/AuthClientSdk.h>

int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(imtdb);

	AuthClientSdk::CAuthorizationController authorizationController;

	QApplication app(argc, argv);
	itest::CStandardTestExecutor instance;
	return instance.RunTests(argc, argv);
}


