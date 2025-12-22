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

	QString certPath = "C:/cert/server.crt";
	AuthServerSdk::SslConfig sslConfig;
	sslConfig.localCertificatePath = certPath;
	sslConfig.caCertificatePaths << certPath;
	sslConfig.privateKeyPath = "C:/cert/server.key";

	AuthServerSdk::CAuthorizableServer server;

	AuthServerSdk::ServerConfig pumaConfig;
	pumaConfig.wsPort = 8788;
	pumaConfig.httpPort = 7788;
	pumaConfig.sslConfig = sslConfig;

	server.SetPumaConnectionParam(pumaConfig);
	server.SetProductId("ProLife");
	server.SetFeaturesFilePath(":/Features/TestFeatures");

	AuthServerSdk::ServerConfig config;
	config.wsPort = 8888;
	config.httpPort = 7777;
	config.sslConfig = sslConfig;

	server.Start(config);

	AuthClientSdk::CAuthorizationController authorizationController;

	AuthClientSdk::SslConfig clientSslConfig;
	clientSslConfig.caCertificatePaths << certPath;

	AuthClientSdk::ServerConfig serverConfig;
	serverConfig.wsPort = 8888;
	serverConfig.httpPort = 7777;
	serverConfig.sslConfig = clientSslConfig;
	authorizationController.SetConnectionParam(serverConfig);

	AuthClientSdk::Login loginData;
	authorizationController.SetProductId("ProLife");

	QString errorMessage;
	AuthClientSdk::SuperuserStatus suExists = authorizationController.SuperuserExists(errorMessage);
	qDebug() << "Superuser exists" << (int) suExists;

	QByteArrayList userIds = authorizationController.GetUserIds();
	QByteArray userId = authorizationController.CreateUser("Ivan", "Ivan", "1", "ivan@mail.ru");

	bool ok = authorizationController.Login("su", "1", loginData);
	if (ok){
		qDebug() << "Login successful" << loginData.accessToken;
	}
	else{
		qDebug() << "Login failed";
	}

	AuthClientSdk::SystemType systemType = authorizationController.GetUserAuthSystem("su");

	AuthClientSdk::User suUserData;
	authorizationController.GetUserByLogin("su", suUserData);

	QByteArrayList userIds2 = authorizationController.GetUserIds();

	AuthClientSdk::CAdministrationViewWidget loginWidget;
	loginWidget.SetConnectionParam(serverConfig);
	loginWidget.SetLoginParam(loginData);
	loginWidget.setWindowTitle("Login Widget");
	loginWidget.resize(1000, 1000);
	loginWidget.show();

	return app.exec();
}

