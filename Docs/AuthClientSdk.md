# AuthClientSdk Documentation

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Getting Started](#getting-started)
- [API Reference](#api-reference)
- [Authentication](#authentication)
- [Authorization](#authorization)
- [User Management](#user-management)
- [Role Management](#role-management)
- [Group Management](#group-management)
- [Usage Examples](#usage-examples)
- [Best Practices](#best-practices)
- [Troubleshooting](#troubleshooting)

## Overview

The AuthClientSdk (Authorization Client SDK) is a comprehensive C++ library that provides client-side functionality for authentication, authorization, and user management. It enables applications to authenticate users, check permissions, and manage users, roles, and groups through a centralized authorization server.

The SDK is designed for:
- Client applications requiring secure authentication
- Systems needing role-based access control (RBAC)
- Applications with user and permission management requirements
- Secure client-server communication with SSL/TLS support

### Key Characteristics
- **PIMPL Pattern**: Uses Pointer to Implementation for ABI stability
- **Platform Support**: Windows-only (WIN32 conditional compilation)
- **Qt-based**: Built on Qt framework
- **ImtCore Integration**: Leverages ImtCore authentication framework
- **Comprehensive API**: Full CRUD operations for users, roles, and groups

## Features

### Authentication & Authorization
- **User Login/Logout**: Secure credential-based authentication
- **Token Management**: Access token generation and validation
- **Permission Checking**: Real-time permission verification
- **Superuser Management**: Initial superuser account setup
- **Multi-Auth Support**: Local and LDAP authentication systems

### User Management
- **User CRUD**: Create, read, update, delete user accounts
- **User Lookup**: Find users by ID or login
- **Password Management**: Change user passwords securely
- **User Permissions**: Query user permissions
- **Role Assignment**: Add/remove roles from users

### Role-Based Access Control (RBAC)
- **Role Management**: Create and manage roles
- **Permission Assignment**: Add/remove permissions to/from roles
- **Role Hierarchy**: Support for complex role structures
- **Dynamic Permissions**: Query and modify role permissions

### Group Management
- **Group CRUD**: Create, read, update, delete groups
- **User Grouping**: Add/remove users to/from groups
- **Group Roles**: Assign roles to groups
- **Group Hierarchy**: Organize users into logical groups

### Security
- **SSL/TLS Support**: Encrypted client-server communication
- **Certificate Verification**: CA certificate validation
- **Token-based Auth**: Secure access token management
- **Configurable Security**: Flexible SSL configuration options

## Architecture

### Component Structure

```
┌──────────────────────────────────────┐
│  CAuthorizationController (Public)   │
├──────────────────────────────────────┤
│  CAuthorizationControllerImpl (PIMPL)│
├──────────────────────────────────────┤
│      CAuthClientSdk (Component)      │
├──────────────────────────────────────┤
│         ImtCore Framework            │
│  ┌───────────────────────────────┐   │
│  │ ILogin                        │   │
│  │ IRightsProvider               │   │
│  │ IAccessTokenProvider          │   │
│  │ IUserManager                  │   │
│  │ IRoleManager                  │   │
│  │ IUserGroupManager             │   │
│  │ ISuperuserController          │   │
│  │ IServerConnectionInterface    │   │
│  └───────────────────────────────┘   │
└──────────────────────────────────────┘
```

### Data Flow

```
Application
    ↓
CAuthorizationController (Public API)
    ↓
CAuthorizationControllerImpl (Business Logic)
    ↓
CAuthClientSdk (Component Manager)
    ↓
ImtCore Interfaces (Backend Communication)
    ↓
Authorization Server
```

### Dependencies
- **Qt Core**: QString, QByteArray, QByteArrayList
- **Qt Network**: QSslConfiguration, QSsl
- **ACF**: Component framework
- **ImtCore**: Authentication, user management, networking

## Getting Started

### Prerequisites
- Qt 5 or Qt 6
- C++ compiler with C++17 support
- ImtCore framework
- AuthServerSdk or compatible authorization server
- Windows platform (WIN32)

### Basic Integration

1. **Include the SDK header**:
```cpp
#include <AuthClientSdk/AuthClientSdk.h>
```

2. **Link against the SDK library**:
   - Add `AuthClientSdk` to your project's linked libraries
   - Ensure Qt modules are linked: Core, Network

3. **Initialize and authenticate**:
```cpp
using namespace AuthClientSdk;

// Create controller
CAuthorizationController authController;

// Configure server connection
ServerConfig config;
config.host = "localhost";
config.httpPort = 8080;
config.wsPort = 8090;
authController.SetConnectionParam(config);

// Set product ID
authController.SetProductId("MyProduct-ID");

// Login
Login loginData;
if (authController.Login("username", "password", loginData)) {
    qInfo() << "Logged in as:" << loginData.userName;
    qInfo() << "Access token:" << loginData.accessToken;
} else {
    qCritical() << "Login failed";
}
```

## API Reference

### Main Class: `CAuthorizationController`

#### Constructor & Destructor
```cpp
CAuthorizationController();
virtual ~CAuthorizationController();
```

### Connection & Configuration

#### `SetConnectionParam()`
```cpp
bool SetConnectionParam(const ServerConfig& config) const;
```
Configures connection to the authorization server.

**Parameters:**
- `config`: Server configuration (host, ports, SSL)

**Returns:**
- `true` if configured successfully
- `false` if connection interface unavailable

#### `SetProductId()`
```cpp
virtual void SetProductId(const QByteArray& productId) const;
```
Sets the product identifier for licensing and authorization context.

**Parameters:**
- `productId`: Binary product identifier

### Authentication

#### `Login()`
```cpp
virtual bool Login(const QString& login, 
                   const QString& password, 
                   Login& out) const;
```
Authenticates a user with login credentials.

**Parameters:**
- `login`: User login identifier
- `password`: User password
- `out`: Output structure containing login session data

**Returns:**
- `true` if authentication successful
- `false` if authentication failed

**Output Data** (`Login` structure):
- `accessToken`: Access token for subsequent requests
- `userName`: Display name of the user
- `productId`: Product identifier
- `permissions`: List of user permissions

#### `Logout()`
```cpp
virtual bool Logout() const;
```
Logs out the current user and invalidates the session.

**Returns:**
- `true` if logout successful
- `false` on failure

#### `GetToken()`
```cpp
virtual QByteArray GetToken() const;
```
Retrieves the current access token.

**Returns:**
- Current access token, or empty if not authenticated

### Authorization

#### `HasPermission()`
```cpp
virtual bool HasPermission(const QByteArray& permissionId) const;
```
Checks if the current user has a specific permission.

**Parameters:**
- `permissionId`: Permission identifier to check

**Returns:**
- `true` if user has permission
- `false` if user lacks permission or not authenticated

### Superuser Management

#### `SuperuserExists()`
```cpp
virtual SuperuserStatus SuperuserExists(QString& errorMessage) const;
```
Checks whether a superuser account exists in the system.

**Parameters:**
- `errorMessage`: Output error description if check fails

**Returns:**
- `SuperuserStatus::Exists`: Superuser exists
- `SuperuserStatus::NotExists`: No superuser configured
- `SuperuserStatus::Unknown`: Cannot determine status

**Usage**: Call this during initial setup to determine if first-time configuration is needed.

#### `CreateSuperuser()`
```cpp
virtual bool CreateSuperuser(const QByteArray& password) const;
```
Creates the initial superuser account.

**Parameters:**
- `password`: Superuser password

**Returns:**
- `true` if superuser created successfully
- `false` on failure

**Note**: Typically called only once during initial system setup.

### User Management

#### `GetUserIds()`
```cpp
virtual QByteArrayList GetUserIds() const;
```
Retrieves all user identifiers in the system.

**Returns:**
- List of user IDs, or empty list on failure

#### `GetUser()`
```cpp
virtual bool GetUser(const QByteArray& userId, User& userData) const;
```
Retrieves user information by user ID.

**Parameters:**
- `userId`: User identifier
- `userData`: Output user information

**Returns:**
- `true` if user found
- `false` if user not found or error

**Output Data** (`User` structure):
- `name`: User display name
- `email`: User email address
- `login`: User login identifier
- `roleIds`: List of role IDs assigned to user
- `groupIds`: List of group IDs user belongs to

#### `GetUserByLogin()`
```cpp
virtual bool GetUserByLogin(const QByteArray& login, User& userData) const;
```
Retrieves user information by login identifier.

**Parameters:**
- `login`: User login
- `userData`: Output user information

**Returns:**
- `true` if user found
- `false` if user not found or error

#### `CreateUser()`
```cpp
virtual QByteArray CreateUser(
    const QString& userName,
    const QByteArray& login,
    const QByteArray& password,
    const QString& email) const;
```
Creates a new user account.

**Parameters:**
- `userName`: User display name
- `login`: User login identifier (must be unique)
- `password`: User password
- `email`: User email address

**Returns:**
- User ID of created user, or empty on failure

#### `RemoveUser()`
```cpp
virtual bool RemoveUser(const QByteArray& userId) const;
```
Deletes a user account.

**Parameters:**
- `userId`: User identifier to remove

**Returns:**
- `true` if user removed successfully
- `false` on failure

#### `ChangeUserPassword()`
```cpp
virtual bool ChangeUserPassword(
    const QByteArray& login,
    const QByteArray& oldPassword,
    const QByteArray& newPassword) const;
```
Changes a user's password.

**Parameters:**
- `login`: User login
- `oldPassword`: Current password (for verification)
- `newPassword`: New password

**Returns:**
- `true` if password changed successfully
- `false` if old password incorrect or other error

#### `AddRolesToUser()`
```cpp
virtual bool AddRolesToUser(
    const QByteArray& userId, 
    const QByteArrayList& roleIds) const;
```
Assigns roles to a user.

**Parameters:**
- `userId`: User identifier
- `roleIds`: List of role IDs to add

**Returns:**
- `true` if roles added successfully
- `false` on failure

#### `RemoveRolesFromUser()`
```cpp
virtual bool RemoveRolesFromUser(
    const QByteArray& userId, 
    const QByteArrayList& roleIds) const;
```
Removes roles from a user.

**Parameters:**
- `userId`: User identifier
- `roleIds`: List of role IDs to remove

**Returns:**
- `true` if roles removed successfully
- `false` on failure

#### `GetUserPermissions()`
```cpp
virtual QByteArrayList GetUserPermissions(const QByteArray& userId) const;
```
Retrieves all permissions assigned to a user (from all roles).

**Parameters:**
- `userId`: User identifier

**Returns:**
- List of permission IDs, or empty list on failure

#### `GetUserAuthSystem()`
```cpp
virtual SystemType GetUserAuthSystem(const QByteArray& login) const;
```
Determines the authentication system used by a user.

**Parameters:**
- `login`: User login

**Returns:**
- `SystemType::Local`: Local authentication
- `SystemType::Ldap`: LDAP authentication
- `SystemType::Unknown`: Cannot determine

### Role Management

#### `GetRoleIds()`
```cpp
virtual QByteArrayList GetRoleIds() const;
```
Retrieves all role identifiers in the system.

**Returns:**
- List of role IDs, or empty list on failure

#### `GetRole()`
```cpp
virtual bool GetRole(const QByteArray& roleId, Role& roleData) const;
```
Retrieves role information.

**Parameters:**
- `roleId`: Role identifier
- `roleData`: Output role information

**Returns:**
- `true` if role found
- `false` if role not found or error

**Output Data** (`Role` structure):
- `name`: Role name
- `description`: Role description
- `permissionIds`: List of permission IDs assigned to role

#### `CreateRole()`
```cpp
virtual QByteArray CreateRole(
    const QString& roleName,
    const QString& roleDescription = QString(),
    const QByteArrayList& permissions = QByteArrayList());
```
Creates a new role.

**Parameters:**
- `roleName`: Role name
- `roleDescription`: Optional role description
- `permissions`: Optional initial permissions

**Returns:**
- Role ID of created role, or empty on failure

#### `RemoveRole()`
```cpp
virtual bool RemoveRole(const QByteArray& roleId) const;
```
Deletes a role.

**Parameters:**
- `roleId`: Role identifier to remove

**Returns:**
- `true` if role removed successfully
- `false` on failure

#### `GetRolePermissions()`
```cpp
virtual QByteArrayList GetRolePermissions(const QByteArray& roleId) const;
```
Retrieves permissions assigned to a role.

**Parameters:**
- `roleId`: Role identifier

**Returns:**
- List of permission IDs, or empty list on failure

#### `AddPermissionsToRole()`
```cpp
virtual bool AddPermissionsToRole(
    const QByteArray& roleId,
    const QByteArrayList& permissions) const;
```
Adds permissions to a role.

**Parameters:**
- `roleId`: Role identifier
- `permissions`: List of permission IDs to add

**Returns:**
- `true` if permissions added successfully
- `false` on failure

#### `RemovePermissionsFromRole()`
```cpp
virtual bool RemovePermissionsFromRole(
    const QByteArray& roleId,
    const QByteArrayList& permissions) const;
```
Removes permissions from a role.

**Parameters:**
- `roleId`: Role identifier
- `permissions`: List of permission IDs to remove

**Returns:**
- `true` if permissions removed successfully
- `false` on failure

### Group Management

#### `GetGroupIds()`
```cpp
virtual QByteArrayList GetGroupIds() const;
```
Retrieves all group identifiers in the system.

**Returns:**
- List of group IDs, or empty list on failure

#### `GetGroup()`
```cpp
virtual bool GetGroup(const QByteArray& groupId, Group& groupData) const;
```
Retrieves group information.

**Parameters:**
- `groupId`: Group identifier
- `groupData`: Output group information

**Returns:**
- `true` if group found
- `false` if group not found or error

**Output Data** (`Group` structure):
- `name`: Group name
- `description`: Group description
- `userIds`: List of user IDs in the group
- `roleIds`: List of role IDs assigned to group

#### `CreateGroup()`
```cpp
virtual QByteArray CreateGroup(
    const QString& groupName, 
    const QString& description) const;
```
Creates a new group.

**Parameters:**
- `groupName`: Group name
- `description`: Group description

**Returns:**
- Group ID of created group, or empty on failure

#### `RemoveGroup()`
```cpp
virtual bool RemoveGroup(const QByteArray& groupId) const;
```
Deletes a group.

**Parameters:**
- `groupId`: Group identifier to remove

**Returns:**
- `true` if group removed successfully
- `false` on failure

#### `AddUsersToGroup()`
```cpp
virtual bool AddUsersToGroup(
    const QByteArray& groupId,
    const QByteArrayList& userIds) const;
```
Adds users to a group.

**Parameters:**
- `groupId`: Group identifier
- `userIds`: List of user IDs to add

**Returns:**
- `true` if users added successfully
- `false` on failure

#### `RemoveUsersFromGroup()`
```cpp
virtual bool RemoveUsersFromGroup(
    const QByteArray& groupId,
    const QByteArrayList& userIds) const;
```
Removes users from a group.

**Parameters:**
- `groupId`: Group identifier
- `userIds`: List of user IDs to remove

**Returns:**
- `true` if users removed successfully
- `false` on failure

#### `AddRolesToGroup()`
```cpp
virtual bool AddRolesToGroup(
    const QByteArray& groupId,
    const QByteArrayList& roleIds) const;
```
Assigns roles to a group.

**Parameters:**
- `groupId`: Group identifier
- `roleIds`: List of role IDs to add

**Returns:**
- `true` if roles added successfully
- `false` on failure

#### `RemoveRolesFromGroup()`
```cpp
virtual bool RemoveRolesFromGroup(
    const QByteArray& groupId,
    const QByteArrayList& roleIds) const;
```
Removes roles from a group.

**Parameters:**
- `groupId`: Group identifier
- `roleIds`: List of role IDs to remove

**Returns:**
- `true` if roles removed successfully
- `false` on failure

### Configuration Structures

#### `ServerConfig`
```cpp
struct ServerConfig {
    QString host = "localhost";             // Server hostname/IP
    int httpPort;                           // HTTP/HTTPS port
    int wsPort;                             // WebSocket port
    std::optional<SslConfig> sslConfig;     // Optional SSL configuration
};
```

#### `SslConfig`
```cpp
struct SslConfig {
    QList<QString> caCertificatePaths;                      // Trusted CA certificates
    QSsl::EncodingFormat caCertificateFormat = QSsl::Pem;   // CA cert format
    QSsl::SslProtocol protocol = QSsl::TlsV1_2;             // TLS protocol
    bool ignoreSslErrors = false;                           // Ignore SSL errors (testing only)
};
```

#### Enumerations

**`SuperuserStatus`**
```cpp
enum class SuperuserStatus {
    Unknown,    // Status cannot be determined
    Exists,     // Superuser exists
    NotExists   // Superuser does not exist
};
```

**`SystemType`**
```cpp
enum class SystemType {
    Unknown,    // Unknown authentication system
    Local,      // Local authentication
    Ldap        // LDAP-based authentication
};
```

## Authentication

### Login Flow

```
1. Configure server connection (SetConnectionParam)
2. Set product ID (SetProductId)
3. Call Login(username, password, loginData)
4. On success: Store access token
5. Use token for subsequent API calls
6. Call Logout() when done
```

### Login Example
```cpp
CAuthorizationController auth;

// Configure connection
ServerConfig config;
config.host = "auth.example.com";
config.httpPort = 8443;
config.wsPort = 8453;
auth.SetConnectionParam(config);

// Set product context
auth.SetProductId("MyApp-v1.0");

// Authenticate
Login loginData;
if (auth.Login("john.doe", "SecurePass123!", loginData)) {
    qInfo() << "Welcome," << loginData.userName;
    qInfo() << "You have" << loginData.permissions.size() << "permissions";
    
    // Store token for later use
    QByteArray token = loginData.accessToken;
} else {
    qWarning() << "Authentication failed";
}
```

### Token Management

```cpp
// Get current token
QByteArray currentToken = auth.GetToken();

// Token is automatically used for all API calls
// No need to manually pass it to each function
```

## Authorization

### Permission Checking

```cpp
// Check if user has specific permission
if (auth.HasPermission("user.create")) {
    // User can create users
    createUserButton->setEnabled(true);
} else {
    // User cannot create users
    createUserButton->setEnabled(false);
}

// Check multiple permissions
QByteArrayList requiredPermissions = {
    "user.create",
    "user.edit",
    "user.delete"
};

bool hasAllPermissions = true;
for (const auto& perm : requiredPermissions) {
    if (!auth.HasPermission(perm)) {
        hasAllPermissions = false;
        break;
    }
}

if (hasAllPermissions) {
    qInfo() << "User has full user management permissions";
}
```

### Getting User Permissions

```cpp
// Get all permissions for a user
QByteArrayList userPermissions = auth.GetUserPermissions(userId);

qInfo() << "User permissions:";
for (const auto& perm : userPermissions) {
    qInfo() << "  -" << perm;
}
```

## Usage Examples

### Example 1: Basic Authentication
```cpp
#include <AuthClientSdk/AuthClientSdk.h>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    using namespace AuthClientSdk;
    
    CAuthorizationController auth;
    
    // Configure server
    ServerConfig config;
    config.host = "localhost";
    config.httpPort = 8080;
    config.wsPort = 8090;
    auth.SetConnectionParam(config);
    
    // Set product ID
    auth.SetProductId("MyApplication");
    
    // Login
    Login loginData;
    if (auth.Login("admin", "password", loginData)) {
        qInfo() << "Logged in successfully";
        qInfo() << "User:" << loginData.userName;
        qInfo() << "Token:" << loginData.accessToken;
        
        // Check permission
        if (auth.HasPermission("admin.access")) {
            qInfo() << "User has admin access";
        }
        
        // Logout
        auth.Logout();
    }
    
    return 0;
}
```

### Example 2: Secure Connection with SSL
```cpp
#include <AuthClientSdk/AuthClientSdk.h>

using namespace AuthClientSdk;

CAuthorizationController auth;

// Configure SSL
SslConfig sslConfig;
sslConfig.caCertificatePaths.append("certs/ca.crt");
sslConfig.protocol = QSsl::TlsV1_2;
sslConfig.ignoreSslErrors = false;  // Validate certificates

ServerConfig config;
config.host = "secure.example.com";
config.httpPort = 8443;
config.wsPort = 8453;
config.sslConfig = sslConfig;

auth.SetConnectionParam(config);

// Login over secure connection
Login loginData;
if (auth.Login("user", "password", loginData)) {
    qInfo() << "Secure login successful";
}
```

### Example 3: User Management
```cpp
using namespace AuthClientSdk;

CAuthorizationController auth;
// ... configure and login ...

// Create a new user
QByteArray newUserId = auth.CreateUser(
    "Jane Smith",           // Display name
    "jane.smith",           // Login
    "SecurePassword123!",   // Password
    "jane@example.com"      // Email
);

if (!newUserId.isEmpty()) {
    qInfo() << "Created user with ID:" << newUserId;
    
    // Assign roles to the user
    QByteArrayList roles = {"developer", "tester"};
    if (auth.AddRolesToUser(newUserId, roles)) {
        qInfo() << "Roles assigned successfully";
    }
    
    // Get user details
    User userData;
    if (auth.GetUser(newUserId, userData)) {
        qInfo() << "User name:" << userData.name;
        qInfo() << "User email:" << userData.email;
        qInfo() << "User roles:" << userData.roleIds;
    }
}
```

### Example 4: Role Management
```cpp
using namespace AuthClientSdk;

CAuthorizationController auth;
// ... configure and login ...

// Create a new role
QByteArray roleId = auth.CreateRole(
    "Content Editor",                    // Role name
    "Can edit and publish content",      // Description
    {"content.edit", "content.publish"}  // Initial permissions
);

if (!roleId.isEmpty()) {
    qInfo() << "Created role with ID:" << roleId;
    
    // Add more permissions
    QByteArrayList additionalPerms = {"content.delete"};
    auth.AddPermissionsToRole(roleId, additionalPerms);
    
    // Get role details
    Role roleData;
    if (auth.GetRole(roleId, roleData)) {
        qInfo() << "Role name:" << roleData.name;
        qInfo() << "Description:" << roleData.description;
        qInfo() << "Permissions:" << roleData.permissionIds;
    }
}
```

### Example 5: Group Management
```cpp
using namespace AuthClientSdk;

CAuthorizationController auth;
// ... configure and login ...

// Create a group
QByteArray groupId = auth.CreateGroup(
    "Development Team",
    "All developers"
);

if (!groupId.isEmpty()) {
    qInfo() << "Created group with ID:" << groupId;
    
    // Add users to group
    QByteArrayList userIds = {userId1, userId2, userId3};
    if (auth.AddUsersToGroup(groupId, userIds)) {
        qInfo() << "Users added to group";
    }
    
    // Assign roles to group
    QByteArrayList roleIds = {"developer", "code-reviewer"};
    if (auth.AddRolesToGroup(groupId, roleIds)) {
        qInfo() << "Roles assigned to group";
    }
    
    // Get group details
    Group groupData;
    if (auth.GetGroup(groupId, groupData)) {
        qInfo() << "Group name:" << groupData.name;
        qInfo() << "Members:" << groupData.userIds.size();
        qInfo() << "Roles:" << groupData.roleIds;
    }
}
```

### Example 6: First-Time Setup
```cpp
using namespace AuthClientSdk;

CAuthorizationController auth;
// ... configure connection ...

// Check if superuser exists
QString errorMsg;
SuperuserStatus status = auth.SuperuserExists(errorMsg);

if (status == SuperuserStatus::NotExists) {
    qInfo() << "No superuser found, creating one...";
    
    // Create superuser
    if (auth.CreateSuperuser("SuperSecurePassword123!")) {
        qInfo() << "Superuser created successfully";
        qInfo() << "Default login: 'admin'";
    } else {
        qCritical() << "Failed to create superuser";
    }
} else if (status == SuperuserStatus::Exists) {
    qInfo() << "Superuser already exists";
} else {
    qWarning() << "Cannot determine superuser status:" << errorMsg;
}
```

### Example 7: Password Change
```cpp
using namespace AuthClientSdk;

CAuthorizationController auth;
// ... configure and login ...

// Change password (requires old password)
if (auth.ChangeUserPassword(
        "john.doe",          // Login
        "OldPassword123",    // Old password
        "NewPassword456!"))  // New password
{
    qInfo() << "Password changed successfully";
} else {
    qWarning() << "Password change failed - check old password";
}
```

### Example 8: Complete User Administration
```cpp
using namespace AuthClientSdk;

void manageUsers(CAuthorizationController& auth) {
    // List all users
    QByteArrayList userIds = auth.GetUserIds();
    qInfo() << "Total users:" << userIds.size();
    
    for (const auto& userId : userIds) {
        User userData;
        if (auth.GetUser(userId, userData)) {
            qInfo() << "User:" << userData.name 
                    << "(" << userData.login << ")";
            qInfo() << "  Email:" << userData.email;
            qInfo() << "  Roles:" << userData.roleIds;
            qInfo() << "  Groups:" << userData.groupIds;
            
            // Get authentication system
            SystemType authSystem = auth.GetUserAuthSystem(userData.login);
            QString systemName = (authSystem == SystemType::Local) 
                ? "Local" 
                : (authSystem == SystemType::Ldap) 
                    ? "LDAP" 
                    : "Unknown";
            qInfo() << "  Auth System:" << systemName;
            
            // Get permissions
            QByteArrayList perms = auth.GetUserPermissions(userId);
            qInfo() << "  Permissions:" << perms.size();
        }
    }
}
```

## Best Practices

### Security
1. **Never hardcode credentials**: Use configuration files or secure storage
2. **Use SSL/TLS in production**: Always encrypt client-server communication
3. **Validate certificates**: Don't set `ignoreSslErrors = true` in production
4. **Store tokens securely**: Use Qt's secure storage or encrypted files
5. **Clear sensitive data**: Call `Login::Clear()` when done
6. **Implement session timeout**: Logout after inactivity period
7. **Strong passwords**: Enforce password complexity requirements
8. **Validate permissions**: Always check permissions before privileged operations

### Error Handling
```cpp
// Always check return values
if (!auth.Login(user, pass, loginData)) {
    qWarning() << "Login failed - check credentials and connection";
    // Handle error appropriately
    return;
}

// Check for interface availability
QByteArray token = auth.GetToken();
if (token.isEmpty()) {
    qWarning() << "No valid token - user not authenticated";
}

// Validate before operations
QByteArrayList userIds = auth.GetUserIds();
if (userIds.isEmpty()) {
    qWarning() << "No users found or query failed";
}
```

### Connection Management
```cpp
// Configure connection once at startup
bool setupAuth(CAuthorizationController& auth) {
    ServerConfig config;
    config.host = getServerHost();  // From config file
    config.httpPort = getHttpPort();
    config.wsPort = getWsPort();
    
    if (!auth.SetConnectionParam(config)) {
        qCritical() << "Failed to configure connection";
        return false;
    }
    
    auth.SetProductId(getProductId());
    return true;
}
```

### Resource Management
```cpp
// Proper cleanup
class AuthManager {
public:
    AuthManager() {
        setupAuth(m_auth);
    }
    
    ~AuthManager() {
        // Logout on destruction
        if (isLoggedIn()) {
            m_auth.Logout();
        }
    }
    
    bool login(const QString& user, const QString& pass) {
        Login loginData;
        if (m_auth.Login(user, pass, loginData)) {
            m_isLoggedIn = true;
            m_currentUser = loginData.userName;
            return true;
        }
        return false;
    }
    
    bool isLoggedIn() const { return m_isLoggedIn; }
    
private:
    CAuthorizationController m_auth;
    bool m_isLoggedIn = false;
    QString m_currentUser;
};
```

### Permission-Based UI
```cpp
// Enable/disable UI based on permissions
void updateUI(CAuthorizationController& auth) {
    ui->createUserButton->setEnabled(
        auth.HasPermission("user.create"));
    
    ui->deleteUserButton->setEnabled(
        auth.HasPermission("user.delete"));
    
    ui->manageRolesButton->setEnabled(
        auth.HasPermission("role.manage"));
    
    ui->adminPanel->setVisible(
        auth.HasPermission("admin.access"));
}
```

### Logging
```cpp
// Log authentication events
void logAuthEvent(const QString& event, const Login& data) {
    qInfo() << "[AUTH]" << event;
    qInfo() << "  User:" << data.userName;
    qInfo() << "  Product:" << data.productId;
    qInfo() << "  Permissions:" << data.permissions.size();
    // Don't log access token!
}

// Usage
Login loginData;
if (auth.Login(user, pass, loginData)) {
    logAuthEvent("Login successful", loginData);
}
```

## Troubleshooting

### Login Fails

**Problem**: `Login()` returns false

**Possible Causes:**
1. Incorrect credentials
2. Server not reachable
3. Connection not configured
4. Product ID mismatch

**Solutions:**
```cpp
// Verify connection first
if (!auth.SetConnectionParam(config)) {
    qCritical() << "Connection configuration failed";
}

// Check server is running
// Verify credentials
// Check logs on both client and server

// Test with known good credentials (e.g., superuser)
```

### Permission Check Always Returns False

**Problem**: `HasPermission()` always returns false

**Possible Causes:**
1. User not logged in
2. Permission doesn't exist
3. User lacks the permission
4. Token expired

**Solutions:**
```cpp
// Verify login first
Login loginData;
if (!auth.Login(user, pass, loginData)) {
    qWarning() << "Not logged in";
    return;
}

// Check what permissions user actually has
QByteArrayList userPerms = auth.GetUserPermissions(userId);
qDebug() << "User permissions:" << userPerms;

// Verify permission name is correct
if (auth.HasPermission("user.create")) {  // Correct spelling
    // ...
}
```

### SSL Connection Fails

**Problem**: Cannot connect with SSL enabled

**Possible Causes:**
1. CA certificate not found
2. Certificate validation fails
3. Protocol mismatch
4. Server not using SSL

**Solutions:**
```cpp
// Verify CA certificate path
QFile caFile(sslConfig.caCertificatePaths.first());
if (!caFile.exists()) {
    qCritical() << "CA certificate not found";
}

// Try with SSL errors ignored (testing only!)
sslConfig.ignoreSslErrors = true;  // WARNING: INSECURE

// Match TLS version with server
sslConfig.protocol = QSsl::TlsV1_2;  // Or TlsV1_3

// Verify server is actually using SSL on the specified port
```

### User Creation Fails

**Problem**: `CreateUser()` returns empty ID

**Possible Causes:**
1. Login already exists
2. Insufficient permissions
3. Invalid email format
4. Password doesn't meet requirements

**Solutions:**
```cpp
// Check if login already exists
User existingUser;
if (auth.GetUserByLogin(login, existingUser)) {
    qWarning() << "Login already exists";
    return;
}

// Verify current user has permission
if (!auth.HasPermission("user.create")) {
    qWarning() << "Insufficient permissions";
    return;
}

// Validate inputs
if (password.length() < 8) {
    qWarning() << "Password too short";
    return;
}
```

### Interface Not Found Warnings

**Problem**: Console shows "interface not found" warnings

**Possible Causes:**
1. ImtCore not properly initialized
2. Missing component dependencies
3. ACF configuration issues

**Solutions:**
1. Ensure ImtCore framework is installed and initialized
2. Check component initialization in ACF files
3. Verify all required libraries are linked
4. Check SDK component instantiation

### Empty Lists Returned

**Problem**: `GetUserIds()`, `GetRoleIds()`, etc. return empty

**Possible Causes:**
1. No data exists
2. Query failed
3. Insufficient permissions
4. Not connected to server

**Solutions:**
```cpp
// Verify connection
QByteArray token = auth.GetToken();
if (token.isEmpty()) {
    qWarning() << "Not authenticated";
    // Login first
}

// Check if data exists (e.g., create test user)
// Verify permissions
// Check server logs
```

## Integration with UI Frameworks

### Qt Widgets Integration
```cpp
class LoginDialog : public QDialog {
    Q_OBJECT
public:
    LoginDialog(CAuthorizationController* auth, QWidget* parent = nullptr)
        : QDialog(parent), m_auth(auth) {
        setupUi();
    }
    
private slots:
    void onLoginClicked() {
        QString user = ui->usernameEdit->text();
        QString pass = ui->passwordEdit->text();
        
        Login loginData;
        if (m_auth->Login(user, pass, loginData)) {
            QMessageBox::information(this, "Success", 
                "Welcome, " + loginData.userName);
            accept();
        } else {
            QMessageBox::warning(this, "Error", 
                "Login failed - check credentials");
        }
    }
    
private:
    CAuthorizationController* m_auth;
    Ui::LoginDialog* ui;
};
```

### QML Integration
```cpp
// Create QML-exposed wrapper
class AuthManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY loginChanged)
    Q_PROPERTY(QString currentUser READ currentUser NOTIFY loginChanged)
    
public:
    Q_INVOKABLE bool login(const QString& user, const QString& pass) {
        Login loginData;
        if (m_auth.Login(user, pass, loginData)) {
            m_currentUser = loginData.userName;
            emit loginChanged();
            return true;
        }
        return false;
    }
    
    Q_INVOKABLE bool hasPermission(const QString& perm) {
        return m_auth.HasPermission(perm.toUtf8());
    }
    
    bool isLoggedIn() const { return !m_currentUser.isEmpty(); }
    QString currentUser() const { return m_currentUser; }
    
signals:
    void loginChanged();
    
private:
    CAuthorizationController m_auth;
    QString m_currentUser;
};

// Register for QML
qmlRegisterType<AuthManager>("AuthClientSdk", 1, 0, "AuthManager");
```

## Performance Considerations

### Caching
```cpp
// Cache user permissions
class AuthCache {
public:
    QByteArrayList getUserPermissions(
        CAuthorizationController& auth, 
        const QByteArray& userId) {
        
        // Check cache
        if (m_permissionCache.contains(userId)) {
            auto entry = m_permissionCache[userId];
            if (!entry.isExpired()) {
                return entry.permissions;
            }
        }
        
        // Fetch from server
        QByteArrayList perms = auth.GetUserPermissions(userId);
        
        // Cache for 5 minutes
        m_permissionCache[userId] = CacheEntry{perms, QDateTime::currentDateTime().addSecs(300)};
        
        return perms;
    }
    
private:
    struct CacheEntry {
        QByteArrayList permissions;
        QDateTime expires;
        bool isExpired() const { 
            return QDateTime::currentDateTime() > expires; 
        }
    };
    
    QHash<QByteArray, CacheEntry> m_permissionCache;
};
```

### Batch Operations
```cpp
// Batch role assignments
void assignRolesToUsers(
    CAuthorizationController& auth,
    const QByteArrayList& userIds,
    const QByteArrayList& roleIds) {
    
    for (const auto& userId : userIds) {
        // Single call to add all roles to user
        auth.AddRolesToUser(userId, roleIds);
    }
}
```

## Platform Notes

### Windows Support
- Primary platform for AuthClientSdk
- WIN32 preprocessor conditional compilation
- Compatible with Visual C++ 2015/2017/2019

### Build Systems
- CMake: `Impl/AuthClientSdk/CMake`
- QMake: `Impl/AuthClientSdk/QMake`

### UI Components
The SDK includes QML widgets for login and administration:
- **LoginWidget**: Pre-built login UI component
- **AdministrationWidget**: User/role/group administration UI

See source files:
- `CLoginViewWidget.cpp/h`
- `CAdministrationViewWidget.cpp/h`

## Additional Resources

- **Main README**: [README.md](../README.md)
- **AuthServerSdk Documentation**: [AuthServerSdk.md](AuthServerSdk.md)
- **Doxygen API**: Generate with `doxygen Doxyfile` in Docs directory
- **ImtCore Documentation**: Refer to ImtCore framework documentation
- **Qt Documentation**: https://doc.qt.io/

## Version History

See Git commit history for detailed changes to the SDK.

## License

See the repository license file for licensing information.
