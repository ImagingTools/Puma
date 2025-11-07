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
	// AuthServerSdk::CAuthorizableServer server;
	// server.SetDatabaseSettings("localhost", 5432, "test", "postgres", "root");
	// server.SetPumaConnectionParam("localhost", 7788, 8788);
	// server.SetProductId("ProLife");
	// server.SetFeaturesFilePath(":/Features/TestFeatures");

	// server.Start(7777, 8888);

	AuthClientSdk::CAuthorizationController authorizationController;

	AuthClientSdk::Login loginData;
	authorizationController.SetProductId("ProLife");

	QString errorMessage;
	AuthClientSdk::SuperuserStatus suExists = authorizationController.SuperuserExists(errorMessage);
	qDebug() << "SU exists" << suExists;

	if (suExists == AuthClientSdk::NotExists){
		authorizationController.CreateSuperuser("1");
	}

	QByteArrayList userIds = authorizationController.GetUserIds();
	QByteArray userId = authorizationController.CreateUser("Ivan", "Ivan", "1", "ivan@mail.ru");

	bool ok = authorizationController.Login("su", "1", loginData);
	if (ok){
		qDebug() << "Login successfuly" << loginData.accessToken;
	}
	else{
		qDebug() << "Login failed";
	}

	QByteArrayList userIds2 = authorizationController.GetUserIds();

	AuthClientSdk::CAdministrationViewWidget loginWidget;
	loginWidget.SetConnectionParam("localhost", 7778, 8778);
	loginWidget.SetLoginParam(loginData);
	loginWidget.setWindowTitle("Login Widget");
	loginWidget.resize(1000, 1000);
	loginWidget.show();

	return app.exec();
}

