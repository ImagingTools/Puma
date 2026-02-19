# Puma - Cross-Platform Authentication and Authorization Framework

Puma is a sophisticated, production-grade authentication and authorization server framework built on Qt and ImtCore. It provides a centralized authorization system with SSL/TLS support, client SDKs, and a modular plugin architecture designed for enterprise applications.

## Overview

Puma enables applications to implement robust authentication and authorization mechanisms through a centralized server architecture. It offers both server and client components, making it easy to integrate secure user authentication into your applications.

## Key Features

- **Centralized Authorization Server** - Multi-client authentication and authorization management
- **SSL/TLS Security** - Built-in support for secure connections with certificate management
- **Cross-Platform Support** - Compatible with Windows and Linux systems
- **Client SDKs** - Ready-to-use SDKs for integrating authentication into applications
- **WebSocket Communication** - Real-time server-client communication protocol
- **QML UI Framework** - Modern declarative UI components for login and administration
- **Modular Plugin Architecture** - Extensible settings management through plugins
- **Component-Based Design** - Built on ImtCore framework with Partitura configuration system
- **Database Integration** - SQLite/SQL database support for user and authorization data
- **Automated Version Management** - Git-based version tracking system

## Architecture

### Core Components

#### Server Components
- **PumaServer** - Main authorization server application with Qt-based architecture
- **AuthServerSdk** - SDK for embedding authorization server functionality in external applications
- **PumaServerConfigurator** - GUI tool for managing and configuring the authorization server
- **PumaSettingsPlugin** - Plugin system for extensible server settings management

#### Client Components
- **PumaClient** - Client application for interacting with the authorization server
- **AuthClientSdk** - SDK for client applications to authenticate with the server

#### Libraries
- **pumaqml** - QML/Qt Quick components library providing UI widgets (LoginWidget, AdministrationWidget)
- **pumatest** - Testing framework with utilities for controllers and authentication flows
- **puma** - Core library with version management and shared functionality

## Technologies Used

- **Qt 5/6** - Cross-platform application framework (Widgets, QML, WebSockets, SQL, Network)
- **ImtCore** - Component-based application framework
- **Partitura** - Declarative component architecture configuration (`.acc` files)
- **CMake** - Primary build system
- **C++** - Core programming language
- **OpenSSL** - SSL/TLS security implementation
- **SQLite** - Database backend
- **WebSockets** - Real-time communication protocol

## Building the Project

### Prerequisites

- Qt 5 or Qt 6
- CMake 3.x or higher
- C++ compiler (Visual C++ 2015/2017/2019 for Windows, GCC/Clang for Linux)
- OpenSSL libraries
- ImtCore framework

### Build Instructions

The project uses CMake as the primary build system. Build configurations are located in the `Build/` directory.

#### Windows
```cmd
cd Build
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release
```

#### Linux
```bash
cd Build
cmake ..
make
```

### Installation

Installation scripts are provided in the `Install/` directory. After building, run the appropriate installation script for your platform.

## Configuration

Configuration files are managed through the Partitura system (`.acc` files) located in the `Partitura/` directory. The main configuration package is `Partitura/PumaVoce.arp/` which contains various `.acc` configuration files for different components.

Server configuration can be managed through:
- **PumaServerConfigurator** GUI tool
- Direct editing of configuration files in `Config/` directory
- Settings plugins for advanced customization

## Version Management

Puma includes an automated Git-based version management system. See [Build/Git/README.md](Build/Git/README.md) for details on:
- Installing Git hooks
- Automatic version updates
- Manual version management

## Documentation

### SDK Documentation

- **AuthServerSdk** - [Docs/AuthServerSdk.md](Docs/AuthServerSdk.md) - Complete guide for embedding authorization server functionality
- **AuthClientSdk** - [Docs/AuthClientSdk.md](Docs/AuthClientSdk.md) - Complete guide for client authentication and user management

### Other Documentation

- **API Documentation** - Generate using Doxygen (see `Docs/Doxyfile`)
- **Git Scripts** - [Build/Git/README.md](Build/Git/README.md)
- **Source Code** - Headers in `Include/`, implementations in `Impl/`

### Generating API Documentation

```bash
cd Docs
doxygen Doxyfile
```

Note: The Doxyfile may need to be updated to reflect the current project name.

## Project Structure

```
Puma/
├── Build/              # Build system files and Git scripts
├── Config/             # Configuration files
├── Docs/               # Documentation and Doxygen configuration
├── Impl/               # Implementation files
│   ├── AuthClientSdk/  # Client authentication SDK
│   ├── AuthServerSdk/  # Server authentication SDK
│   ├── Plugins/        # Plugin implementations
│   ├── PumaClient/     # Client application
│   ├── PumaServer/     # Server application
│   └── PumaServerConfigurator/ # Configuration tool
├── Include/            # Header files
│   ├── puma/           # Core library headers
│   ├── pumaqml/        # QML components
│   └── pumatest/       # Testing framework
├── Install/            # Installation scripts
└── Partitura/          # Partitura configuration files
```

## Testing

The project includes a comprehensive testing framework (`pumatest`) with test cases for:
- Authorization controllers
- User management
- Authentication flows
- Client SDK functionality

## License

Puma is licensed under a Commercial License. See [LICENSE](LICENSE) for full license text.

For licensing inquiries, please contact: info@imagingtools.de

For information about third-party dependencies and their licenses, see [Docs/Dependencies.md](Docs/Dependencies.md).

## Security

Security is a top priority for Puma. For information about:
- Reporting security vulnerabilities
- Security best practices
- Supported versions
- Security advisories

Please see our [Security Policy](SECURITY.md).

## Compliance

Puma is designed to comply with the EU Cyber Resilience Act (CRA) and includes:
- **Software Bill of Materials (SBOM)**: [Docs/SBOM.json](Docs/SBOM.json) - Complete inventory of dependencies
- **Dependencies Documentation**: [Docs/Dependencies.md](Docs/Dependencies.md) - Detailed dependency information
- **Security Policy**: [SECURITY.md](SECURITY.md) - Vulnerability disclosure and security practices

## Contributing

Contributions are welcome! Please read our [Contributing Guidelines](CONTRIBUTING.md) before submitting changes.

Key points for contributors:
- Follow existing code style and conventions
- Include tests for new features
- Update documentation as needed
- Follow security guidelines for authentication/authorization code
- Builds must succeed on all supported platforms

For security-related contributions, please review [SECURITY.md](SECURITY.md) first.

## Support

For issues, questions, or contributions, please use the GitHub issue tracker for this repository.
