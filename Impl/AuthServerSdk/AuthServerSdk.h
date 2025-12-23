#pragma once


/**
* @file AuthServerSdk.h
* @brief Public API of the Authorizable Server SDK.
*
* This header defines configuration structures and the main
* `CAuthorizableServer` class used to configure, start, and control
* an authorizable server with optional SSL/TLS support.
*
* The SDK is intended to be used by external applications that need
* to embed or control the authorization server.
*
* @ingroup AuthServerSdk
*/

#if defined _MSC_VER || defined __MINGW32__ || defined __MINGW64__
#define AUTH_SERVER_SDK_EXPORT __declspec(dllexport)
#elif defined __GNUC__
#define AUTH_SERVER_SDK_EXPORT __attribute__((visibility("default")))
#else
#define AUTH_SERVER_SDK_EXPORT
#endif


// STL includes
#include <optional>

// Qt includes
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtNetwork/QSslConfiguration>
#include <QtNetwork/QSslSocket>


namespace AuthServerSdk
{


class CAuthorizableServerImpl;


/**
* @brief SSL/TLS configuration parameters.
*
* This structure defines all SSL-related settings required to run
* the server in secure mode, including certificates, private keys,
* trusted Certificate Authorities, and verification policies.
*
* The structure is optional and supplied via `ServerConfig`.
*/
struct SslConfig
{
	/**
	* @brief Path to the local (server) certificate file.
	*/
	QString localCertificatePath;

	/**
	* @brief Encoding format of the local certificate.
	*
	* Default value is PEM.
	*/
	QSsl::EncodingFormat localCertificateFormat = QSsl::Pem;

	/**
	* @brief Path to the private key file.
	*/
	QString privateKeyPath;

	/**
	* @brief Private key algorithm.
	*
	* Typically RSA or EC.
	*/
	QSsl::KeyAlgorithm privateKeyAlgorithm = QSsl::Rsa;

	/**
	* @brief Encoding format of the private key.
	*/
	QSsl::EncodingFormat privateKeyFormat = QSsl::Pem;

	/**
	* @brief Passphrase for the private key.
	*
	* Leave empty if the key is not encrypted.
	*/
	QByteArray privateKeyPassPhrase;

	/**
	* @brief List of paths to trusted CA certificates.
	*
	* These certificates are used to verify client certificates
	* when peer verification is enabled.
	*/
	QList<QString> caCertificatePaths;

	/**
	* @brief Encoding format of CA certificates.
	*/
	QSsl::EncodingFormat caCertificateFormat = QSsl::Pem;

	/**
	* @brief Peer verification mode.
	*
	* Controls whether and how client certificates are verified.
	*/
	QSslSocket::PeerVerifyMode verifyMode =
		QSslSocket::PeerVerifyMode::AutoVerifyPeer;

	/**
	* @brief SSL/TLS protocol version.
	*
	* Defines the minimum or exact TLS protocol version to be used.
	*/
	QSsl::SslProtocol protocol = QSsl::TlsV1_2;
};


/**
* @brief General server configuration.
*
* This structure defines network-level parameters for the server,
* including ports, host, and optional SSL configuration.
*/
struct ServerConfig
{
	/**
	* @brief HTTP port number.
	*
	* Used for plain HTTP or HTTPS depending on SSL configuration.
	*/
	int httpPort = 80;

	/**
	* @brief WebSocket port number.
	*/
	int wsPort = 90;

	/**
	* @brief Host address or interface.
	*
	* Examples: "localhost", "0.0.0.0", "::1".
	*/
	QString host = "localhost";

	/**
	* @brief Optional SSL configuration.
	*
	* If not set, the server operates in non-secure mode.
	*/
	std::optional<SslConfig> sslConfig;
};


/**
* @brief Main Authorizable Server interface.
*
* This class provides a high-level API for controlling the lifecycle
* and configuration of the authorizable server.
*
* Internally, it uses the PIMPL pattern to hide implementation details
* and ensure ABI stability.
*/
class AUTH_SERVER_SDK_EXPORT CAuthorizableServer
{
public:
	/**
	* @brief Constructs the server instance.
	*
	* The server is not started automatically.
	*/
	CAuthorizableServer();

	/**
	* @brief Virtual destructor.
	*
	* Stops the server if it is still running and releases resources.
	*/
	virtual ~CAuthorizableServer();

	/**
	* @brief Starts the server.
	*
	* Initializes network listeners and applies the provided configuration.
	*
	* @param serverConfig Server configuration parameters.
	* @return true if the server was successfully started, false otherwise.
	*/
	virtual bool Start(const ServerConfig& serverConfig) const;

	/**
	* @brief Stops the server.
	*
	* Gracefully shuts down all listeners and active connections.
	*
	* @return true if the server was successfully stopped, false otherwise.
	*/
	virtual bool Stop() const;

	/**
	* @brief Sets the path to the features configuration file.
	*
	* This file typically defines enabled features, licenses,
	* or authorization rules.
	*
	* @param filePath Absolute or relative path to the features file.
	* @return true if the path was accepted, false otherwise.
	*/
	virtual bool SetFeaturesFilePath(const QString& filePath) const;

	/**
	* @brief Configures Puma connection parameters.
	*
	* Applies server configuration parameters used for integration
	* with the Puma backend or service.
	*
	* @param serverConfig Puma-related server configuration.
	* @return true if the parameters were applied successfully.
	*/
	virtual bool SetPumaConnectionParam(const ServerConfig& serverConfig) const;

	/**
	* @brief Sets the product identifier.
	*
	* The product ID is used for authorization, licensing,
	* or telemetry purposes.
	*
	* @param productId Binary product identifier.
	* @return true if the product ID was set successfully.
	*/
	virtual bool SetProductId(const QByteArray& productId) const;

private:
	/**
	* @brief Pointer to the internal implementation.
	*
	* Hidden via PIMPL to preserve ABI compatibility.
	*/
	CAuthorizableServerImpl* m_implPtr;
};

} // namespace AuthServerSdk


/**
* @defgroup AuthServerSdk Authorizable Server SDK
*
* @brief Public API for the Authorizable Server.
*
* This module provides interfaces and configuration structures
* required to embed and control the authorization server.
*/

/**
* @mainpage
*
* @section Introduction
*
* The Authorizable Server SDK provides a C++ interface for running
* an authorization-enabled server with optional SSL/TLS support.
*
* The SDK is designed to be embedded into host applications and
* exposes a minimal, stable API suitable for long-term integration.
*/
