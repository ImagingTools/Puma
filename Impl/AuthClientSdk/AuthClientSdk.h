#pragma once


/**
* @file AuthClientSdk.h
* @brief Public API of the Authorization Client SDK.
*
* This header defines data models, configuration structures,
* enumerations, and the main `CAuthorizationController` class
* used by client applications to interact with an authorization server.
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
* returned after a successful login.
*/
struct Login
{
	/**
	* @brief Access token issued by the server.
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
	*/
	QByteArrayList permissions;

	/**
	* @brief Clears all stored login data.
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
	*/
	QByteArray login;

	/**
	* @brief List of role identifiers assigned to the user.
	*/
	QByteArrayList roleIds;

	/**
	* @brief List of group identifiers the user belongs to.
	*/
	QByteArrayList groupIds;
};


/**
* @brief Role entity description.
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
	*/
	QByteArrayList permissionIds;
};


/**
* @brief Group entity description.
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
	*/
	QByteArrayList userIds;

	/**
	* @brief List of role identifiers assigned to the group.
	*/
	QByteArrayList roleIds;
};


/**
* @brief Superuser existence status.
*/
enum class SuperuserStatus
{
	Unknown,    /**< Status cannot be determined*/
	Exists,     /**< Superuser exists*/
	NotExists   /**< Superuser does not exist*/
};


/**
* @brief Authentication system type.
*/
enum class SystemType
{
	Unknown, /**< Unknown authentication system*/
	Local,   /**< Local authentication*/
	Ldap     /**< LDAP-based authentication*/
};


/**
* @brief SSL/TLS client configuration.
*
* Defines SSL parameters for secure communication with the server.
*/
struct SslConfig
{
	/**
	* @brief Paths to trusted CA certificates.
	*/
	QList<QString> caCertificatePaths;

	/**
	* @brief Encoding format of CA certificates.
	*/
	QSsl::EncodingFormat caCertificateFormat = QSsl::Pem;

	/**
	* @brief SSL/TLS protocol version.
	*/
	QSsl::SslProtocol protocol = QSsl::TlsV1_2;

	/**
	* @brief Ignore SSL validation errors.
	*
	* Intended for testing environments only.
	*/
	bool ignoreSslErrors = false;
};


/**
* @brief Server connection configuration.
*/
struct ServerConfig
{
	/**
	* @brief Server host name or IP address.
	*/
	QString host = "localhost";

	/**
	* @brief HTTP port number.
	*/
	int httpPort;

	/**
	* @brief WebSocket port number.
	*/
	int wsPort;

	/**
	* @brief Optional SSL configuration.
	*
	* If not set, an unencrypted connection is used.
	*/
	std::optional<SslConfig> sslConfig;
};


/**
* @brief Authorization controller.
*
* Provides a high-level client-side API for authentication,
* authorization, and user/role/group management.
*
* Uses the PIMPL pattern to hide implementation details
* and preserve ABI compatibility.
*/
class AUTH_CLIENT_SDK_EXPORT CAuthorizationController
{
public:
	/**
	* @brief Constructs the authorization controller.
	*/
	CAuthorizationController();

	/**
	* @brief Virtual destructor.
	*/
	virtual ~CAuthorizationController();

	/**
	* @brief Authenticates a user.
	*
	* @param login User login identifier.
	* @param password User password.
	* @param out Output structure containing login session data.
	* @return true if authentication succeeded, false otherwise.
	*/
	virtual bool Login(const QString& login, const QString& password, Login& out) const;

	/**
	* @brief Logs out the current user.
	*
	* @return true if logout succeeded, false otherwise.
	*/
	virtual bool Logout() const;

	/**
	* @brief Sets server connection parameters.
	*
	* @param config Server configuration.
	* @return true if parameters were applied successfully.
	*/
	bool SetConnectionParam(const ServerConfig& config) const;

	/**
	* @brief Checks whether the current user has a specific permission.
	*
	* @param permissionId Permission identifier.
	* @return true if permission is granted, false otherwise.
	*/
	virtual bool HasPermission(const QByteArray& permissionId) const;

	/**
	* @brief Returns the current access token.
	*/
	virtual QByteArray GetToken() const;

	/**
	* @brief Sets the product identifier.
	*
	* @param productId Product identifier.
	*/
	virtual void SetProductId(const QByteArray& productId) const;

	/**
	* @brief Checks whether a superuser exists.
	*
	* @param errorMessage Output error description, if any.
	* @return Superuser existence status.
	*/
	virtual SuperuserStatus SuperuserExists(QString& errorMessage) const;

	/**
	* @brief Creates a superuser account.
	*
	* @param password Superuser password.
	* @return true if creation succeeded, false otherwise.
	*/
	virtual bool CreateSuperuser(const QByteArray& password) const;

	/**
	* @brief Returns all user identifiers.
	*/
	virtual QByteArrayList GetUserIds() const;

	/**
	* @brief Retrieves user data by user ID.
	*/
	virtual bool GetUser(const QByteArray& userId, User& userData) const;

	/**
	* @brief Retrieves user data by login.
	*/
	virtual bool GetUserByLogin(const QByteArray& login, User& userData) const;

	/**
	* @brief Removes a user.
	*/
	virtual bool RemoveUser(const QByteArray& userId) const;

	/**
	* @brief Creates a new user.
	*
	* @return Created user identifier.
	*/
	virtual QByteArray CreateUser(
		const QString& userName,
		const QByteArray& login,
		const QByteArray& password,
		const QString& email) const;

	/**
	* @brief Changes user password.
	*/
	virtual bool ChangeUserPassword(
		const QByteArray& login,
		const QByteArray& oldPassword,
		const QByteArray& newPassword) const;

	/**
	* @brief Adds roles to a user.
	*/
	virtual bool AddRolesToUser(const QByteArray& userId, const QByteArrayList& roleIds) const;

	/**
	* @brief Removes roles from a user.
	*/
	virtual bool RemoveRolesFromUser(const QByteArray& userId, const QByteArrayList& roleIds) const;

	/**
	* @brief Returns user permissions.
	*/
	virtual QByteArrayList GetUserPermissions(const QByteArray& userId) const;

	/**
	* @brief Returns authentication system used by the user.
	*/
	virtual SystemType GetUserAuthSystem(const QByteArray& login) const;

	/**
	* @brief Returns all role identifiers.
	*/
	virtual QByteArrayList GetRoleIds() const;

	/**
	* @brief Retrieves role data.
	*/
	virtual bool GetRole(const QByteArray& roleId, Role& roleData) const;

	/**
	* @brief Creates a role.
	*/
	virtual QByteArray CreateRole(
		const QString& roleName,
		const QString& roleDescription = QString(),
		const QByteArrayList& permissions = QByteArrayList());

	/**
	* @brief Removes a role.
	*/
	virtual bool RemoveRole(const QByteArray& roleId) const;

	/**
	* @brief Returns permissions assigned to a role.
	*/
	virtual QByteArrayList GetRolePermissions(const QByteArray& roleId) const;

	/**
	* @brief Adds permissions to a role.
	*/
	virtual bool AddPermissionsToRole(
		const QByteArray& roleId,
		const QByteArrayList& permissions) const;

	/**
	* @brief Removes permissions from a role.
	*/
	virtual bool RemovePermissionsFromRole(
		const QByteArray& roleId,
		const QByteArrayList& permissions) const;

	/**
	* @brief Returns all group identifiers.
	*/
	virtual QByteArrayList GetGroupIds() const;

	/**
	* @brief Creates a group.
	*/
	virtual QByteArray CreateGroup(const QString& groupName, const QString& description) const;

	/**
	* @brief Removes a group.
	*/
	virtual bool RemoveGroup(const QByteArray& groupId) const;

	/**
	* @brief Retrieves group data.
	*/
	virtual bool GetGroup(const QByteArray& groupId, Group& groupData) const;

	/**
	* @brief Adds users to a group.
	*/
	virtual bool AddUsersToGroup(
		const QByteArray& groupId,
		const QByteArrayList& userIds) const;

	/**
	* @brief Removes users from a group.
	*/
	virtual bool RemoveUsersFromGroup(
		const QByteArray& groupId,
		const QByteArrayList& userIds) const;

	/**
	* @brief Adds roles to a group.
	*/
	virtual bool AddRolesToGroup(
		const QByteArray& groupId,
		const QByteArrayList& roleIds) const;

	/**
	* @brief Removes roles from a group.
	*/
	virtual bool RemoveRolesFromGroup(
		const QByteArray& groupId,
		const QByteArrayList& roleIds) const;

private:
	/**
	* @brief Pointer to the internal implementation.
	*/
	CAuthorizationControllerImpl* m_implPtr;
};


} // namespace AuthClientSdk


/**
* @defgroup AuthClientSdk Authorization Client SDK
*
* @brief Client-side API for authentication and authorization.
*
* Provides functionality for user management, role and group
* administration, and permission checks.
*/
