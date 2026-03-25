// SPDX-License-Identifier: LicenseRef-Puma-Commercial

// Qt includes
#include <QLayout>
#include <QApplication>
#include <QDebug>

// ImtCore includes
#include <imtbase/Init.h>

// PumaClient includes
#include <GeneratedFiles/PumaClient/CPumaClient.h>

// Authorization SDK includes
#include <AuthClientSdk/CAdministrationViewWidget.h>
#include <AuthServerSdk/AuthServerSdk.h>
#include <AuthClientSdk/AuthClientSdk.h>


int main(int argc, char *argv[])
{
	// Initialize Qt resource system for PumaClient
	Q_INIT_RESOURCE(PumaClient);

	// Initialize Qt application
	QApplication app(argc, argv);

	// ---------------------------------------------------------------------
	// Server-side SSL configuration
	// ---------------------------------------------------------------------

	// Path to the server certificate
	QString certPath = "C:/cert/server.crt";

	// Configure SSL for the authorization server
	AuthServerSdk::SslConfig sslConfig;
	sslConfig.localCertificatePath = certPath;          // Server certificate
	sslConfig.caCertificatePaths << certPath;           // Trusted CA certificates
	sslConfig.privateKeyPath = "C:/cert/server.key";    // Private key file

	// Create authorization server instance
	AuthServerSdk::CAuthorizableServer server;

	// ---------------------------------------------------------------------
	// Puma backend connection configuration
	// ---------------------------------------------------------------------

	AuthServerSdk::ServerConfig pumaConfig;
	pumaConfig.wsPort = 8788;                            // Puma WebSocket port
	pumaConfig.httpPort = 7788;                          // Puma HTTP port
	pumaConfig.sslConfig = sslConfig;                    // Enable SSL

	// Apply Puma connection parameters
	server.SetPumaConnectionParam(pumaConfig);

	// Set product identifier used for authorization/licensing
	server.SetProductId("ProLife");

	// Set path to features configuration file (Qt resource)
	server.SetFeaturesFilePath(":/Features/TestFeatures");

	// ---------------------------------------------------------------------
	// Authorization server configuration
	// ---------------------------------------------------------------------

	AuthServerSdk::ServerConfig config;
	config.wsPort = 8888;                                // Server WebSocket port
	config.httpPort = 7777;                              // Server HTTP port
	config.sslConfig = sslConfig;                        // Enable SSL

	// Start authorization server
	server.Start(config);

	// ---------------------------------------------------------------------
	// Client-side authorization controller
	// ---------------------------------------------------------------------

	AuthClientSdk::CAuthorizationController authorizationController;

	// Configure SSL for client-side connection
	AuthClientSdk::SslConfig clientSslConfig;
	clientSslConfig.caCertificatePaths << certPath;     // Trust server certificate

	// Configure server connection parameters
	AuthClientSdk::ServerConfig serverConfig;
	serverConfig.wsPort = 8888;
	serverConfig.httpPort = 7777;
	serverConfig.sslConfig = clientSslConfig;

	// Apply connection parameters to the client controller
	authorizationController.SetConnectionParam(serverConfig);

	// Structure to receive login session data
	AuthClientSdk::Login loginData;

	// Set product identifier on client side
	authorizationController.SetProductId("ProLife");

	// ---------------------------------------------------------------------
	// Superuser check and basic user management
	// ---------------------------------------------------------------------

	QString errorMessage;

	// Check whether a superuser exists on the server
	AuthClientSdk::SuperuserStatus suExists =
		authorizationController.SuperuserExists(errorMessage);

	qDebug() << "Superuser exists:" << static_cast<int>(suExists);

	// Retrieve all existing user IDs
	QByteArrayList userIds = authorizationController.GetUserIds();

	// Create a new user
	QByteArray userId = authorizationController.CreateUser(
		"Ivan",               // User name
		"Ivan",               // Login
		"1",                  // Password
		"ivan@mail.ru"        // Email
	);

	// ---------------------------------------------------------------------
	// Authentication
	// ---------------------------------------------------------------------

	// Perform login as superuser
	bool ok = authorizationController.Login("su", "1", loginData);
	if (ok)
	{
		qDebug() << "Login successful, token:" << loginData.accessToken;
	}
	else
	{
		qDebug() << "Login failed";
	}

	// Determine authentication system used by the user
	AuthClientSdk::SystemType systemType =
		authorizationController.GetUserAuthSystem("su");

	// Retrieve user data by login
	AuthClientSdk::User suUserData;
	authorizationController.GetUserByLogin("su", suUserData);

	// Retrieve updated list of user IDs
	QByteArrayList userIds2 = authorizationController.GetUserIds();

	// ---------------------------------------------------------------------
	// Administration UI widget
	// ---------------------------------------------------------------------

	// Create administration view widget
	AuthClientSdk::CAdministrationViewWidget loginWidget;

	// Apply server connection parameters
	loginWidget.SetConnectionParam(serverConfig);

	// Provide login session data
	loginWidget.SetLoginParam(loginData);

	// Configure UI
	loginWidget.setWindowTitle("Login Widget");
	loginWidget.resize(1000, 1000);
	loginWidget.show();

	// Enter Qt event loop
	return app.exec();
}


