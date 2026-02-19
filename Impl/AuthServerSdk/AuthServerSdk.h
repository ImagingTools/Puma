// SPDX-License-Identifier: LicenseRef-Puma-Commercial
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
* to embed or control the authorization server. It provides:
* - Server lifecycle management (start/stop)
* - Network configuration (HTTP and WebSocket)
* - SSL/TLS security configuration
* - Product and feature licensing
* - Integration with Puma backend
*
* @section usage_example Basic Usage Example
* @code
* using namespace AuthServerSdk;
*
* // Create the server instance
* CAuthorizableServer server;
*
* // Configure server settings
* ServerConfig config;
* config.host = "0.0.0.0";  // Listen on all interfaces
* config.httpPort = 8080;
* config.wsPort = 8081;
*
* // Optional: Configure SSL/TLS for production
* SslConfig sslConfig;
* sslConfig.localCertificatePath = "/etc/ssl/server.crt";
* sslConfig.privateKeyPath = "/etc/ssl/server.key";
* sslConfig.caCertificatePaths = {"/etc/ssl/ca.pem"};
* sslConfig.protocol = QSsl::TlsV1_3;
* sslConfig.verifyMode = QSslSocket::VerifyPeer;
* config.sslConfig = sslConfig;
*
* // Set product ID and features
* server.SetProductId("MyAuthServer");
* server.SetFeaturesFilePath("/path/to/features.xml");
*
* // Start the server
* if (server.Start(config)) {
*     qDebug() << "Server started successfully";
*     // Server is now running and accepting connections
* } else {
*     qCritical() << "Failed to start server";
* }
*
* // Later, stop the server gracefully
* server.Stop();
* @endcode
*
* @section security_notes Security Considerations
*
* - **Always use SSL/TLS in production** to protect credentials and sensitive data
* - **Verify client certificates** when required using appropriate PeerVerifyMode
* - **Use strong cipher suites** by selecting modern SSL protocols (TLS 1.2+)
* - **Protect private keys** with appropriate file permissions and passphrases
* - **Validate feature files** to prevent unauthorized feature activation
*
* @section thread_safety Thread Safety
*
* The CAuthorizableServer class is generally thread-safe. However, Start()
* and Stop() should not be called concurrently. Configuration methods
* should be called before Start() or after Stop().
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
* SSL/TLS provides:
* - **Encryption**: Protects data in transit from eavesdropping
* - **Authentication**: Verifies server identity to clients
* - **Integrity**: Ensures data is not tampered with
*
* The structure is optional and supplied via `ServerConfig`. If not
* provided, the server runs in non-secure mode (plain HTTP/WebSocket).
*
* @warning Always use SSL/TLS in production environments to protect
*          authentication credentials and sensitive authorization data.
*
* @section ssl_setup SSL Setup Guide
*
* 1. Generate or obtain server certificate and private key
* 2. Configure paths to certificate and key files
* 3. Optionally configure CA certificates for client verification
* 4. Choose appropriate SSL protocol (TLS 1.2 or higher recommended)
* 5. Set peer verification mode based on security requirements
*
* @see ServerConfig
*/
struct SslConfig
{
	/**
	* @brief Path to the local (server) certificate file.
	*
	* The server certificate is presented to clients during the SSL/TLS
	* handshake to prove the server's identity. It should be signed by
	* a trusted Certificate Authority (CA) or be a self-signed certificate
	* for testing environments.
	*
	* @note The certificate file should be readable by the server process
	*       and protected from unauthorized access.
	*/
	QString localCertificatePath;

	/**
	* @brief Encoding format of the local certificate.
	*
	* Specifies whether the certificate file is in PEM (Base64-encoded,
	* text format with BEGIN/END markers) or DER (binary) format.
	*
	* Default value is PEM, which is the most common format.
	*
	* @note PEM files typically have .pem, .crt, or .cer extensions.
	*       DER files typically have .der or .cer extensions.
	*/
	QSsl::EncodingFormat localCertificateFormat = QSsl::Pem;

	/**
	* @brief Path to the private key file.
	*
	* The private key corresponds to the server certificate and is used
	* during the SSL/TLS handshake to prove ownership of the certificate.
	*
	* @warning The private key must be kept secure and should never be
	*          shared or transmitted. File permissions should restrict
	*          access to the server process only (e.g., chmod 600).
	*
	* @note If the key is encrypted (password-protected), provide the
	*       passphrase in privateKeyPassPhrase.
	*/
	QString privateKeyPath;

	/**
	* @brief Private key algorithm.
	*
	* Specifies the cryptographic algorithm used by the private key.
	* Common options:
	* - QSsl::Rsa: RSA (most common, 2048-4096 bits recommended)
	* - QSsl::Ec: Elliptic Curve (faster, smaller keys with equivalent security)
	* - QSsl::Dsa: Digital Signature Algorithm (less common)
	*
	* Default is RSA. The algorithm must match the key type.
	*/
	QSsl::KeyAlgorithm privateKeyAlgorithm = QSsl::Rsa;

	/**
	* @brief Encoding format of the private key.
	*
	* Specifies whether the private key file is in PEM (text format)
	* or DER (binary) format. Default is PEM.
	*
	* @note PEM private keys typically start with "-----BEGIN PRIVATE KEY-----"
	*       or "-----BEGIN RSA PRIVATE KEY-----".
	*/
	QSsl::EncodingFormat privateKeyFormat = QSsl::Pem;

	/**
	* @brief Passphrase for the private key.
	*
	* If the private key file is encrypted (password-protected), provide
	* the decryption passphrase here. Leave empty if the key is not encrypted.
	*
	* @note Using encrypted keys adds security but requires the passphrase
	*       to be available at runtime, which presents its own security
	*       challenges.
	*
	* @warning Never hard-code passphrases in source code. Load them from
	*          secure configuration files or environment variables.
	*/
	QByteArray privateKeyPassPhrase;

	/**
	* @brief List of paths to trusted CA certificates.
	*
	* These Certificate Authority (CA) certificates are used to verify
	* client certificates when peer verification is enabled. Each path
	* should point to a file containing one or more CA certificates.
	*
	* Common uses:
	* - Verify client certificates in mutual TLS (mTLS) scenarios
	* - Establish chain of trust for certificate validation
	* - Support multiple CAs for different client types
	*
	* @note This is only required if peer verification is enabled via
	*       verifyMode. For server-only authentication, this can be empty.
	*/
	QList<QString> caCertificatePaths;

	/**
	* @brief Encoding format of CA certificates.
	*
	* Specifies whether CA certificate files are in PEM or DER format.
	* Default is PEM.
	*
	* @note PEM files can contain multiple certificates concatenated together.
	*/
	QSsl::EncodingFormat caCertificateFormat = QSsl::Pem;

	/**
	* @brief Peer verification mode.
	*
	* Controls whether and how client certificates are verified:
	* - VerifyNone: No client verification (server auth only)
	* - VerifyPeer: Verify client certificates if provided
	* - QueryPeer: Request but don't require client certificates
	* - AutoVerifyPeer: Automatic verification based on SSL settings
	*
	* Default is AutoVerifyPeer.
	*
	* @note For mutual TLS (mTLS) where clients must authenticate,
	*       use VerifyPeer and configure caCertificatePaths.
	*
	* @warning VerifyNone is less secure as it doesn't authenticate clients.
	*          Only use when client authentication is not required.
	*/
	QSslSocket::PeerVerifyMode verifyMode =
		QSslSocket::PeerVerifyMode::AutoVerifyPeer;

	/**
	* @brief SSL/TLS protocol version.
	*
	* Defines the minimum or exact TLS protocol version to be used.
	* Recommended values:
	* - QSsl::TlsV1_2: Minimum for production (widely supported)
	* - QSsl::TlsV1_3: Latest protocol with improved security and performance
	*
	* Default is TLS 1.2.
	*
	* @warning Older protocols (SSLv2, SSLv3, TLS 1.0, TLS 1.1) have known
	*          security vulnerabilities and should NOT be used. Many
	*          compliance standards (PCI DSS, etc.) require TLS 1.2 or higher.
	*
	* @note TLS 1.3 provides better security and performance but may not
	*       be supported by older clients.
	*/
	QSsl::SslProtocol protocol = QSsl::TlsV1_2;
};


/**
* @brief General server configuration.
*
* This structure defines network-level parameters for the server,
* including ports, host binding, and optional SSL configuration.
*
* The server supports two protocols:
* - **HTTP**: RESTful API requests
* - **WebSocket**: Bidirectional real-time communication
*
* Each protocol runs on its own port and can be independently
* configured for SSL/TLS encryption.
*
* @see SslConfig
*/
struct ServerConfig
{
	/**
	* @brief HTTP port number.
	*
	* Port used for HTTP/HTTPS REST API communication.
	* 
	* Common values:
	* - 80: Standard HTTP (requires root/admin privileges on Unix)
	* - 443: Standard HTTPS (requires root/admin privileges on Unix)
	* - 8080: Common alternative for HTTP
	* - 8443: Common alternative for HTTPS
	*
	* Default is 80. Choose a port > 1024 to avoid requiring elevated
	* privileges.
	*
	* @note If SSL is configured, this becomes an HTTPS port.
	*/
	int httpPort = 80;

	/**
	* @brief WebSocket port number.
	*
	* Port used for WebSocket/WSS bidirectional communication.
	*
	* Common values:
	* - 8080-8090: Common WebSocket ports
	* - 8443: Common secure WebSocket port
	*
	* Default is 90.
	*
	* @note If SSL is configured, this becomes a WSS (secure WebSocket) port.
	*       WebSocket connections are used for real-time notifications and
	*       bidirectional communication between server and clients.
	*/
	int wsPort = 90;

	/**
	* @brief Host address or interface to bind to.
	*
	* Specifies which network interface the server listens on:
	* - "localhost" or "127.0.0.1": Only local connections (default)
	* - "0.0.0.0": All IPv4 interfaces (public and local)
	* - "::": All IPv6 interfaces
	* - Specific IP: Only that interface (e.g., "192.168.1.100")
	*
	* @note Using "0.0.0.0" or "::" makes the server accessible from
	*       other machines on the network. Ensure proper firewall
	*       configuration and SSL when doing this.
	*/
	QString host = "localhost";

	/**
	* @brief Optional SSL configuration.
	*
	* If set, enables HTTPS and WSS (secure WebSocket) connections with
	* the specified SSL/TLS settings. If not set, the server operates
	* in non-secure mode using plain HTTP and WebSocket.
	*
	* @warning Always configure SSL for production deployments to protect:
	*          - User credentials during authentication
	*          - Access tokens and session data
	*          - Sensitive authorization information
	*          - Administrative operations
	*
	* @note SSL setup requires valid certificates. For testing, you can
	*       use self-signed certificates, but production should use
	*       certificates from a trusted CA.
	*
	* @see SslConfig
	*/
	std::optional<SslConfig> sslConfig;
};


/**
* @brief Main Authorizable Server interface.
*
* This class provides a high-level API for controlling the lifecycle
* and configuration of the authorizable server. It allows embedding
* a full-featured authorization server within your application.
*
* The server provides:
* - User authentication (local and LDAP)
* - Role-based access control
* - Permission management
* - User and group administration
* - Product licensing and feature management
* - Integration with Puma backend
*
* @section implementation_details Implementation
*
* Internally uses the PIMPL (Pointer to Implementation) pattern to:
* - Hide implementation details from the public API
* - Ensure ABI (Application Binary Interface) stability
* - Allow library updates without requiring client recompilation
* - Separate interface from platform-specific implementation
*
* @section lifecycle Server Lifecycle
*
* 1. **Create**: Instantiate CAuthorizableServer
* 2. **Configure**: Set product ID, features, and connection parameters
* 3. **Start**: Call Start() with ServerConfig
* 4. **Run**: Server accepts connections and processes requests
* 5. **Stop**: Call Stop() to gracefully shutdown
* 6. **Destroy**: Delete the server instance
*
* @section example Usage Example
* @code
* CAuthorizableServer server;
* 
* // Configure before starting
* server.SetProductId("MyProduct");
* server.SetFeaturesFilePath("/etc/myapp/features.xml");
* 
* ServerConfig config;
* config.host = "0.0.0.0";
* config.httpPort = 8443;
* config.wsPort = 8444;
* 
* // Configure SSL
* SslConfig ssl;
* ssl.localCertificatePath = "/etc/ssl/server.crt";
* ssl.privateKeyPath = "/etc/ssl/server.key";
* config.sslConfig = ssl;
* 
* // Start server
* if (server.Start(config)) {
*     // Server is running
*     QCoreApplication::exec();  // Keep running
*     server.Stop();
* }
* @endcode
*
* @section thread_safety Thread Safety
*
* - Configuration methods (SetProductId, SetFeaturesFilePath) should
*   be called before Start() or after Stop()
* - Start() and Stop() should not be called concurrently
* - Once started, the server is thread-safe for request handling
*
* @see ServerConfig, SslConfig
*/
class AUTH_SERVER_SDK_EXPORT CAuthorizableServer
{
public:
	/**
	* @brief Constructs the server instance.
	*
	* Creates a new authorizable server in the stopped state. The server
	* is not started automatically and will not accept connections until
	* Start() is called.
	*
	* @note Configuration methods should be called after construction
	*       and before starting the server.
	*
	* @see Start(), SetProductId(), SetFeaturesFilePath()
	*/
	CAuthorizableServer();

	/**
	* @brief Virtual destructor.
	*
	* Destroys the server instance and releases all associated resources.
	* If the server is still running, it is automatically stopped before
	* destruction.
	*
	* @note For graceful shutdown, it's recommended to explicitly call
	*       Stop() before destroying the server to ensure all connections
	*       are properly closed and resources are released.
	*/
	virtual ~CAuthorizableServer();

	/**
	* @brief Starts the server.
	*
	* Initializes network listeners on the configured ports and starts
	* accepting client connections. Both HTTP and WebSocket servers are
	* started according to the provided configuration.
	*
	* The server will:
	* 1. Apply connection parameters (host, ports)
	* 2. Configure SSL/TLS if specified
	* 3. Start HTTP server on the specified port
	* 4. Start WebSocket server on the specified port
	* 5. Begin accepting and processing client requests
	*
	* @param serverConfig Server configuration including network settings
	*                     and optional SSL configuration.
	*
	* @return true if the server was successfully started and is now running.
	* @return false if startup failed due to:
	*         - Port already in use
	*         - Invalid SSL configuration
	*         - Missing or invalid certificate files
	*         - Insufficient permissions (e.g., binding to ports < 1024)
	*         - Required interface not available
	*
	* @note This method blocks briefly during initialization but returns
	*       once the server is running. The server then handles requests
	*       asynchronously.
	*
	* @warning Ensure SetProductId() and SetFeaturesFilePath() are called
	*          before Start() if required by your application.
	*
	* @see Stop(), ServerConfig, SslConfig
	*
	* @par Example:
	* @code
	* ServerConfig config;
	* config.host = "0.0.0.0";
	* config.httpPort = 8080;
	* config.wsPort = 8081;
	*
	* if (!server.Start(config)) {
	*     qCritical() << "Failed to start server";
	*     return -1;
	* }
	* qDebug() << "Server is running";
	* @endcode
	*/
	virtual bool Start(const ServerConfig& serverConfig) const;

	/**
	* @brief Stops the server.
	*
	* Gracefully shuts down all network listeners and closes active
	* connections. Both HTTP and WebSocket servers are stopped.
	*
	* The shutdown process:
	* 1. Stops accepting new connections
	* 2. Closes HTTP server
	* 3. Closes WebSocket server  
	* 4. Waits for active requests to complete (with timeout)
	* 5. Releases network resources
	*
	* @return true if the server was successfully stopped.
	* @return false if stopping failed (server not running, error during
	*         shutdown, etc.).
	*
	* @note After stopping, the server can be restarted by calling Start()
	*       again with new or the same configuration.
	*
	* @note Active connections may be forcefully closed if they don't
	*       complete within the shutdown timeout period.
	*
	* @see Start()
	*/
	virtual bool Stop() const;

	/**
	* @brief Sets the path to the features configuration file.
	*
	* Loads a features configuration file that defines enabled features,
	* licenses, authorization rules, or product capabilities. The file
	* format is typically XML containing product feature definitions.
	*
	* @param filePath Absolute or relative path to the features configuration
	*                 file (e.g., "/etc/myapp/features.xml").
	*
	* @return true if the file was found, loaded, and parsed successfully.
	* @return false if:
	*         - File doesn't exist or cannot be read
	*         - File format is invalid
	*         - Required interface is unavailable
	*
	* @note This should typically be called before Start() to ensure
	*       features are properly configured when the server starts.
	*
	* @note The file is parsed and its contents are applied to the
	*       product info interface immediately.
	*
	* @see SetProductId()
	*/
	virtual bool SetFeaturesFilePath(const QString& filePath) const;

	/**
	* @brief Configures Puma connection parameters.
	*
	* Sets connection parameters for integration with the Puma backend
	* or service. This allows the authorization server to communicate
	* with Puma for extended functionality or data synchronization.
	*
	* @param serverConfig Puma-related server configuration including
	*                     host, ports, and optional SSL settings.
	*
	* @return true if the parameters were applied successfully.
	* @return false if configuration failed (required interface unavailable).
	*
	* @note This is optional and only needed if your deployment includes
	*       Puma backend integration. The configuration uses the same
	*       ServerConfig structure as the main server but applies to
	*       outbound connections to Puma.
	*
	* @see SetConnectionParam(), ServerConfig
	*/
	virtual bool SetPumaConnectionParam(const ServerConfig& serverConfig) const;

	/**
	* @brief Sets the product identifier.
	*
	* Configures the product ID used for authorization, licensing,
	* and telemetry purposes. The product ID is an application-specific
	* identifier that scopes authorization rules and features.
	*
	* @param productId Binary product identifier (e.g., "MyAuthServer",
	*                  "ProductX"). Should be a unique, stable identifier
	*                  for your application.
	*
	* @return true if the product ID was set successfully.
	* @return false if setting failed (required interface unavailable).
	*
	* @note This should typically be called during initialization before
	*       Start(). The product ID affects which features and authorization
	*       rules apply.
	*
	* @see SetFeaturesFilePath()
	*/
	virtual bool SetProductId(const QByteArray& productId) const;

private:
	/**
	* @brief Pointer to the internal implementation.
	*
	* This is part of the PIMPL (Pointer to Implementation) pattern,
	* which separates the interface from the implementation. Benefits:
	* - Hides implementation details from clients
	* - Preserves ABI compatibility across library versions
	* - Reduces compilation dependencies
	* - Allows platform-specific implementations
	*
	* @note Clients should never directly access or modify this pointer.
	*       All interaction should be through the public API methods.
	*/
	CAuthorizableServerImpl* m_implPtr;
};

} // namespace AuthServerSdk


/**
* @defgroup AuthServerSdk Authorizable Server SDK
*
* @brief Public API for the Authorizable Server.
*
* The Authorizable Server SDK provides a C++ interface for embedding
* and controlling a full-featured authorization server within your
* application. It supports:
*
* - **Authentication**: Local and LDAP-based user authentication
* - **Authorization**: Role-based access control (RBAC)
* - **User Management**: User, role, and group administration
* - **Secure Communication**: SSL/TLS encryption for HTTP and WebSocket
* - **Product Licensing**: Feature and license management
* - **Backend Integration**: Puma backend connectivity
*
* The server implements modern authorization patterns including:
* - JWT token-based authentication
* - Permission-based authorization
* - Hierarchical role and group structures
* - Product-scoped authorization rules
*
* @section architecture Architecture
*
* The SDK uses a layered architecture:
* - **Public API**: CAuthorizableServer class (this SDK)
* - **ACF Framework**: Component-based architecture
* - **ImtCore**: Core services (auth, database, networking)
* - **Network Layer**: HTTP and WebSocket protocols
* - **Storage**: Database backend for users, roles, permissions
*
* @section use_cases Use Cases
*
* - Embedding authorization in desktop applications
* - Building custom authentication servers
* - Implementing product licensing systems
* - Creating multi-tenant authorization services
* - Integrating with existing applications
*
* @see CAuthorizableServer, ServerConfig, SslConfig
*/

/**
* @mainpage Authorizable Server SDK Documentation
*
* @section intro Introduction
*
* The Authorizable Server SDK provides a comprehensive C++ interface for
* embedding and controlling an authorization server within your application.
* It delivers enterprise-grade authentication and authorization capabilities
* with minimal integration effort.
*
* @section features Key Features
*
* - **Multi-Protocol Support**: HTTP/HTTPS and WebSocket/WSS
* - **Flexible Authentication**: Local database and LDAP integration
* - **Fine-Grained Authorization**: Permission-based access control
* - **Enterprise Security**: SSL/TLS with configurable cipher suites
* - **User Management**: Complete CRUD operations for users, roles, and groups
* - **Product Licensing**: Feature enablement and license management
* - **High Performance**: Async I/O and efficient request handling
* - **ABI Stable**: Binary compatibility across versions via PIMPL pattern
*
* @section getting_started Getting Started
*
* @subsection step1 Step 1: Create Server Instance
* @code
* using namespace AuthServerSdk;
* CAuthorizableServer server;
* @endcode
*
* @subsection step2 Step 2: Configure Server
* @code
* server.SetProductId("MyApplication");
* server.SetFeaturesFilePath("/etc/myapp/features.xml");
* @endcode
*
* @subsection step3 Step 3: Configure Network and SSL
* @code
* ServerConfig config;
* config.host = "0.0.0.0";
* config.httpPort = 8443;
* config.wsPort = 8444;
*
* SslConfig ssl;
* ssl.localCertificatePath = "/etc/ssl/server.crt";
* ssl.privateKeyPath = "/etc/ssl/server.key";
* ssl.caCertificatePaths = {"/etc/ssl/ca.pem"};
* ssl.protocol = QSsl::TlsV1_3;
* config.sslConfig = ssl;
* @endcode
*
* @subsection step4 Step 4: Start Server
* @code
* if (server.Start(config)) {
*     qDebug() << "Server running on port" << config.httpPort;
* } else {
*     qCritical() << "Failed to start server";
* }
* @endcode
*
* @section security Security Best Practices
*
* - Always use SSL/TLS in production (TLS 1.2 or higher)
* - Protect private keys with appropriate file permissions (chmod 600)
* - Use strong passwords/passphrases for encrypted keys
* - Enable client certificate verification for sensitive deployments
* - Keep certificates up to date and rotate regularly
* - Use secure credential storage (never hard-code secrets)
* - Configure appropriate firewall rules
* - Monitor and log security events
*
* @section platform Platform Support
*
* - Windows (Win32, Win64)
* - Requires Qt framework
* - ACF (Adaptive Component Framework) infrastructure
* - ImtCore libraries
*
* @section api_reference API Reference
*
* - CAuthorizableServer - Main server control class
* - ServerConfig - Network configuration
* - SslConfig - SSL/TLS security settings
*
* @see AuthServerSdk
*/
