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

**Important**: The AuthServerSdk implementation is based internally on communication with the Puma server. The SDK acts as a wrapper that configures and manages the connection to the underlying Puma authorization infrastructure, providing a simplified API for embedding authorization capabilities into host applications.

The SDK is designed for:
- Applications that need to provide authentication services to multiple clients
- Enterprise systems requiring centralized authorization management
- Secure server implementations with SSL/TLS encryption
- Systems requiring WebSocket-based real-time communication
- Applications that need to integrate with the Puma authorization framework

### Key Characteristics
- **PIMPL Pattern**: Uses the Pointer to Implementation (PIMPL) design pattern for ABI stability
- **Platform Support**: Windows-only (WIN32 conditional compilation)
- **Qt-based**: Built on Qt framework for cross-platform compatibility within Windows
- **ImtCore Integration**: Leverages ImtCore component framework for modular architecture
- **Puma Integration**: Internally communicates with Puma server infrastructure for authorization services

## Features

### Core Capabilities
- **Dual Protocol Support**: HTTP and WebSocket server capabilities
- **SSL/TLS Security**: Comprehensive SSL/TLS configuration with certificate management
- **Certificate Management**: Support for server certificates, CA certificates, and private keys
- **Flexible Configuration**: Runtime configuration of ports, hosts, and security settings
- **Product Licensing**: Integration with product ID and features file management
- **Puma Backend Integration**: Internal communication with Puma server for authorization processing
- **Connection Management**: Separate configuration for external server endpoints and internal Puma backend connections

### Security Features
- **SSL/TLS 1.2+**: Modern TLS protocol support
- **Certificate Verification**: Configurable peer verification modes
- **Multiple CA Support**: Load multiple trusted Certificate Authority certificates
- **Key Encryption**: Support for encrypted private keys with passphrase protection
- **PEM/DER Support**: Multiple certificate encoding format support

## Architecture

### Component Structure

```
┌──────────────────────────────────────────────────────┐
│            CAuthorizableServer (Public API)          │
├──────────────────────────────────────────────────────┤
│         CAuthorizableServerImpl (PIMPL)              │
├──────────────────────────────────────────────────────┤
│            CAuthServerSdk (Component)                │
├──────────────────────────────────────────────────────┤
│              ImtCore Framework                       │
│  ┌───────────────────────────────────────────────┐   │
│  │ IServerDispatcher                            │   │
│  │ IServerConnectionInterface                   │   │
│  │ ISslConfigurationApplier                     │   │
│  │ IApplicationInfoController                   │   │
│  │ IProductInfo                                 │   │
│  └───────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────┘
                         ↓
                         ↓ (Internal Communication)
                         ↓
┌──────────────────────────────────────────────────────┐
│              Puma Authorization Server               │
│  ┌───────────────────────────────────────────────┐   │
│  │ • User Authentication                        │   │
│  │ • Authorization Processing                   │   │
│  │ • Database Management                        │   │
│  │ • Session Management                         │   │
│  │ • Permission Resolution                      │   │
│  └───────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────┘
```

### Architecture Overview

The AuthServerSdk operates as a **bridge layer** between your application and the Puma authorization server:

1. **Public API Layer** (`CAuthorizableServer`): Provides a clean, stable API for host applications
2. **Implementation Layer** (`CAuthorizableServerImpl`): Handles configuration and communication setup
3. **Component Layer** (`CAuthServerSdk`): Manages ImtCore components and interfaces
4. **ImtCore Framework**: Provides low-level server communication and SSL capabilities
5. **Puma Server** (Backend): Performs actual authentication, authorization, and user management

**Important**: The SDK does not implement authorization logic itself. Instead, it:
- Configures the connection parameters to the Puma server
- Manages SSL/TLS settings for secure communication
- Starts HTTP and WebSocket server endpoints
- Routes requests to the underlying Puma infrastructure
- Manages product licensing and features configuration

### Communication Flow

```
Client Application
       ↓
  HTTP/WebSocket
       ↓
AuthServerSdk (This SDK)
       ↓
  Internal Connection
       ↓
Puma Authorization Server
       ↓
  Database / LDAP
```

### Dependencies
- **Qt Core**: QString, QByteArray, QFile
- **Qt Network**: QSslConfiguration, QSslSocket, QSsl
- **ACF (Adaptive Component Framework)**: Component lifecycle management
- **ImtCore**: Application info, licensing, server communication, SSL configuration
- **Puma Server**: Backend authorization infrastructure (required for operation)

## Getting Started

### Prerequisites

- Qt 5 or Qt 6
- C++ compiler with C++17 support
- OpenSSL libraries
- ImtCore framework installed
- **Puma Authorization Server** - The SDK requires a running Puma server instance for backend operations
- Windows platform (WIN32)

### Deployment Models

The AuthServerSdk can be deployed in two primary configurations:

1. **Embedded Mode**: Your application embeds the SDK and connects to a separate Puma server instance
   - SDK runs in your application process
   - Communicates with external Puma server
   - Best for distributed architectures

2. **Integrated Mode**: Your application and Puma server run in the same infrastructure
   - SDK manages connection to local Puma server
   - Simplified deployment
   - Best for standalone applications

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
Configures connection parameters for the backend Puma authorization server.

**Important**: This method configures the **internal connection** from the SDK to the Puma backend server, which handles the actual authorization processing. This is different from the external server configuration set via `Start()`.

**Parameters:**
- `serverConfig`: Configuration for the Puma backend connection (host, ports, SSL)

**Returns:**
- `true` if parameters applied successfully
- `false` if Puma connection interface unavailable

**Usage Scenario**:
- When your AuthServerSdk instance needs to connect to a separate Puma server
- Use this to specify where the Puma authorization backend is running
- Typically called before `Start()` to establish the backend connection

**Example**:
```cpp
// Configure connection to Puma backend server
ServerConfig pumaBackend;
pumaBackend.host = "puma-backend.internal";
pumaBackend.httpPort = 9080;
pumaBackend.wsPort = 9090;
server.SetPumaConnectionParam(pumaBackend);

// Then configure and start the external-facing server
ServerConfig externalServer;
externalServer.host = "0.0.0.0";
externalServer.httpPort = 8080;
externalServer.wsPort = 8090;
server.Start(externalServer);
```

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

### Understanding the Dual Configuration Model

The AuthServerSdk uses a **dual configuration model** because it operates as a bridge between external clients and the internal Puma authorization server:

1. **External Server Configuration** (via `Start()`):
   - Defines the ports and host that **external clients** connect to
   - This is your application's public-facing authorization endpoint
   - Clients (AuthClientSdk instances) connect to these endpoints
   - Example: `httpPort = 8080, wsPort = 8090, host = "0.0.0.0"`

2. **Puma Backend Configuration** (via `SetPumaConnectionParam()`):
   - Defines where the **Puma authorization server** is running
   - This is the internal connection for authorization processing
   - The SDK forwards requests to this backend
   - Example: `httpPort = 9080, wsPort = 9090, host = "puma-backend.internal"`

**Communication Flow**:
```
Client → [External:8080] → AuthServerSdk → [Internal:9080] → Puma Server
```

### Configuration Scenarios

#### Scenario 1: Standalone Deployment (Local Puma)
When Puma server runs on the same machine:

```cpp
ServerConfig config;
config.httpPort = 8080;
config.wsPort = 8090;
config.host = "localhost";

// Puma backend might be on different ports
ServerConfig pumaConfig;
pumaConfig.httpPort = 9080;
pumaConfig.wsPort = 9090;
pumaConfig.host = "localhost";

server.SetPumaConnectionParam(pumaConfig);
server.Start(config);
```

#### Scenario 2: Distributed Deployment (Remote Puma)
When Puma server runs on a different machine:

```cpp
// External facing configuration
ServerConfig externalConfig;
externalConfig.httpPort = 8080;
externalConfig.wsPort = 8090;
externalConfig.host = "0.0.0.0";  // Accept external connections

// Internal Puma backend configuration
ServerConfig pumaConfig;
pumaConfig.httpPort = 9080;
pumaConfig.wsPort = 9090;
pumaConfig.host = "puma.internal.network";  // Internal Puma server

server.SetPumaConnectionParam(pumaConfig);
server.Start(externalConfig);
```

#### Scenario 3: Secure Deployment (SSL on Both Connections)
When both external and internal connections use SSL:

```cpp
// External SSL configuration
SslConfig externalSsl;
externalSsl.localCertificatePath = "certs/external.crt";
externalSsl.privateKeyPath = "certs/external.key";
externalSsl.caCertificatePaths.append("certs/ca.crt");

ServerConfig externalConfig;
externalConfig.httpPort = 8443;
externalConfig.wsPort = 8453;
externalConfig.host = "0.0.0.0";
externalConfig.sslConfig = externalSsl;

// Internal Puma SSL configuration
SslConfig internalSsl;
internalSsl.localCertificatePath = "certs/internal.crt";
internalSsl.privateKeyPath = "certs/internal.key";
internalSsl.caCertificatePaths.append("certs/puma-ca.crt");

ServerConfig pumaConfig;
pumaConfig.httpPort = 9443;
pumaConfig.wsPort = 9453;
pumaConfig.host = "puma.internal.network";
pumaConfig.sslConfig = internalSsl;

server.SetPumaConnectionParam(pumaConfig);
server.Start(externalConfig);
```

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

### Example 4: Distributed Architecture with Puma Backend
```cpp
#include <AuthServerSdk/AuthServerSdk.h>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    using namespace AuthServerSdk;
    
    CAuthorizableServer server;
    
    // STEP 1: Configure connection to Puma backend server
    // This is where the actual authorization logic runs
    ServerConfig pumaBackendConfig;
    pumaBackendConfig.httpPort = 9080;
    pumaBackendConfig.wsPort = 9090;
    pumaBackendConfig.host = "puma-backend.internal.network";
    
    if (!server.SetPumaConnectionParam(pumaBackendConfig)) {
        qCritical() << "Failed to configure Puma backend connection";
        return 1;
    }
    
    qInfo() << "Configured Puma backend at" 
            << pumaBackendConfig.host 
            << ":" << pumaBackendConfig.httpPort;
    
    // STEP 2: Configure external-facing server endpoints
    // This is what clients will connect to
    ServerConfig externalConfig;
    externalConfig.httpPort = 8080;
    externalConfig.wsPort = 8090;
    externalConfig.host = "0.0.0.0";  // Accept from all interfaces
    
    // STEP 3: Start the server
    // This starts the external endpoints and connects to Puma backend
    if (!server.Start(externalConfig)) {
        qCritical() << "Failed to start server";
        return 1;
    }
    
    qInfo() << "Authorization server running";
    qInfo() << "External clients connect to:" 
            << externalConfig.host 
            << ":" << externalConfig.httpPort;
    qInfo() << "Backend Puma server at:" 
            << pumaBackendConfig.host 
            << ":" << pumaBackendConfig.httpPort;
    
    // Architecture:
    // Client Apps → [8080] → AuthServerSdk → [9080] → Puma Server
    
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
1. **Configure Puma backend first**: Always call `SetPumaConnectionParam()` before `Start()`
2. **Verify Puma server availability**: Ensure Puma backend server is running before starting the SDK
3. **Separate external and internal ports**: Use different port ranges for external clients and internal Puma communication
4. **Bind to specific interfaces**: Use specific IPs instead of "0.0.0.0" in production
5. **Use non-standard ports**: Avoid default ports (80, 443) when possible
6. **Separate environments**: Different ports/hosts for development, staging, production
7. **Validate configuration**: Check return values of all configuration methods
8. **Load features files**: Use features files for flexible licensing
9. **Document deployment architecture**: Clearly document which ports are for external clients vs. internal Puma communication

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

### Puma Backend Connection Issues

**Problem**: Server starts but authorization requests fail

**Possible Causes:**
1. Puma backend server not running
2. Incorrect Puma connection configuration
3. Network connectivity issues to Puma server
4. Firewall blocking internal connection

**Solutions:**
```cpp
// Verify Puma server is running and accessible
// Check Puma server logs for connection attempts

// Ensure SetPumaConnectionParam is called before Start()
ServerConfig pumaConfig;
pumaConfig.host = "puma-backend.internal";
pumaConfig.httpPort = 9080;
pumaConfig.wsPort = 9090;

if (!server.SetPumaConnectionParam(pumaConfig)) {
    qCritical() << "Failed to configure Puma connection";
    // Check if PumaConnectionInterfaceParam component is available
}

// Test connectivity to Puma server manually
// ping puma-backend.internal
// telnet puma-backend.internal 9080
```

**Important**: The AuthServerSdk cannot function without a properly configured and running Puma backend server. All authorization operations are processed by the Puma server.

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
