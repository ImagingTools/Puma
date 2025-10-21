// ImtCore includes
#include <imtbase/Init.h>

// PumaClient includes
#include <GeneratedFiles/PumaClient/CPumaClient.h>

#include <AuthClientSdk/CAdministrationViewWidget.h>
#include <AuthServerSdk/AuthServerSdk.h>
#include <AuthClientSdk/AuthClientSdk.h>
#include <QLayout>


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(PumaClient);

	QApplication app(argc, argv);

	AuthServerSdk::CAuthorizableServer server;
	server.SetDatabaseSettings("localhost", 5432, "test", "postgres", "root");
	server.SetPumaConnectionParam("localhost", 7788, 8788);
	server.SetProductId("Test");
	server.SetFeaturesFilePath(":/Features/TestFeatures");

	server.Start(7777, 8888);

	AuthClientSdk::CAuthorizationController authorizationController;

	AuthClientSdk::Login loginData;
	authorizationController.SetProductId("Test");

	bool ok = authorizationController.Login("su", "1", loginData);
	if (ok){
		qDebug() << "Login successfuly" << loginData.accessToken;
	}
	else{
		qDebug() << "Login failed";
	}

	AuthClientSdk::CAdministrationViewWidget loginWidget;
	loginWidget.SetConnectionParam("localhost", 7777, 8888);
	loginWidget.SetLoginParam(loginData);
	loginWidget.setWindowTitle("Login Widget");
	loginWidget.resize(1000, 1000);
	loginWidget.show();

	return app.exec();
}

