# Puma Server

## Overview

**Puma** is a user administration and authorization server application built on the ImagingTools framework stack. It provides a REST/GraphQL API server for managing users, roles, and groups with JWT-based authentication and LDAP integration support.

## Purpose

The Puma Server application serves as a central authentication and user management system that:

- Manages user accounts, roles, and groups through a GraphQL API
- Provides JWT (JSON Web Token) based authentication
- Supports LDAP integration for enterprise authentication
- Offers database-backed persistent storage (PostgreSQL)
- Exposes HTTP/WebSocket server interfaces for client applications
- Includes web-based administration interface

## Architecture

Puma Server is built using a component-based architecture (ACF - Adaptive Component Framework) with the following layers:

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Layer                         │
│  (ConsoleApplicationSqlLogFramework, ApplicationInfo)        │
└─────────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────────┐
│                      API Layer                               │
│  - GraphQL Endpoint: /Puma/graphql                          │
│  - File Access: /Puma/files/*                               │
│  - Web UI: /Puma/Views/* (Administration Interface)         │
└─────────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────────┐
│              Authentication & Authorization                  │
│  - JWT Session Management                                    │
│  - LDAP Authorization Controller                            │
│  - Credential Controllers (Standard & LDAP)                 │
└─────────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────────┐
│                   Data Layer                                 │
│  - User, Role, UserGroup Repositories (SQL)                 │
│  - Database Migration System                                │
│  - PostgreSQL Database Engine                               │
└─────────────────────────────────────────────────────────────┘
                           │
┌─────────────────────────────────────────────────────────────┐
│                Server Infrastructure                         │
│  - HTTP Server Framework                                     │
│  - WebSocket Server Framework                               │
│  - SSL/TLS Configuration                                     │
└─────────────────────────────────────────────────────────────┘
```

## Component Composition (Partitura)

The application is composed of several ACF components organized in the `Partitura/PumaVoce.arp` directory:

### Core Components

#### 1. **PumaServerBase** (`PumaServerBase.acc`)
The main server base component that orchestrates all subsystems:
- DatabaseEngine (PostgreSQL)
- HttpServerFramework (HTTP/WebSocket server)
- PumaHandlers (Request routing)
- PumaSettings (Configuration management)
- Repositories (User, Role, UserGroup data access)
- SmtpClient (Email notifications)
- SslConfigurationManager (SSL/TLS setup)
- Migration system (Database schema updates)

#### 2. **Authorization** (`Authorization.acc`)
Authentication and session management subsystem:
- **AuthorizationController** (ImtAuthGqlPck/LdapAuthorizationController) - Main authorization handler
- **JwtSessionController** (ImtAuthVoce/JwtSessionController) - JWT token management
- **GqlJwtSessionController** (ImtAuthGqlPck) - GraphQL JWT endpoints
- **CredentialController** (ImtAuthPck) - Standard credential validation
- **LdapCredentialController** (ImtAuthPck) - LDAP credential validation
- **HashCalculator** (ImtCryptPck) - Password hashing
- **Demultiplexer** (ImtServerGqlPck/GraphQLDemultiplexer) - Request routing

#### 3. **PumaHandlers** (`PumaHandlers.acc`)
Request handling and data controller components:
- **RootDataController** (PumaVoce) - Central GraphQL request coordinator
- **StandardGraphQlHandlers** (ImtGraphQlVoce) - File and web view handlers
- **RestoringDatabaseController** (ImtDatabasePck) - Database backup/restore
- Collection observers for Users, Roles, and UserGroups

#### 4. **RootDataController** (`RootDataController.acc`)
Document and collection management:
- **DocumentRevisionController** - Document versioning
- **Pages** - User administration pages (Users, Roles, UserGroups)
- **Repositories** - SQL repositories for persistent storage
  - RoleDocumentSqlRepository
  - UserSqlRepository
  - UserGroupSqlRepository
  - UserVerificationCollection

#### 5. **PumaSettings** (`PumaSettings.acc`)
Server configuration components:
- Database connection settings
- Backup configuration
- LDAP settings
- Logging parameters
- SSL/TLS configuration
- File auto-persistence

#### 6. **PumaSettingsRepresentation** (`PumaSettingsRepresentation.acc`)
Configuration UI representation layer for settings management

#### 7. **PumaLocalization** (`PumaLocalization.acc`)
Internationalization and localization support

#### 8. **PumaTestHandler** (`PumaTestHandler.acc`)
Testing and diagnostic handlers

## ImagingTools Component Dependencies

### ImtCore Framework Components

Puma Server extensively uses components from the **ImagingTools/ImtCore** repository:

#### Core Application Framework (ImtCoreVoce)
- **ConsoleApplicationSqlLogFramework** - Console application with SQL logging
- **ApplicationInfo** - Application metadata
- **RuntimeStatus** - Runtime state management
- **VersionInfo** - Version information
- **SystemConfig** - System configuration
- **TranslationManager** - Localization management

#### Database Components (ImtDatabasePck, ImtCoreVoce)
- **DatabaseEngine** - Database abstraction layer (PostgreSQL QPSQL driver)
- **PgSqlDatabaseEngine** - PostgreSQL-specific engine
- **DatabaseAccessSettings** - Connection configuration
- **RestoringDatabaseController** - Backup and restore functionality
- **CompositeMigrationController** - Schema migration system

#### Server Components (ImtHttpServerVoce, ImtServerAppPck)
- **HttpServerFramework** - HTTP server infrastructure
- **WebSocketServerFramework** - WebSocket server
- **FileBasedSslConfiguration** - SSL/TLS configuration
- **ServerSettings** - Server configuration management
- **ConnectionCollection** - Connection pool management

#### GraphQL Components (ImtGraphQlVoce, ImtServerGqlPck)
- **StandardGraphQlHandlers** - GraphQL request handlers
- **GraphQLDemultiplexer** - Request routing and demultiplexing
- **SerializableObjectCollectionController** - Object serialization
- **BinaryDataProvider** - Binary data handling
- **FileController** - File access management

#### Authentication Components (ImtAuthVoce, ImtAuthPck, ImtAuthGqlPck)
- **JwtSessionController** - JWT token generation and validation
- **LdapAuthorizationController** - LDAP authentication
- **GqlJwtSessionController** - GraphQL authentication endpoints
- **CredentialController** - Standard credential validation
- **LdapCredentialController** - LDAP credential validation
- **SessionInfo** - Session metadata

#### User Administration (ImtUserAdministrationVoce)
- **UsersPage** - User management UI
- **RolesPage** - Role management UI
- **UserGroupsPage** - User group management UI
- **UserSqlRepository** - User data persistence
- **RoleSqlRepository** - Role data persistence
- **UserGroupSqlRepository** - User group data persistence
- **UserVerificationCollection** - Email verification management

#### Utility Components
- **FileAutoPersistence** (FilePck) - Automatic file persistence
- **ComposedParamsSet** (ImtComPck) - Parameter composition
- **SmtpClient** (ImtMailPck) - Email functionality
- **HashCalculator** (ImtCryptPck) - Cryptographic hashing
- **LoggingParams** (ImtCorePck) - Logging configuration

### ACF Framework (ImagingTools/Acf)

The application is built on the **Adaptive Component Framework (ACF)** which provides:

- **Component Model** - XML-based component definitions (.acc files)
- **Component Linking** - Component interconnection (.accl files)
- **Dependency Injection** - Automatic dependency resolution
- **Interface-based Design** - Clean separation of concerns
- **Runtime Composition** - Dynamic component assembly

The ACF versions used in the Partitura:
- **ACF Version**: 5398-5430
- **ACF-Solutions Version**: 2202-2253
- **Qt Framework**: 394753-395264

## Package Usage Statistics

Based on the component composition analysis:

| Package | Component Count | Purpose |
|---------|----------------|---------|
| ImtUserAdministrationVoce | 10 | User/Role/Group management pages and repositories |
| ImtServerAppPck | 10 | Server application framework and settings |
| ImtServerGqlPck | 8 | GraphQL request handling and serialization |
| ImtDatabasePck | 8 | Database access and migration |
| PumaVoce | 7 | Puma-specific components |
| ImtCoreVoce | 7 | Core application framework |
| ImtHttpServerVoce | 6 | HTTP/WebSocket server infrastructure |
| ImtAuthPck | 5 | Authentication and credential management |
| ImtAuthGqlPck | 5 | GraphQL authentication endpoints |
| BasePck | 5 | Base framework utilities |
| FilePck | 4 | File management |
| ImtGraphQlVoce | 2 | GraphQL handlers |
| ImtCorePck | 2 | Core utilities |
| ImtAuthVoce | 1 | JWT session controller |
| ImtCryptPck | 1 | Cryptographic functions |
| ImtMailPck | 1 | Email client |
| ImtComPck/ImtComVoce | 2 | Component composition |
| ImtAppPck | 1 | Application framework |
| QtPck | 1 | Qt framework integration |

## Building the Project

### Prerequisites

- Qt Framework (version 5.x or 6.x)
- PostgreSQL client libraries
- C++ compiler (MSVC, GCC, or Clang)
- ImagingTools/ImtCore repository
- ImagingTools/Acf framework

### Build Process

The project uses a custom build system located in the `Build` directory.

#### Windows

```cmd
cd Build
REM Build scripts should be available here
```

#### Linux/macOS

```bash
cd Build
# Build scripts should be available here
```

### Git Hooks

The project includes Git hooks for automatic version management. See [Build/Git/README.md](Build/Git/README.md) for details on installing hooks that automatically update version information.

## Configuration

Server configuration is managed through the PumaSettings components and includes:

- **Database Settings** - PostgreSQL connection parameters
- **Server Settings** - HTTP port, WebSocket configuration
- **SSL/TLS Settings** - Certificate and key paths
- **LDAP Settings** - LDAP server configuration (optional)
- **Backup Settings** - Automatic backup configuration
- **Logging Settings** - Log levels and output configuration

Configuration files are stored in the `Config` directory.

## API Endpoints

The server exposes the following endpoints:

- **GraphQL API**: `/Puma/graphql` - Main API endpoint for queries and mutations
- **File Access**: `/Puma/files/*` - Static file serving
- **Web UI**: `/Puma/Views/*` - Administration interface (HTML/QML)

### GraphQL Operations

The API supports operations for:
- User management (create, read, update, delete)
- Role management
- User group management
- Authentication (login, logout, token refresh)
- Session management
- JWT token operations

## Development

### Project Structure

```
Puma/
├── Build/              # Build system and scripts
├── Config/             # Configuration files
├── Docs/               # Documentation (Doxygen)
├── Impl/               # Implementation
│   ├── PumaServer/     # Main server application
│   ├── PumaClient/     # Client application
│   ├── AuthServerSdk/  # Server SDK
│   ├── AuthClientSdk/  # Client SDK
│   └── Plugins/        # Plugin modules
├── Include/            # Header files
├── Install/            # Installation resources
└── Partitura/          # Component composition (ACF)
    └── PumaVoce.arp/   # Main application partitura
```

### Component Development

Components are defined using the ACF XML format:
- `.acc` files - Component configuration
- `.accl` files - Component linking and positioning

To modify the application architecture, edit the corresponding `.acc` files in `Partitura/PumaVoce.arp/`.

## License

Copyright (C) 2024-2025 ImagingTools GmbH. All rights reserved.

This software is part of the ImagingTools SDK.

## Version Information

Version information is automatically managed through Git hooks. The current revision is embedded in the compiled application and accessible via the VersionInfo component.

For version management details, see [Build/Git/README.md](Build/Git/README.md).
