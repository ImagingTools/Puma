// SPDX-License-Identifier: LicenseRef-Puma-Commercial
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


