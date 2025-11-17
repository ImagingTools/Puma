#pragma once


#if defined _MSC_VER || defined __MINGW32__ || defined __MINGW64__
#define AUTH_CLIENT_SDK_EXPORT __declspec(dllexport)
#elif defined __GNUC__
#define AUTH_CLIENT_SDK_EXPORT __attribute__((visibility("default")))
#else
#define AUTH_CLIENT_SDK_EXPORT
#endif


// Qt includes
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QByteArrayList>


namespace AuthClientSdk
{


class CAuthorizationControllerImpl;


struct Login
{
	QByteArray accessToken;
	QString userName;
	QByteArray productId;
	QByteArrayList permissions;

	void Clear(){
		accessToken.clear();
		userName.clear();
		productId.clear();
		permissions.clear();
	}
};

struct User 
{
	QString name;
	QString email;
	QByteArray login;
	QByteArrayList roleIds;
	QByteArrayList groupIds;
};

struct Role 
{
	QString name;
	QString description;
	QByteArrayList permissionIds;
};

struct Group 
{
	QString name;
	QString description;
	QByteArrayList userIds;
	QByteArrayList roleIds;
};


enum class SuperuserStatus 
{
	Unknown,
	Exists,
	NotExists
};


enum class SystemType
{
	Unknown,
	Local,
	Ldap
};


/**
*	\ingroup AuthClientSdk
*/
class AUTH_CLIENT_SDK_EXPORT CAuthorizationController
{
public:
	CAuthorizationController();

	virtual ~CAuthorizationController();

	virtual bool Login(const QString& login, const QString& password, Login& out);
	virtual bool Logout();
	bool SetConnectionParam(const QString& host, int httpPort, int wsPort);
	virtual bool HasPermission(const QByteArray& permissionId);
	virtual QByteArray GetToken() const;
	virtual void SetProductId(const QByteArray& productId);
	virtual SuperuserStatus SuperuserExists(QString& errorMessage);
	virtual bool CreateSuperuser(const QByteArray& password);
	virtual QByteArrayList GetUserIds() const;
	virtual bool GetUser(const QByteArray& userId, User& userData) const;
	virtual bool GetUserByLogin(const QByteArray& login, User& userData) const;
	virtual bool RemoveUser(const QByteArray& userId);
	virtual QByteArray CreateUser(const QString& userName, const QByteArray& login, const QByteArray& password, const QString& email);
	virtual bool ChangeUserPassword(const QByteArray& login, const QByteArray& oldPassword, const QByteArray& newPassword);
	virtual bool AddRolesToUser(const QByteArray& userId, const QByteArrayList& roleIds);
	virtual bool RemoveRolesFromUser(const QByteArray& userId, const QByteArrayList& roleIds);
	virtual QByteArrayList GetUserPermissions(const QByteArray& userId) const;
	virtual SystemType GetUserAuthSystem(const QByteArray& login) const;

	virtual QByteArrayList GetRoleIds() const;
	virtual bool GetRole(const QByteArray& roleId, Role& roleData) const;
	virtual QByteArray CreateRole(
				const QString& roleName,
				const QString& roleDescription = QString(),
				const QByteArrayList& permissions = QByteArrayList());
	virtual bool RemoveRole(const QByteArray& roleId);
	virtual QByteArrayList GetRolePermissions(const QByteArray& roleId) const;
	virtual bool AddPermissionsToRole(const QByteArray& roleId, const QByteArrayList& permissions);
	virtual bool RemovePermissionsFromRole(const QByteArray& roleId, const QByteArrayList& permissions);

	virtual QByteArrayList GetGroupIds() const;
	virtual QByteArray CreateGroup(const QString& groupName, const QString& description);
	virtual bool RemoveGroup(const QByteArray& groupId);
	virtual bool GetGroup(const QByteArray& groupId, Group& groupData) const;
	virtual bool AddUsersToGroup(const QByteArray& groupId, const QByteArrayList& userIds);
	virtual bool RemoveUsersFromGroup(const QByteArray& groupId, const QByteArrayList& userIds);
	virtual bool AddRolesToGroup(const QByteArray& groupId, const QByteArrayList& roleIds);
	virtual bool RemoveRolesFromGroup(const QByteArray& groupId, const QByteArrayList& roleIds);

private:
	CAuthorizationControllerImpl* m_implPtr;
};


} // namespace AuthClientSdk


