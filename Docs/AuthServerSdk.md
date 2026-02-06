# AuthServerSdk Documentation

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Getting Started](#getting-started)
- [API Reference](#api-reference)
- [Configuration](#configuration)
- [Usage Examples](#usage-examples)
- [SSL/TLS Configuration](#ssltls-configuration)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)

## Overview

The AuthServerSdk (Authorization Server SDK) is a C++ library that provides a high-level API for embedding an authorization server into your applications. It enables applications to implement robust authentication and authorization mechanisms with optional SSL/TLS support, WebSocket communication, and flexible configuration options.

The SDK is designed for:
- Applications that need to provide authentication services to multiple clients
- Enterprise systems requiring centralized authorization management
- Secure server implementations with SSL/TLS encryption
- Systems requiring WebSocket-based real-time communication

### Key Characteristics
- **PIMPL Pattern**: Uses the Pointer to Implementation (PIMPL) design pattern for ABI stability
- **Platform Support**: Windows-only (WIN32 conditional compilation)
- **Qt-based**: Built on Qt framework for cross-platform compatibility within Windows
- **ImtCore Integration**: Leverages ImtCore component framework for modular architecture

## Features

### Core Capabilities
- **Dual Protocol Support**: HTTP and WebSocket server capabilities
- **SSL/TLS Security**: Comprehensive SSL/TLS configuration with certificate management
- **Certificate Management**: Support for server certificates, CA certificates, and private keys
- **Flexible Configuration**: Runtime configuration of ports, hosts, and security settings
- **Product Licensing**: Integration with product ID and features file management
- **Connection Management**: Separate configuration for server and Puma backend connections

### Security Features
- **SSL/TLS 1.2+**: Modern TLS protocol support
- **Certificate Verification**: Configurable peer verification modes
- **Multiple CA Support**: Load multiple trusted Certificate Authority certificates
- **Key Encryption**: Support for encrypted private keys with passphrase protection
- **PEM/DER Support**: Multiple certificate encoding format support

## Architecture

### Component Structure

```
┌─────────────────────────────────────┐
│   CAuthorizableServer (Public API)  │
├─────────────────────────────────────┤
│   CAuthorizableServerImpl (PIMPL)   │
├─────────────────────────────────────┤
│      CAuthServerSdk (Component)     │
├─────────────────────────────────────┤
│         ImtCore Framework           │
│  ┌──────────────────────────────┐   │
│  │ IServerDispatcher            │   │
│  │ IServerConnectionInterface   │   │
│  │ ISslConfigurationApplier     │   │
│  │ IApplicationInfoController   │   │
│  │ IProductInfo                 │   │
│  └──────────────────────────────┘   │
└─────────────────────────────────────┘
```

### Dependencies
- **Qt Core**: QString, QByteArray, QFile
- **Qt Network**: QSslConfiguration, QSslSocket, QSsl
- **ACF (Adaptive Component Framework)**: Component lifecycle management
- **ImtCore**: Application info, licensing, server communication, SSL configuration

## Getting Started

### Prerequisites
- Qt 5 or Qt 6
- C++ compiler with C++17 support
- OpenSSL libraries
- ImtCore framework installed
- Windows platform (WIN32)

### Basic Integration

1. **Include the SDK header**:
```cpp
#include <AuthServerSdk/AuthServerSdk.h>
```

2. **Link against the SDK library**:
   - Add `AuthServerSdk` to your project's linked libraries
   - Ensure Qt modules are linked: Core, Network

3. **Initialize and start the server**:
```cpp
using namespace AuthServerSdk;

// Create server instance
CAuthorizableServer server;

// Configure server
ServerConfig config;
config.httpPort = 8080;
config.wsPort = 8090;
config.host = "localhost";

// Start the server
if (server.Start(config)) {
    qDebug() << "Server started successfully";
} else {
    qCritical() << "Failed to start server";
}
```

## API Reference

### Main Class: `CAuthorizableServer`

#### Constructor
```cpp
CAuthorizableServer();
```
Creates a new server instance. The server is not started automatically.

#### Destructor
```cpp
virtual ~CAuthorizableServer();
```
Stops the server if running and releases all resources.

#### `Start()`
```cpp
virtual bool Start(const ServerConfig& serverConfig) const;
```
Starts the authorization server with the provided configuration.

**Parameters:**
- `serverConfig`: Server configuration including ports, host, and optional SSL settings

**Returns:**
- `true` if server started successfully
- `false` on failure (check logs for details)

**Behavior:**
1. Validates and applies connection parameters
2. Configures SSL/TLS if provided
3. Starts HTTP server
4. Starts WebSocket server

#### `Stop()`
```cpp
virtual bool Stop() const;
```
Gracefully stops all server listeners and active connections.

**Returns:**
- `true` if server stopped successfully
- `false` on failure

#### `SetFeaturesFilePath()`
```cpp
virtual bool SetFeaturesFilePath(const QString& filePath) const;
```
Sets the path to the features configuration file for licensing and feature management.

**Parameters:**
- `filePath`: Absolute or relative path to the features XML file

**Returns:**
- `true` if file was loaded and parsed successfully
- `false` if file not found or parsing failed

**Features File Format:**
The file should be a CompactXML format containing product feature definitions.

#### `SetPumaConnectionParam()`
```cpp
virtual bool SetPumaConnectionParam(const ServerConfig& serverConfig) const;
```
Configures connection parameters for Puma backend integration.

**Parameters:**
- `serverConfig`: Configuration for Puma connection

**Returns:**
- `true` if parameters applied successfully
- `false` if Puma connection interface unavailable

#### `SetProductId()`
```cpp
virtual bool SetProductId(const QByteArray& productId) const;
```
Sets the product identifier for licensing and authorization purposes.

**Parameters:**
- `productId`: Binary product identifier

**Returns:**
- `true` if product ID set successfully
- `false` if application info controller unavailable

### Configuration Structures

#### `ServerConfig`
```cpp
struct ServerConfig {
    int httpPort = 80;                      // HTTP/HTTPS port
    int wsPort = 90;                        // WebSocket port
    QString host = "localhost";             // Server host/interface
    std::optional<SslConfig> sslConfig;     // Optional SSL configuration
};
```

#### `SslConfig`
```cpp
struct SslConfig {
    QString localCertificatePath;                           // Server certificate path
    QSsl::EncodingFormat localCertificateFormat = QSsl::Pem; // Certificate format
    QString privateKeyPath;                                  // Private key path
    QSsl::KeyAlgorithm privateKeyAlgorithm = QSsl::Rsa;     // Key algorithm
    QSsl::EncodingFormat privateKeyFormat = QSsl::Pem;      // Key format
    QByteArray privateKeyPassPhrase;                        // Key passphrase (if encrypted)
    QList<QString> caCertificatePaths;                      // Trusted CA certificates
    QSsl::EncodingFormat caCertificateFormat = QSsl::Pem;   // CA cert format
    QSslSocket::PeerVerifyMode verifyMode = 
        QSslSocket::PeerVerifyMode::AutoVerifyPeer;         // Peer verification
    QSsl::SslProtocol protocol = QSsl::TlsV1_2;             // TLS protocol version
};
```

## Configuration

### Basic HTTP Configuration
```cpp
ServerConfig config;
config.httpPort = 8080;
config.wsPort = 8090;
config.host = "0.0.0.0";  // Listen on all interfaces
```

### SSL/TLS Configuration
```cpp
ServerConfig config;
config.httpPort = 8443;  // HTTPS
config.wsPort = 8453;    // Secure WebSocket
config.host = "0.0.0.0";

// Configure SSL
SslConfig sslConfig;
sslConfig.localCertificatePath = "/path/to/server.crt";
sslConfig.localCertificateFormat = QSsl::Pem;
sslConfig.privateKeyPath = "/path/to/server.key";
sslConfig.privateKeyAlgorithm = QSsl::Rsa;
sslConfig.privateKeyFormat = QSsl::Pem;
sslConfig.privateKeyPassPhrase = "your-passphrase";  // Optional
sslConfig.protocol = QSsl::TlsV1_2;
sslConfig.verifyMode = QSslSocket::VerifyPeer;

// Add trusted CA certificates
sslConfig.caCertificatePaths.append("/path/to/ca.crt");

config.sslConfig = sslConfig;
```

### Port Configuration Guidelines
- **HTTP Port**: 
  - Default: 80 (non-SSL) or 443 (SSL)
  - Development: 8080-8999
  - Production: Use standard ports or registered high ports (1024-49151)

- **WebSocket Port**: 
  - Typically offset from HTTP port (e.g., HTTP:8080, WS:8090)
  - Avoid conflicts with other services

## Usage Examples

### Example 1: Basic Server
```cpp
#include <AuthServerSdk/AuthServerSdk.h>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    using namespace AuthServerSdk;
    
    // Create and configure server
    CAuthorizableServer server;
    
    ServerConfig config;
    config.httpPort = 8080;
    config.wsPort = 8090;
    config.host = "localhost";
    
    // Set product ID
    server.SetProductId("MyProduct-ID-12345");
    
    // Start server
    if (!server.Start(config)) {
        qCritical() << "Failed to start server";
        return 1;
    }
    
    qInfo() << "Authorization server running";
    qInfo() << "HTTP Port:" << config.httpPort;
    qInfo() << "WebSocket Port:" << config.wsPort;
    
    return app.exec();
}
```

### Example 2: Secure Server with SSL/TLS
```cpp
#include <AuthServerSdk/AuthServerSdk.h>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    using namespace AuthServerSdk;
    
    CAuthorizableServer server;
    
    // Configure SSL
    SslConfig sslConfig;
    sslConfig.localCertificatePath = "certs/server.crt";
    sslConfig.privateKeyPath = "certs/server.key";
    sslConfig.caCertificatePaths.append("certs/ca.crt");
    sslConfig.protocol = QSsl::TlsV1_2;
    sslConfig.verifyMode = QSslSocket::VerifyPeer;
    
    // Configure server with SSL
    ServerConfig config;
    config.httpPort = 8443;
    config.wsPort = 8453;
    config.host = "0.0.0.0";
    config.sslConfig = sslConfig;
    
    // Start server
    if (!server.Start(config)) {
        qCritical() << "Failed to start secure server";
        return 1;
    }
    
    qInfo() << "Secure authorization server running";
    
    return app.exec();
}
```

### Example 3: Server with Features File
```cpp
#include <AuthServerSdk/AuthServerSdk.h>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    using namespace AuthServerSdk;
    
    CAuthorizableServer server;
    
    // Set product configuration
    server.SetProductId("MyProduct-v1.0");
    
    // Load features file
    if (!server.SetFeaturesFilePath("config/features.xml")) {
        qWarning() << "Failed to load features file";
    }
    
    // Configure and start
    ServerConfig config;
    config.httpPort = 8080;
    config.wsPort = 8090;
    
    if (!server.Start(config)) {
        qCritical() << "Failed to start server";
        return 1;
    }
    
    return app.exec();
}
```

### Example 4: Server with Puma Backend Integration
```cpp
#include <AuthServerSdk/AuthServerSdk.h>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    using namespace AuthServerSdk;
    
    CAuthorizableServer server;
    
    // Configure Puma backend connection
    ServerConfig pumaConfig;
    pumaConfig.httpPort = 9080;
    pumaConfig.wsPort = 9090;
    pumaConfig.host = "puma-backend.local";
    
    if (!server.SetPumaConnectionParam(pumaConfig)) {
        qWarning() << "Failed to configure Puma connection";
    }
    
    // Configure main server
    ServerConfig config;
    config.httpPort = 8080;
    config.wsPort = 8090;
    config.host = "0.0.0.0";
    
    if (!server.Start(config)) {
        qCritical() << "Failed to start server";
        return 1;
    }
    
    return app.exec();
}
```

## SSL/TLS Configuration

### Certificate Requirements

#### Server Certificate
- **Purpose**: Identifies the server to clients
- **Format**: PEM or DER
- **Requirements**: 
  - Must be valid (not expired)
  - Common Name (CN) should match server hostname
  - Should be signed by a trusted CA (or self-signed for testing)

#### Private Key
- **Purpose**: Server's private key for SSL/TLS encryption
- **Format**: PEM or DER
- **Algorithm**: RSA (recommended) or EC
- **Key Size**: Minimum 2048 bits for RSA
- **Encryption**: Can be encrypted with passphrase

#### CA Certificates
- **Purpose**: Verify client certificates (when peer verification enabled)
- **Format**: PEM or DER
- **Usage**: Multiple CA certificates can be loaded

### Generating Self-Signed Certificates (Testing Only)

```bash
# Generate private key
openssl genrsa -out server.key 2048

# Generate certificate signing request
openssl req -new -key server.key -out server.csr

# Generate self-signed certificate (valid for 365 days)
openssl x509 -req -days 365 -in server.csr -signkey server.key -out server.crt
```

### Peer Verification Modes

- **`AutoVerifyPeer`**: Automatically verify peer certificates (default)
- **`VerifyPeer`**: Always verify peer certificates
- **`VerifyNone`**: Do not verify peer certificates (insecure, testing only)
- **`QueryPeer`**: Query but don't require verification

### TLS Protocol Versions

Supported protocols (in order of security):
1. `QSsl::TlsV1_3` - TLS 1.3 (most secure, recommended)
2. `QSsl::TlsV1_2` - TLS 1.2 (default, widely compatible)
3. `QSsl::TlsV1_1` - TLS 1.1 (deprecated, avoid)
4. `QSsl::TlsV1_0` - TLS 1.0 (deprecated, avoid)

**Recommendation**: Use TLS 1.2 minimum, TLS 1.3 when possible.

## Best Practices

### Security
1. **Always use SSL/TLS in production environments**
2. **Use strong certificates**: Minimum 2048-bit RSA or 256-bit EC keys
3. **Keep private keys secure**: Never commit to version control, use restricted file permissions
4. **Use certificate verification**: Enable peer verification with trusted CAs
5. **Rotate certificates**: Implement certificate renewal before expiration
6. **Use encrypted private keys**: Protect keys with strong passphrases
7. **Disable weak protocols**: Avoid TLS 1.0 and 1.1

### Configuration
1. **Bind to specific interfaces**: Use specific IPs instead of "0.0.0.0" in production
2. **Use non-standard ports**: Avoid default ports (80, 443) when possible
3. **Separate environments**: Different ports/hosts for development, staging, production
4. **Validate configuration**: Check return values of all configuration methods
5. **Load features files**: Use features files for flexible licensing

### Resource Management
1. **Proper cleanup**: Always stop the server before destruction
2. **Handle errors**: Check return values and log errors
3. **Connection limits**: Monitor and limit concurrent connections
4. **Graceful shutdown**: Stop server cleanly during application exit

### Logging
1. **Enable Qt logging**: Use qDebug, qInfo, qWarning, qCritical
2. **Log configuration**: Log server start parameters (ports, host)
3. **Log SSL operations**: Monitor certificate loading and SSL errors
4. **Don't log secrets**: Never log private keys or passphrases

## Troubleshooting

### Server Fails to Start

**Problem**: `Start()` returns false

**Possible Causes:**
1. Port already in use
2. Insufficient permissions (ports < 1024)
3. Invalid SSL configuration
4. Missing dependencies

**Solutions:**
```cpp
// Check if port is available
// Use different ports or kill process using the port

// Ensure proper permissions
// Run as administrator or use high ports (>1024)

// Validate SSL files exist
QFile certFile(sslConfig.localCertificatePath);
if (!certFile.exists()) {
    qWarning() << "Certificate file not found";
}
```

### SSL Certificate Loading Fails

**Problem**: Certificate or key file cannot be loaded

**Possible Causes:**
1. File path incorrect
2. File format mismatch
3. Encrypted key without passphrase
4. Corrupt certificate/key

**Solutions:**
```cpp
// Verify file paths are absolute or relative to working directory
qDebug() << "Current dir:" << QDir::currentPath();
qDebug() << "Cert path:" << QDir(sslConfig.localCertificatePath).absolutePath();

// Check file format matches configuration
// PEM files start with: -----BEGIN CERTIFICATE-----
// DER files are binary

// Provide passphrase for encrypted keys
sslConfig.privateKeyPassPhrase = "your-passphrase";
```

### Connection Refused

**Problem**: Clients cannot connect to server

**Possible Causes:**
1. Firewall blocking ports
2. Server bound to localhost only
3. Wrong IP/port configuration
4. SSL handshake failures

**Solutions:**
```cpp
// Bind to all interfaces for external access
config.host = "0.0.0.0";  // IPv4 all interfaces
// or
config.host = "::";       // IPv6 all interfaces

// Check firewall rules
// Windows: netsh advfirewall firewall add rule ...
// Linux: sudo ufw allow 8080/tcp
```

### Interface Not Found Errors

**Problem**: Warnings about missing interfaces (IServerDispatcher, etc.)

**Possible Causes:**
1. ImtCore components not properly initialized
2. Missing component dependencies
3. ACF configuration issues

**Solutions:**
1. Ensure ImtCore framework is properly installed
2. Check component initialization in ACF configuration files
3. Verify all required libraries are linked
4. Check SDK component creation: `m_sdk.EnsureAutoInitComponentsCreated()`

### Memory Issues

**Problem**: Memory leaks or crashes

**Solutions:**
1. Always call `Stop()` before server destruction
2. Ensure PIMPL cleanup in destructor
3. Check for proper Qt object ownership
4. Use Qt memory profiling tools

## Performance Considerations

### Optimization Tips
1. **Connection pooling**: Reuse connections when possible
2. **Buffer sizes**: Tune TCP buffer sizes for high throughput
3. **Thread configuration**: Configure Qt's thread pool appropriately
4. **Keep-alive**: Enable HTTP keep-alive for better performance
5. **Compression**: Consider enabling compression for large payloads

### Monitoring
1. Monitor active connection count
2. Track request/response times
3. Monitor SSL handshake times
4. Log resource usage (memory, CPU)

## Integration with ImtCore

The SDK integrates with several ImtCore interfaces:

- **`imtcom::IServerDispatcher`**: Server lifecycle management
- **`imtcom::IServerConnectionInterface`**: Connection parameters
- **`imtcom::ISslConfigurationApplier`**: SSL/TLS configuration
- **`imtbase::IApplicationInfoController`**: Application metadata
- **`imtlic::IProductInfo`**: Product licensing

These interfaces are accessed through the SDK's component framework.

## Platform Notes

### Windows Support
- Primary platform for AuthServerSdk
- WIN32 preprocessor conditional compilation
- Tested with Visual C++ 2015/2017/2019

### Build Systems
- CMake: `/home/runner/work/Puma/Puma/Impl/AuthServerSdk/CMake`
- QMake: `/home/runner/work/Puma/Puma/Impl/AuthServerSdk/QMake`
- Visual Studio: `/home/runner/work/Puma/Puma/Impl/AuthServerSdk/VC`

## Additional Resources

- **Main README**: [/home/runner/work/Puma/Puma/README.md](../README.md)
- **AuthClientSdk Documentation**: [AuthClientSdk.md](AuthClientSdk.md)
- **Doxygen API**: Generate with `doxygen Doxyfile` in Docs directory
- **ImtCore Documentation**: Refer to ImtCore framework documentation
- **Qt Documentation**: https://doc.qt.io/

## Version History

See Git commit history for detailed changes to the SDK.

## License

See the repository license file for licensing information.
