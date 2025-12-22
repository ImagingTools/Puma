#include <AuthServerSdk/AuthServerSdk.h>


// Qt includes
#include <QtCore/QFile>

// ACF includes
#include <iauth/ILogin.h>
#include <iauth/IRightsProvider.h>
#include <istd/CChangeGroup.h>
#include <iser/CCompactXmlMemReadArchive.h>

// ImtCore includes
#include <imtbase/IApplicationInfoController.h>
#include <imtlic/IProductInfo.h>
#include <imtdb/IDatabaseLoginSettings.h>
#include <imtcom/IServerConnectionInterface.h>
#include <imtcom/IServerDispatcher.h>
#include <imtcom/ISslConfigurationApplier.h>

// Local includes
#include <GeneratedFiles/AuthServerSdk/CAuthServerSdk.h>


namespace AuthServerSdk
{


class CAuthorizableServerImpl
{
public:
	CAuthorizableServerImpl()
	{
		m_sdk.EnsureAutoInitComponentsCreated();
	}


	bool Start(const ServerConfig& serverConfig)
	{
		imtcom::IServerDispatcher* serverControllerPtr = m_sdk.GetInterface<imtcom::IServerDispatcher>();
		if (serverControllerPtr == nullptr){
			qWarning() << "Server dispatcher interface is not available";
			return false;
		}

		if (!SetConnectionParam(serverConfig)){
			qWarning() << "Failed to set connection parameters: HTTP" << serverConfig.httpPort << "WS" << serverConfig.wsPort;
			return false;
		}

		if (serverConfig.sslConfig.has_value()){
			const SslConfig& sslConfig = *serverConfig.sslConfig;
			imtcom::ISslConfigurationApplier* sslConfigurationApplierPtr = m_sdk.GetInterface<imtcom::ISslConfigurationApplier>();
			if (sslConfigurationApplierPtr == nullptr){
				qWarning() << "SSL configuration applier interface is not available";
				return false;
			}

			if (sslConfig.localCertificatePath.isEmpty() || sslConfig.privateKeyPath.isEmpty()){
				qWarning() << "Local certificate or private key path is empty";
				return false;
			}

			// Local certificate
			if (!sslConfigurationApplierPtr->LoadLocalCertificateFromFile(
					sslConfig.localCertificatePath, sslConfig.localCertificateFormat)){
				qWarning() << "Failed to load local certificate from" << sslConfig.localCertificatePath;
				return false;
			}
			qDebug() << "Loaded local certificate from" << sslConfig.localCertificatePath;

			// CA certificates
			if (!sslConfig.caCertificatePaths.isEmpty()){
				for (const QString& certPath : sslConfig.caCertificatePaths){
					if (!sslConfigurationApplierPtr->LoadCaCertificatesFromFile(certPath, sslConfig.caCertificateFormat)){
						qWarning() << "Failed to load CA certificate from" << certPath;
						return false;
					}
					qDebug() << "Loaded CA certificate from" << certPath;
				}
			}
			else{
				qDebug() << "No CA certificates specified";
			}

			// Private key
			if (!sslConfigurationApplierPtr->LoadPrivateKeyFromFile(
					sslConfig.privateKeyPath,
					sslConfig.privateKeyAlgorithm,
					sslConfig.privateKeyFormat,
					sslConfig.privateKeyPassPhrase)){
				qWarning() << "Failed to load private key from" << sslConfig.privateKeyPath;
				return false;
			}
			qDebug() << "Loaded private key from" << sslConfig.privateKeyPath;

			// SSL protocol
			if (!sslConfigurationApplierPtr->SetSslProtocol(sslConfig.protocol)){
				qWarning() << "Failed to set SSL protocol:" << sslConfig.protocol;
				return false;
			}
			qDebug() << "SSL protocol set to" << sslConfig.protocol;

			// Peer verify mode
			if (!sslConfigurationApplierPtr->SetPeerVerifyMode(sslConfig.verifyMode)){
				qWarning() << "Failed to set peer verify mode:" << sslConfig.verifyMode;
				return false;
			}
			qDebug() << "Peer verify mode set to" << static_cast<int>(sslConfig.verifyMode);
		}

		if (!serverControllerPtr->StartServer(imtcom::IServerConnectionInterface::PT_HTTP)){
			qWarning() << "Failed to start HTTP server on port" << serverConfig.httpPort;
			return false;
		}
		qDebug() << "HTTP server started on port" << serverConfig.httpPort;

		if (!serverControllerPtr->StartServer(imtcom::IServerConnectionInterface::PT_WEBSOCKET)){
			qWarning() << "Failed to start WebSocket server on port" << serverConfig.wsPort;
			return false;
		}
		qDebug() << "WebSocket server started on port" << serverConfig.wsPort;

		qDebug() << "Server started successfully";

		return true;
	}


	bool Stop()
	{
		imtcom::IServerDispatcher* serverControllerPtr = m_sdk.GetInterface<imtcom::IServerDispatcher>();
		if (serverControllerPtr == nullptr){
			qWarning() << "Server dispatcher interface is not available";
			return false;
		}

		if (!serverControllerPtr->StopServer(imtcom::IServerConnectionInterface::PT_HTTP)){
			return false;
		}

		if (!serverControllerPtr->StopServer(imtcom::IServerConnectionInterface::PT_WEBSOCKET)){
			return false;
		}

		return true;
	}


	bool SetFeaturesFilePath(const QString& filePath)
	{
		imtlic::IProductInfo* productInfoPtr = m_sdk.GetInterface<imtlic::IProductInfo>();
		if (productInfoPtr == nullptr){
			return false;
		}

		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)){
			return false;
		}

		QByteArray data = file.readAll();
		file.close();

		iser::CCompactXmlMemReadArchive archive(data);

		if (!productInfoPtr->Serialize(archive)){
			return false;
		}

		return true;
	}


	bool SetConnectionParam(const ServerConfig& serverConfig)
	{
		return SetConnectionParamInternal(serverConfig, "ServerConnectionInterfaceParam");
	}


	bool SetPumaConnectionParam(const ServerConfig& serverConfig)
	{
		return SetConnectionParamInternal(serverConfig, "PumaConnectionInterfaceParam");
	}


	bool SetProductId(const QByteArray& productId)
	{
		imtbase::IApplicationInfoController* applicationInfoControllerPtr = m_sdk.GetInterface<imtbase::IApplicationInfoController>();
		if (applicationInfoControllerPtr == nullptr){
			return false;
		}

		applicationInfoControllerPtr->SetApplicationAttribute(imtbase::IApplicationInfoController::ApplicationAttribute::AA_APPLICATION_ID, productId);

		return true;
	}

private:
	bool SetConnectionParamInternal(const ServerConfig& serverConfig, const QByteArray& componentId)
	{
		imtcom::IServerConnectionInterface* serverConnectionParamPtr = m_sdk.GetInterface<imtcom::IServerConnectionInterface>(componentId);
		if (serverConnectionParamPtr != nullptr){
			serverConnectionParamPtr->SetHost(serverConfig.host);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_HTTP, serverConfig.httpPort);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_WEBSOCKET, serverConfig.wsPort);

			if (serverConfig.sslConfig.has_value()){
				serverConnectionParamPtr->SetConnectionFlags(imtcom::IServerConnectionInterface::CF_SECURE);
			}

			return true;
		}

		return false;
	}

private:
	mutable CAuthServerSdk m_sdk;
};


// public methods

CAuthorizableServer::CAuthorizableServer()
	: m_implPtr(nullptr)
{
	m_implPtr = new CAuthorizableServerImpl;
}


CAuthorizableServer::~CAuthorizableServer()
{
	delete m_implPtr;
}


bool CAuthorizableServer::Start(const ServerConfig& serverConfig) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->Start(serverConfig);
	}

	return false;
}


bool CAuthorizableServer::Stop() const
{
	if (m_implPtr != nullptr){
		return m_implPtr->Stop();
	}

	return false;
}


bool CAuthorizableServer::SetFeaturesFilePath(const QString& filePath) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetFeaturesFilePath(filePath);
	}

	return false;
}


bool CAuthorizableServer::SetPumaConnectionParam(const ServerConfig& serverConfig) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetPumaConnectionParam(serverConfig);
	}

	return false;
}


bool CAuthorizableServer::SetProductId(const QByteArray& productId) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetProductId(productId);
	}

	return false;
}


} // namespace AuthServerSdk


