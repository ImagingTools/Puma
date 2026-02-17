// SPDX-License-Identifier: MIT
#pragma once


/**
* @file AuthClientSdk.h
* @brief Public API of the Authorization Client SDK.
*
* This header defines data models, configuration structures,
* enumerations, and the main `CAuthorizationController` class
* used by client applications to interact with an authorization server.
*
* The SDK provides a comprehensive API for:
* - User authentication (local and LDAP)
* - Permission-based authorization
* - User, role, and group management
* - Superuser account management
* - Secure server communication with optional SSL/TLS
*
* @section usage_example Basic Usage Example
* @code
* using namespace AuthClientSdk;
*
* // Create the authorization controller
* CAuthorizationController controller;
*
* // Configure server connection
* ServerConfig serverConfig;
* serverConfig.host = "auth.example.com";
* serverConfig.httpPort = 443;
* serverConfig.wsPort = 8443;
*
* // Optional: Configure SSL for secure communication
* SslConfig sslConfig;
* sslConfig.caCertificatePaths = {"/path/to/ca.pem"};
* sslConfig.protocol = QSsl::TlsV1_3;
* serverConfig.sslConfig = sslConfig;
*
* controller.SetConnectionParam(serverConfig);
* controller.SetProductId("MyProduct");
*
* // Authenticate user
* Login loginData;
* if (controller.Login("username", "password", loginData)) {
*     qDebug() << "Logged in as:" << loginData.userName;
*     qDebug() << "Access token:" << loginData.accessToken;
*     qDebug() << "Permissions:" << loginData.permissions;
*
*     // Check specific permission
*     if (controller.HasPermission("admin.users.create")) {
*         // User has permission to create other users
*     }
* }
* @endcode
*
* @section thread_safety Thread Safety
* The CAuthorizationController class is generally thread-safe for read operations
* when used from multiple threads. However, Login/Logout operations should not be
* called concurrently. User/role/group management operations may require external
* synchronization depending on the backend implementation.
*
* @ingroup AuthClientSdk
*/

#if defined _MSC_VER || defined __MINGW32__ || defined __MINGW64__
#define AUTH_CLIENT_SDK_EXPORT __declspec(dllexport)
#elif defined __GNUC__
#define AUTH_CLIENT_SDK_EXPORT __attribute__((visibility("default")))
#else
#define AUTH_CLIENT_SDK_EXPORT
#endif


// STL includes
#include <optional>

// Qt includes
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QByteArrayList>
#include <QtNetwork/QSslConfiguration>


namespace AuthClientSdk
{


class CAuthorizationControllerImpl;


/**
* @brief Login session data.
*
* Contains authentication and authorization information
* returned after a successful login. This structure holds
* all the necessary data for maintaining an authenticated
* session with the authorization server.
*
* @note All fields are populated by the Login() method and
*       should be treated as read-only by client code.
*
* @see CAuthorizationController::Login()
*/
struct Login
{
	/**
	* @brief Access token issued by the server.
	*
	* This JWT (JSON Web Token) or similar token is used to authenticate
	* subsequent API requests to the authorization server. It should be
	* included in HTTP Authorization headers for authenticated requests.
	*
	* @note The token has an expiration time defined by the server.
	*       Client applications should handle token refresh or re-authentication
	*       when the token expires.
	*/
	QByteArray accessToken;

	/**
	* @brief User display name.
	*/
	QString userName;

	/**
	* @brief Product identifier associated with the session.
	*/
	QByteArray productId;

	/**
	* @brief List of permission identifiers assigned to the user.
	*
	* Contains all permissions granted to the user through their assigned
	* roles and group memberships. Permission IDs are application-specific
	* strings that define what actions the user is authorized to perform.
	*
	* @note Permissions are aggregated from:
	*       - Direct role assignments
	*       - Group role assignments
	*       - Inherited permissions from parent groups
	*/
	QByteArrayList permissions;

	/**
	* @brief Clears all stored login data.
	*
	* Resets all fields to their default empty state. This is typically
	* called automatically by the Login() method before populating the
	* structure, or manually when ending a session.
	*
	* @note This does not perform a server-side logout operation.
	*       Use CAuthorizationController::Logout() for that purpose.
	*/
	void Clear()
	{
		accessToken.clear();
		userName.clear();
		productId.clear();
		permissions.clear();
	}
};


/**
* @brief User entity description.
*
* Represents a user account in the authorization system. Users can be
* assigned roles directly or through group membership, which determines
* their permissions.
*
* @note User accounts can be authenticated through local storage or
*       external systems like LDAP, as indicated by GetUserAuthSystem().
*/
struct User
{
	/**
	* @brief User name.
	*/
	QString name;

	/**
	* @brief User email address.
	*/
	QString email;

	/**
	* @brief User login identifier.
	*
	* Unique identifier used for authentication. This is the username
	* entered during login and is distinct from the internal user ID.
	*/
	QByteArray login;

	/**
	* @brief List of role identifiers assigned to the user.
	*
	* Contains IDs of roles directly assigned to this user for the
	* current product. Does not include roles inherited through groups.
	*
	* @see AddRolesToUser(), RemoveRolesFromUser()
	*/
	QByteArrayList roleIds;

	/**
	* @brief List of group identifiers the user belongs to.
	*
	* Groups provide a way to organize users and assign roles collectively.
	* Users inherit all permissions from roles assigned to their groups.
	*
	* @see AddUsersToGroup(), RemoveUsersFromGroup()
	*/
	QByteArrayList groupIds;
};


/**
* @brief Role entity description.
*
* Roles are collections of permissions that can be assigned to users
* or groups. They provide a way to manage authorization by grouping
* related permissions together.
*
* @note Roles are product-specific. The same role name in different
*       products may have different permissions.
*/
struct Role
{
	/**
	* @brief Role name.
	*/
	QString name;

	/**
	* @brief Human-readable role description.
	*/
	QString description;

	/**
	* @brief List of permission identifiers assigned to the role.
	*
	* Permissions define specific capabilities or actions that users
	* with this role are authorized to perform. Permission IDs are
	* application-defined strings (e.g., "admin.users.create").
	*
	* @see AddPermissionsToRole(), RemovePermissionsFromRole()
	*/
	QByteArrayList permissionIds;
};


/**
* @brief Group entity description.
*
* Groups allow organizing users and assigning roles collectively.
* This simplifies permission management by allowing bulk assignments
* rather than managing each user individually.
*
* @note When a user is added to a group, they automatically inherit
*       all permissions from the roles assigned to that group.
*/
struct Group
{
	/**
	* @brief Group name.
	*/
	QString name;

	/**
	* @brief Human-readable group description.
	*/
	QString description;

	/**
	* @brief List of user identifiers belonging to the group.
	*
	* All users in this list are members of the group and inherit
	* permissions from the group's assigned roles.
	*
	* @see AddUsersToGroup(), RemoveUsersFromGroup()
	*/
	QByteArrayList userIds;

	/**
	* @brief List of role identifiers assigned to the group.
	*
	* All group members automatically receive permissions from these roles.
	* This provides an efficient way to manage permissions for teams or
	* departments.
	*
	* @see AddRolesToGroup(), RemoveRolesFromGroup()
	*/
	QByteArrayList roleIds;
};


/**
* @brief Superuser existence status.
*
* Indicates whether a superuser account exists in the authorization system.
* A superuser is a privileged account with full administrative access.
*
* @note The superuser account is typically created during initial system
*       setup and is used for bootstrapping user management.
*/
enum class SuperuserStatus
{
	Unknown,    /**< Status cannot be determined (e.g., server unreachable)*/
	Exists,     /**< Superuser account exists and is active*/
	NotExists   /**< Superuser account does not exist and needs to be created*/
};


/**
* @brief Authentication system type.
*
* Specifies the backend authentication mechanism used for a user account.
* The system supports both local (database) authentication and external
* LDAP directory integration.
*/
enum class SystemType
{
	Unknown, /**< Authentication system could not be determined*/
	Local,   /**< Local database authentication*/
	Ldap     /**< LDAP directory authentication (Active Directory, OpenLDAP, etc.)*/
};


/**
* @brief SSL/TLS client configuration.
*
* Defines SSL parameters for secure communication with the server.
* When this structure is provided in ServerConfig, the client enables
* secure HTTPS/WSS connections by setting the CF_SECURE flag on the
* underlying connection interface. If not provided, the client uses
* unencrypted HTTP/WebSocket connections.
*
* @note The actual SSL configuration and certificate handling is managed
*       by the underlying ImtCore connection interface (IServerConnectionInterface).
*       The fields in this structure are available for advanced scenarios
*       (such as mutual TLS with client certificates) but their usage depends
*       on the connection interface implementation. For basic HTTPS client
*       connections, simply providing this structure (even with default values)
*       is typically sufficient to enable secure connections.
*
* @warning For production environments, SSL should always be enabled
*          to protect authentication credentials and sensitive data.
*
* @see ServerConfig, SetConnectionParam()
*/
struct SslConfig
{
	/**
	* @brief Paths to trusted CA certificates.
	*
	* Optional file paths to Certificate Authority (CA) certificates for
	* verifying the server's SSL certificate.
	*
	* @note Usage depends on the connection interface implementation.
	*/
	QList<QString> caCertificatePaths;

	/**
	* @brief Encoding format of CA certificates.
	*
	* Specifies whether certificates are in PEM (Base64-encoded) or
	* DER (binary) format. Default is PEM, which is most common.
	*
	* @note Usage depends on the connection interface implementation.
	*/
	QSsl::EncodingFormat caCertificateFormat = QSsl::Pem;

	/**
	* @brief SSL/TLS protocol version.
	*
	* Specifies the minimum TLS version to use. TLS 1.2 or higher is
	* recommended for security. Default is TLS 1.2.
	*
	* @note Older protocols like SSLv2, SSLv3, and TLS 1.0 are deprecated
	*       and should not be used due to known security vulnerabilities.
	*       Usage depends on the connection interface implementation.
	*/
	QSsl::SslProtocol protocol = QSsl::TlsV1_2;

	/**
	* @brief Ignore SSL validation errors.
	*
	* When true, SSL certificate validation errors may be ignored by the
	* connection interface. This allows connections to servers with
	* self-signed or expired certificates.
	*
	* @warning This setting should be false in production environments.
	*          Only set to true for testing with self-signed certificates.
	*          Ignoring SSL errors exposes the application to man-in-the-middle
	*          attacks and defeats the purpose of using SSL/TLS.
	*
	* @note Usage depends on the connection interface implementation.
	*/
	bool ignoreSslErrors = false;
};


/**
* @brief Server connection configuration.
*
* Contains network parameters for connecting to the authorization server.
* Both HTTP and WebSocket connections are supported for different
* communication patterns.
*
* @note The SDK uses HTTP for REST API calls and WebSocket for
*       real-time notifications and bidirectional communication.
*/
struct ServerConfig
{
	/**
	* @brief Server host name or IP address.
	*
	* Can be a domain name (e.g., "auth.example.com"), IPv4 address
	* (e.g., "192.168.1.100"), or IPv6 address (e.g., "::1").
	* Default is "localhost" for local development.
	*/
	QString host = "localhost";

	/**
	* @brief HTTP port number.
	*
	* Port used for HTTP/HTTPS REST API communication.
	* Standard ports: 80 (HTTP), 443 (HTTPS).
	*/
	int httpPort;

	/**
	* @brief WebSocket port number.
	*
	* Port used for WebSocket/WSS bidirectional communication.
	* Common ports: 8080 (WS), 8443 (WSS).
	*/
	int wsPort;

	/**
	* @brief Optional SSL configuration.
	*
	* If set, HTTPS and WSS (secure WebSocket) connections are used.
	* If not set, plain HTTP and WS connections are used.
	*
	* @warning Always configure SSL for production deployments to
	*          protect credentials and sensitive authorization data.
	*/
	std::optional<SslConfig> sslConfig;
};


/**
* @brief Authorization controller.
*
* Provides a high-level client-side API for authentication,
* authorization, and user/role/group management. This is the main
* entry point for all authorization-related operations in client
* applications.
*
* The controller acts as a facade over the underlying authorization
* system, providing a simplified and stable interface. It handles:
* - User authentication (Login/Logout)
* - Permission checking
* - User account management (CRUD operations)
* - Role and permission management
* - Group management
* - Superuser account setup
*
* @section implementation_notes Implementation Details
*
* Uses the PIMPL (Pointer to Implementation) pattern to hide
* implementation details and preserve ABI (Application Binary Interface)
* compatibility across library versions. This allows the SDK to be
* updated without requiring client applications to recompile.
*
* @section lifecycle Lifecycle
*
* 1. Create a CAuthorizationController instance
* 2. Configure server connection with SetConnectionParam()
* 3. Set product ID with SetProductId()
* 4. Call Login() to authenticate a user
* 5. Perform authorized operations (check permissions, manage users, etc.)
* 6. Call Logout() when done
*
* @section thread_safety Thread Safety
*
* This class is generally thread-safe for read operations. However,
* Login/Logout should not be called concurrently from multiple threads.
* User/role/group management operations may require external synchronization.
*
* @see Login, User, Role, Group, ServerConfig
*/
class AUTH_CLIENT_SDK_EXPORT CAuthorizationController
{
public:
	/**
	* @brief Constructs the authorization controller.
	*
	* Creates a new controller instance with default settings.
	* The controller is ready to use after construction, but
	* server connection parameters should be configured before
	* attempting to login.
	*
	* @note This constructor allocates internal resources.
	*       Always ensure the object is properly destroyed when done.
	*
	* @see SetConnectionParam(), SetProductId()
	*/
	CAuthorizationController();

	/**
	* @brief Virtual destructor.
	*
	* Destroys the controller and releases all associated resources.
	* If a user is currently logged in, this does NOT automatically
	* call Logout() - you should explicitly logout before destruction
	* if server-side session cleanup is required.
	*/
	virtual ~CAuthorizationController();

	/**
	* @brief Authenticates a user.
	*
	* Attempts to authenticate the user with the provided credentials
	* against the authorization server. On success, populates the output
	* structure with session data including access token, username,
	* product ID, and permissions.
	*
	* @param login User login identifier (username).
	* @param password User password in plain text (transmitted securely if SSL is enabled).
	* @param[out] out Output structure to receive login session data.
	*                 Cleared before population. Only valid if method returns true.
	*
	* @return true if authentication succeeded and session data was retrieved.
	* @return false if authentication failed due to:
	*         - Invalid credentials
	*         - Server communication error
	*         - Missing required interfaces
	*         - User account disabled or locked
	*
	* @note This method clears the output structure before attempting login.
	*       The access token in the output should be stored securely and used
	*       for subsequent authenticated API calls.
	*
	* @warning Passwords are transmitted in plain text unless SSL is configured.
	*          Always use HTTPS in production environments.
	*
	* @see Logout(), HasPermission(), GetToken()
	*
	* @par Example:
	* @code
	* CAuthorizationController controller;
	* controller.SetConnectionParam(serverConfig);
	* 
	* Login sessionData;
	* if (controller.Login("alice", "secret123", sessionData)) {
	*     qDebug() << "Welcome, " << sessionData.userName;
	*     qDebug() << "Your permissions:" << sessionData.permissions;
	* } else {
	*     qDebug() << "Login failed";
	* }
	* @endcode
	*/
	virtual bool Login(const QString& login, const QString& password, Login& out) const;

	/**
	* @brief Logs out the current user.
	*
	* Terminates the current user session on the server and invalidates
	* the access token. After logout, permission checks will fail and
	* user-specific operations will require re-authentication.
	*
	* @return true if logout succeeded or no user was logged in.
	* @return false if logout failed due to server communication error.
	*
	* @note This method should be called before destroying the controller
	*       if proper session cleanup is required. However, access tokens
	*       will eventually expire on their own if not explicitly revoked.
	*
	* @see Login()
	*/
	virtual bool Logout() const;

	/**
	* @brief Sets server connection parameters.
	*
	* Configures the network settings for communicating with the
	* authorization server. This must be called before attempting
	* to login or perform any server operations.
	*
	* When SSL configuration is provided, this method enables secure
	* connections by setting the CF_SECURE flag on the underlying
	* IServerConnectionInterface. The detailed SSL configuration fields
	* (certificates, protocols, etc.) are available for advanced scenarios
	* but their actual usage depends on the connection interface implementation.
	*
	* @param config Server configuration including host, ports, and
	*               optional SSL settings.
	*
	* @return true if parameters were applied successfully.
	* @return false if the configuration could not be applied (e.g.,
	*         required interface not available).
	*
	* @note This method can be called multiple times to update connection
	*       parameters, but should not be called while operations are in
	*       progress.
	*
	* @note For basic HTTPS client connections, providing an SslConfig
	*       structure (even with default values) is typically sufficient
	*       to enable secure communication.
	*
	* @see ServerConfig, SslConfig
	*
	* @par Example:
	* @code
	* ServerConfig config;
	* config.host = "auth.example.com";
	* config.httpPort = 443;
	* config.wsPort = 8443;
	* 
	* SslConfig ssl;
	* ssl.caCertificatePaths = {"/etc/ssl/certs/ca.pem"};
	* ssl.protocol = QSsl::TlsV1_3;
	* config.sslConfig = ssl;
	* 
	* controller.SetConnectionParam(config);
	* @endcode
	*/
	bool SetConnectionParam(const ServerConfig& config) const;

	/**
	* @brief Checks whether the current user has a specific permission.
	*
	* Verifies if the currently authenticated user has been granted the
	* specified permission through their assigned roles and group memberships.
	*
	* @param permissionId Application-defined permission identifier
	*                     (e.g., "admin.users.create", "data.read").
	*
	* @return true if the user has the specified permission.
	* @return false if:
	*         - User does not have the permission
	*         - No user is currently logged in
	*         - Permission check interface is unavailable
	*
	* @note Permission IDs are case-sensitive and application-specific.
	*       They should match the permissions defined in the server's
	*       authorization configuration.
	*
	* @see Login(), GetUserPermissions()
	*/
	virtual bool HasPermission(const QByteArray& permissionId) const;

	/**
	* @brief Returns the current access token.
	*
	* Retrieves the JWT (JSON Web Token) or similar access token for the
	* currently authenticated user. This token should be included in HTTP
	* Authorization headers for authenticated API requests.
	*
	* @return Current access token as a byte array.
	* @return Empty QByteArray if no user is logged in or token is unavailable.
	*
	* @note The token has a server-defined expiration time. Applications
	*       should handle token expiration by re-authenticating when
	*       requests fail with authorization errors.
	*
	* @see Login()
	*/
	virtual QByteArray GetToken() const;

	/**
	* @brief Sets the product identifier.
	*
	* Configures the product ID used for product-specific authorization
	* and licensing. Roles and permissions are often scoped to specific
	* products, so this must be set appropriately for the application.
	*
	* @param productId Binary product identifier (e.g., "MyApp", "ProductX").
	*
	* @note This should typically be called once during initialization,
	*       before login. Changing the product ID may affect which
	*       permissions are available to users.
	*
	* @see Login()
	*/
	virtual void SetProductId(const QByteArray& productId) const;

	/**
	* @brief Checks whether a superuser exists.
	*
	* Queries the server to determine if a superuser account has been
	* created. The superuser is a privileged account used for initial
	* system setup and administrative tasks.
	*
	* @param[out] errorMessage Receives error description if status is Unknown.
	*
	* @return SuperuserStatus::Exists if a superuser account exists.
	* @return SuperuserStatus::NotExists if no superuser has been created yet.
	* @return SuperuserStatus::Unknown if the status could not be determined
	*         (server unreachable, interface unavailable, etc.).
	*
	* @note This is typically used during initial application setup to
	*       determine if the superuser needs to be created.
	*
	* @see CreateSuperuser(), SuperuserStatus
	*/
	virtual SuperuserStatus SuperuserExists(QString& errorMessage) const;

	/**
	* @brief Creates a superuser account.
	*
	* Creates the initial superuser account with full administrative
	* privileges. This should only be called during initial system setup
	* when no superuser exists.
	*
	* @param password Superuser password. Should be strong and securely stored.
	*
	* @return true if the superuser was created successfully.
	* @return false if creation failed (superuser already exists, invalid
	*         password, server error, etc.).
	*
	* @warning This is a sensitive operation that grants full system access.
	*          Only call this during initial setup and ensure the password
	*          is properly secured.
	*
	* @note After creating the superuser, you can login with username
	*       "superuser" (or as defined by the server) and the provided password.
	*
	* @see SuperuserExists()
	*/
	virtual bool CreateSuperuser(const QByteArray& password) const;

	/**
	* @brief Returns all user identifiers.
	*
	* Retrieves a list of all user object IDs registered in the system.
	* These IDs can be used with GetUser() to fetch detailed user information.
	*
	* @return List of user object identifiers (internal database IDs).
	* @return Empty list if no users exist or operation failed.
	*
	* @note Requires appropriate administrative permissions.
	*       The returned IDs are internal object identifiers, not login names.
	*
	* @see GetUser(), GetUserByLogin()
	*/
	virtual QByteArrayList GetUserIds() const;

	/**
	* @brief Retrieves user data by user ID.
	*
	* Fetches complete information for a specific user, including name,
	* email, login identifier, assigned roles, and group memberships.
	*
	* @param userId User object identifier (from GetUserIds()).
	* @param[out] userData Output structure to receive user information.
	*
	* @return true if user was found and data was retrieved.
	* @return false if user doesn't exist or operation failed.
	*
	* @note Requires appropriate permissions to view user data.
	*
	* @see GetUserIds(), GetUserByLogin(), User
	*/
	virtual bool GetUser(const QByteArray& userId, User& userData) const;

	/**
	* @brief Retrieves user data by login.
	*
	* Fetches user information using the login identifier (username)
	* instead of the internal user object ID.
	*
	* @param login User login identifier (username).
	* @param[out] userData Output structure to receive user information.
	*
	* @return true if user was found and data was retrieved.
	* @return false if user doesn't exist or operation failed.
	*
	* @note This is often more convenient than GetUser() when you have
	*       the username but not the internal user ID.
	*
	* @see GetUser(), User
	*/
	virtual bool GetUserByLogin(const QByteArray& login, User& userData) const;

	/**
	* @brief Removes a user.
	*
	* Permanently deletes a user account from the system. This operation
	* cannot be undone. All role and group assignments for this user are
	* also removed.
	*
	* @param userId User object identifier to remove.
	*
	* @return true if user was successfully removed.
	* @return false if removal failed (user not found, insufficient permissions, etc.).
	*
	* @warning This is a destructive operation. Ensure proper authorization
	*          and confirmation before deleting user accounts.
	*
	* @note Requires administrative permissions.
	*/
	virtual bool RemoveUser(const QByteArray& userId) const;

	/**
	* @brief Creates a new user.
	*
	* Creates a new user account with the specified credentials and details.
	* The new user can then be assigned to roles and groups.
	*
	* @param userName Display name for the user (e.g., "John Doe").
	* @param login Unique login identifier (username for authentication).
	* @param password Initial password for the user account.
	* @param email User's email address.
	*
	* @return User object identifier of the created user.
	* @return Empty QByteArray if creation failed (login already exists,
	*         invalid parameters, insufficient permissions, etc.).
	*
	* @note Requires administrative permissions.
	*       The login identifier must be unique across all users.
	*
	* @see RemoveUser(), ChangeUserPassword()
	*/
	virtual QByteArray CreateUser(
		const QString& userName,
		const QByteArray& login,
		const QByteArray& password,
		const QString& email) const;

	/**
	* @brief Changes user password.
	*
	* Updates the password for a user account. Requires knowledge of the
	* current password for security verification.
	*
	* @param login User login identifier.
	* @param oldPassword Current password (for verification).
	* @param newPassword New password to set.
	*
	* @return true if password was changed successfully.
	* @return false if operation failed (wrong old password, user not found,
	*         password policy violation, etc.).
	*
	* @note For security, this operation requires the old password to
	*       prevent unauthorized password changes. Administrators may
	*       have different mechanisms to reset passwords without the
	*       old password.
	*/
	virtual bool ChangeUserPassword(
		const QByteArray& login,
		const QByteArray& oldPassword,
		const QByteArray& newPassword) const;

	/**
	* @brief Adds roles to a user.
	*
	* Assigns one or more roles to a user for the current product.
	* The user immediately gains all permissions associated with these roles.
	*
	* @param userId User object identifier.
	* @param roleIds List of role identifiers to assign.
	*
	* @return true if roles were added successfully.
	* @return false if operation failed (user not found, role not found,
	*         insufficient permissions, etc.).
	*
	* @note Requires administrative permissions.
	*       Roles are product-specific based on the current product ID.
	*
	* @see RemoveRolesFromUser(), SetProductId()
	*/
	virtual bool AddRolesToUser(const QByteArray& userId, const QByteArrayList& roleIds) const;

	/**
	* @brief Removes roles from a user.
	*
	* Unassigns one or more roles from a user. The user loses permissions
	* associated with these roles unless they have them through other roles
	* or group memberships.
	*
	* @param userId User object identifier.
	* @param roleIds List of role identifiers to remove.
	*
	* @return true if roles were removed successfully.
	* @return false if operation failed (user not found, role not assigned,
	*         insufficient permissions, etc.).
	*
	* @note Requires administrative permissions.
	*       Removing a role doesn't affect permissions from other sources.
	*
	* @see AddRolesToUser()
	*/
	virtual bool RemoveRolesFromUser(const QByteArray& userId, const QByteArrayList& roleIds) const;

	/**
	* @brief Returns user permissions.
	*
	* Retrieves all permissions granted to a user for the current product,
	* aggregated from all their assigned roles (both direct and through groups).
	*
	* @param userId User object identifier.
	*
	* @return List of permission identifiers the user has.
	* @return Empty list if user not found or has no permissions.
	*
	* @note This returns the effective permissions after resolving all
	*       role and group assignments.
	*
	* @see HasPermission()
	*/
	virtual QByteArrayList GetUserPermissions(const QByteArray& userId) const;

	/**
	* @brief Returns authentication system used by the user.
	*
	* Determines whether a user account uses local (database) authentication
	* or external LDAP directory authentication.
	*
	* @param login User login identifier.
	*
	* @return SystemType::Local if user authenticates locally.
	* @return SystemType::Ldap if user authenticates via LDAP.
	* @return SystemType::Unknown if system type cannot be determined.
	*
	* @note LDAP users are typically managed in the external directory,
	*       while local users are fully managed within this system.
	*
	* @see SystemType
	*/
	virtual SystemType GetUserAuthSystem(const QByteArray& login) const;

	/**
	* @brief Returns all role identifiers.
	*
	* Retrieves a list of all role IDs defined in the system for the
	* current product. These IDs can be used with GetRole() for details.
	*
	* @return List of role identifiers.
	* @return Empty list if no roles exist or operation failed.
	*
	* @note Roles are product-specific. Only roles for the current product
	*       (set via SetProductId()) are returned.
	*
	* @see GetRole(), CreateRole()
	*/
	virtual QByteArrayList GetRoleIds() const;

	/**
	* @brief Retrieves role data.
	*
	* Fetches complete information for a specific role, including name,
	* description, and assigned permissions.
	*
	* @param roleId Role identifier.
	* @param[out] roleData Output structure to receive role information.
	*
	* @return true if role was found and data was retrieved.
	* @return false if role doesn't exist or operation failed.
	*
	* @see GetRoleIds(), CreateRole(), Role
	*/
	virtual bool GetRole(const QByteArray& roleId, Role& roleData) const;

	/**
	* @brief Creates a role.
	*
	* Creates a new role with the specified name, description, and initial
	* set of permissions. The role can then be assigned to users or groups.
	*
	* @param roleName Human-readable role name (e.g., "Administrator", "Viewer").
	* @param roleDescription Optional description explaining the role's purpose.
	* @param permissions Optional list of permission IDs to assign initially.
	*
	* @return Role identifier of the created role.
	* @return Empty QByteArray if creation failed.
	*
	* @note Requires administrative permissions.
	*       Roles are created for the current product ID.
	*       Permissions can be added later using AddPermissionsToRole().
	*
	* @see RemoveRole(), AddPermissionsToRole()
	*/
	virtual QByteArray CreateRole(
		const QString& roleName,
		const QString& roleDescription = QString(),
		const QByteArrayList& permissions = QByteArrayList());

	/**
	* @brief Removes a role.
	*
	* Permanently deletes a role from the system. All assignments of this
	* role to users and groups are also removed.
	*
	* @param roleId Role identifier to remove.
	*
	* @return true if role was successfully removed.
	* @return false if removal failed.
	*
	* @warning This is a destructive operation. Users and groups with this
	*          role will lose associated permissions.
	*
	* @note Requires administrative permissions.
	*
	* @see CreateRole()
	*/
	virtual bool RemoveRole(const QByteArray& roleId) const;

	/**
	* @brief Returns permissions assigned to a role.
	*
	* Retrieves the list of permission IDs directly assigned to this role.
	*
	* @param roleId Role identifier.
	*
	* @return List of permission identifiers.
	* @return Empty list if role not found or has no permissions.
	*
	* @see AddPermissionsToRole(), RemovePermissionsFromRole()
	*/
	virtual QByteArrayList GetRolePermissions(const QByteArray& roleId) const;

	/**
	* @brief Adds permissions to a role.
	*
	* Assigns one or more permissions to a role. Users and groups with this
	* role immediately gain these permissions.
	*
	* @param roleId Role identifier.
	* @param permissions List of permission IDs to add.
	*
	* @return true if permissions were added successfully.
	* @return false if operation failed.
	*
	* @note Requires administrative permissions.
	*
	* @see RemovePermissionsFromRole(), CreateRole()
	*/
	virtual bool AddPermissionsToRole(
		const QByteArray& roleId,
		const QByteArrayList& permissions) const;

	/**
	* @brief Removes permissions from a role.
	*
	* Unassigns one or more permissions from a role. Users and groups with
	* only this role as a source for these permissions will lose them.
	*
	* @param roleId Role identifier.
	* @param permissions List of permission IDs to remove.
	*
	* @return true if permissions were removed successfully.
	* @return false if operation failed.
	*
	* @note Requires administrative permissions.
	*
	* @see AddPermissionsToRole()
	*/
	virtual bool RemovePermissionsFromRole(
		const QByteArray& roleId,
		const QByteArrayList& permissions) const;

	/**
	* @brief Returns all group identifiers.
	*
	* Retrieves a list of all group IDs defined in the system.
	* These IDs can be used with GetGroup() for details.
	*
	* @return List of group identifiers.
	* @return Empty list if no groups exist or operation failed.
	*
	* @see GetGroup(), CreateGroup()
	*/
	virtual QByteArrayList GetGroupIds() const;

	/**
	* @brief Creates a group.
	*
	* Creates a new user group with the specified name and description.
	* Users and roles can then be added to the group.
	*
	* @param groupName Human-readable group name (e.g., "Developers", "Managers").
	* @param description Optional description explaining the group's purpose.
	*
	* @return Group identifier of the created group.
	* @return Empty QByteArray if creation failed.
	*
	* @note Requires administrative permissions.
	*       Users and roles can be added using AddUsersToGroup() and
	*       AddRolesToGroup() respectively.
	*
	* @see RemoveGroup(), AddUsersToGroup(), AddRolesToGroup()
	*/
	virtual QByteArray CreateGroup(const QString& groupName, const QString& description) const;

	/**
	* @brief Removes a group.
	*
	* Permanently deletes a group from the system. User memberships are
	* removed, but the users themselves are not deleted.
	*
	* @param groupId Group identifier to remove.
	*
	* @return true if group was successfully removed.
	* @return false if removal failed.
	*
	* @warning Users in this group will lose permissions granted through
	*          the group's role assignments.
	*
	* @note Requires administrative permissions.
	*
	* @see CreateGroup()
	*/
	virtual bool RemoveGroup(const QByteArray& groupId) const;

	/**
	* @brief Retrieves group data.
	*
	* Fetches complete information for a specific group, including name,
	* description, member users, and assigned roles.
	*
	* @param groupId Group identifier.
	* @param[out] groupData Output structure to receive group information.
	*
	* @return true if group was found and data was retrieved.
	* @return false if group doesn't exist or operation failed.
	*
	* @see GetGroupIds(), CreateGroup(), Group
	*/
	virtual bool GetGroup(const QByteArray& groupId, Group& groupData) const;

	/**
	* @brief Adds users to a group.
	*
	* Adds one or more users to a group. Users immediately gain permissions
	* from all roles assigned to the group.
	*
	* @param groupId Group identifier.
	* @param userIds List of user IDs to add to the group.
	*
	* @return true if users were added successfully.
	* @return false if operation failed.
	*
	* @note Requires administrative permissions.
	*
	* @see RemoveUsersFromGroup(), AddRolesToGroup()
	*/
	virtual bool AddUsersToGroup(
		const QByteArray& groupId,
		const QByteArrayList& userIds) const;

	/**
	* @brief Removes users from a group.
	*
	* Removes one or more users from a group. Users lose permissions
	* granted through the group's roles, unless they have them from
	* other sources.
	*
	* @param groupId Group identifier.
	* @param userIds List of user IDs to remove from the group.
	*
	* @return true if users were removed successfully.
	* @return false if operation failed.
	*
	* @note Requires administrative permissions.
	*
	* @see AddUsersToGroup()
	*/
	virtual bool RemoveUsersFromGroup(
		const QByteArray& groupId,
		const QByteArrayList& userIds) const;

	/**
	* @brief Adds roles to a group.
	*
	* Assigns one or more roles to a group. All group members immediately
	* gain permissions from these roles.
	*
	* @param groupId Group identifier.
	* @param roleIds List of role IDs to assign to the group.
	*
	* @return true if roles were added successfully.
	* @return false if operation failed.
	*
	* @note Requires administrative permissions.
	*       This is an efficient way to grant permissions to many users at once.
	*
	* @see RemoveRolesFromGroup(), AddUsersToGroup()
	*/
	virtual bool AddRolesToGroup(
		const QByteArray& groupId,
		const QByteArrayList& roleIds) const;

	/**
	* @brief Removes roles from a group.
	*
	* Unassigns one or more roles from a group. Group members lose
	* permissions from these roles unless they have them from other sources.
	*
	* @param groupId Group identifier.
	* @param roleIds List of role IDs to remove from the group.
	*
	* @return true if roles were removed successfully.
	* @return false if operation failed.
	*
	* @note Requires administrative permissions.
	*
	* @see AddRolesToGroup()
	*/
	virtual bool RemoveRolesFromGroup(
		const QByteArray& groupId,
		const QByteArrayList& roleIds) const;

private:
	/**
	* @brief Pointer to the internal implementation.
	*
	* This is part of the PIMPL (Pointer to Implementation) pattern,
	* which hides implementation details from the public API. This allows
	* the SDK to evolve internally without breaking binary compatibility
	* with client applications.
	*
	* @note Clients should never directly access or modify this pointer.
	*/
	CAuthorizationControllerImpl* m_implPtr;
};


} // namespace AuthClientSdk


/**
* @defgroup AuthClientSdk Authorization Client SDK
*
* @brief Client-side API for authentication and authorization.
*
* The Authorization Client SDK provides a comprehensive C++ interface
* for integrating authentication and authorization capabilities into
* client applications. It supports:
*
* - **Authentication**: Local and LDAP-based user authentication
* - **Authorization**: Permission-based access control
* - **User Management**: Create, modify, and delete user accounts
* - **Role Management**: Define roles with specific permissions
* - **Group Management**: Organize users and assign roles collectively
* - **Secure Communication**: Optional SSL/TLS encryption
*
* The SDK uses a client-server architecture where this library acts as
* the client communicating with a remote authorization server via HTTP/HTTPS
* and WebSocket protocols.
*
* @section key_concepts Key Concepts
*
* - **Users**: Individual accounts with login credentials
* - **Roles**: Named collections of permissions
* - **Groups**: Collections of users that inherit role permissions
* - **Permissions**: Granular capabilities (e.g., "admin.users.create")
* - **Product ID**: Scope for product-specific authorization rules
*
* @section getting_started Getting Started
*
* 1. Create a CAuthorizationController instance
* 2. Configure server connection and SSL if needed
* 3. Set the product identifier
* 4. Authenticate users with Login()
* 5. Check permissions with HasPermission()
* 6. Manage users, roles, and groups as needed
*
* @see CAuthorizationController, Login, User, Role, Group
*/
