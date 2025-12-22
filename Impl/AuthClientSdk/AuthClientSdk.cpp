#include <AuthClientSdk/AuthClientSdk.h>


// Qt includes
#include <QDebug>

// ACF includes
#include <ibase/IApplicationInfo.h>
#include <iauth/ILogin.h>
#include <iauth/IRightsProvider.h>

// ImtCore includes
#include <imtbase/IApplicationInfoController.h>
#include <imtauth/IAccessTokenProvider.h>
#include <imtauth/IUserPermissionsController.h>
#include <imtauth/ISuperuserController.h>
#include <imtauth/ISuperuserProvider.h>
#include <imtauth/IUserManager.h>
#include <imtauth/IRoleManager.h>
#include <imtauth/IUserGroupManager.h>
#include <imtcom/IServerConnectionInterface.h>

// Local includes
#include <GeneratedFiles/AuthClientSdk/CAuthClientSdk.h>


namespace AuthClientSdk
{


class CAuthorizationControllerImpl
{
public:
	CAuthorizationControllerImpl()
	{
	}

	bool Login(const QString& login, const QString& password, Login& out)
	{
		out.Clear();

		iauth::ILogin* loginPtr = m_sdk.GetInterface<iauth::ILogin>();
		if (loginPtr == nullptr) {
			qWarning() << "[Login] Failed: iauth::ILogin interface not found";
			return false;
		}

		bool ok = loginPtr->Login(login, password);
		if (!ok) {
			qWarning() << "[Login] Failed: iauth::ILogin::Login() returned false";
			return false;
		}

		iauth::CUser* userPtr = loginPtr->GetLoggedUser();
		if (userPtr == nullptr) {
			qWarning() << "[Login] Failed: GetLoggedUser() returned nullptr";
			return false;
		}

		imtauth::IAccessTokenProvider* tokenProviderPtr = m_sdk.GetInterface<imtauth::IAccessTokenProvider>();
		if (tokenProviderPtr == nullptr) {
			qWarning() << "[Login] Failed: imtauth::IAccessTokenProvider interface not found";
			return false;
		}

		ibase::IApplicationInfo* applicationInfoPtr = m_sdk.GetInterface<ibase::IApplicationInfo>();
		if (applicationInfoPtr == nullptr) {
			qWarning() << "[Login] Failed: ibase::IApplicationInfo interface not found";
			return false;
		}

		imtauth::IUserPermissionsController* userPermissionsControllerPtr = m_sdk.GetInterface<imtauth::IUserPermissionsController>();
		if (userPermissionsControllerPtr == nullptr) {
			qWarning() << "[Login] Failed: imtauth::IUserPermissionsController interface not found";
			return false;
		}

		out.productId = applicationInfoPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_APPLICATION_ID).toUtf8();
		out.userName = userPtr->GetUserName();
		out.accessToken = tokenProviderPtr->GetToken(QByteArray());
		out.permissions = userPermissionsControllerPtr->GetPermissions(QByteArray());

		return true;
	}

	bool Logout()
	{
		iauth::ILogin* loginPtr = m_sdk.GetInterface<iauth::ILogin>();
		if (loginPtr == nullptr) {
			qWarning() << "[Logout] Failed: iauth::ILogin interface not found";
			return false;
		}

		return loginPtr->Logout();
	}

	bool SetConnectionParam(const ServerConfig& config)
	{
		imtcom::IServerConnectionInterface* connectionInterfacePtr = m_sdk.GetInterface<imtcom::IServerConnectionInterface>();
		if (connectionInterfacePtr == nullptr) {
			qWarning() << "[SetConnectionParam] Failed: imtcom::IServerConnectionInterface interface not found";
			return false;
		}

		if (config.sslConfig.has_value()){
			connectionInterfacePtr->SetConnectionFlags(imtcom::IServerConnectionInterface::CF_SECURE);
		}

		connectionInterfacePtr->SetHost(config.host);
		connectionInterfacePtr->SetPort(imtcom::IServerConnectionInterface::PT_HTTP, config.httpPort);
		connectionInterfacePtr->SetPort(imtcom::IServerConnectionInterface::PT_WEBSOCKET, config.wsPort);

		return true;
	}

	bool HasPermission(const QByteArray& permissionId)
	{
		iauth::IRightsProvider* rightsProviderPtr = m_sdk.GetInterface<iauth::IRightsProvider>();
		if (rightsProviderPtr == nullptr) {
			qWarning() << "[HasPermission] Failed: iauth::IRightsProvider interface not found";
			return false;
		}

		return rightsProviderPtr->HasRight(permissionId);
	}

	QByteArray GetToken() const
	{
		imtauth::IAccessTokenProvider* accessTokenProviderPtr = m_sdk.GetInterface<imtauth::IAccessTokenProvider>();
		if (accessTokenProviderPtr != nullptr) {
			return accessTokenProviderPtr->GetToken("");
		}

		qWarning() << "[GetToken] Failed: imtauth::IAccessTokenProvider interface not found";

		return QByteArray();
	}

	void SetProductId(const QByteArray& productId)
	{
		imtbase::IApplicationInfoController* applicationInfoControllerPtr = m_sdk.GetInterface<imtbase::IApplicationInfoController>();
		if (applicationInfoControllerPtr != nullptr) {
			applicationInfoControllerPtr->SetApplicationAttribute(
						imtbase::IApplicationInfoController::ApplicationAttribute::AA_APPLICATION_ID,
						productId);
		}
		else{
			qWarning() << "[SetProductId] Failed: IApplicationInfoController not found";
		}
	}

	SuperuserStatus SuperuserExists(QString& errorMessage)
	{
		imtauth::ISuperuserProvider* superuserProviderPtr = m_sdk.GetInterface<imtauth::ISuperuserProvider>();
		if (superuserProviderPtr != nullptr) {
			imtauth::ISuperuserProvider::ExistsStatus status = superuserProviderPtr->SuperuserExists(errorMessage);
			switch (status){
			case imtauth::ISuperuserProvider::ES_EXISTS:
				return SuperuserStatus::Exists;
			case imtauth::ISuperuserProvider::ES_NOT_EXISTS:
				return SuperuserStatus::NotExists;
			case imtauth::ISuperuserProvider::ES_UNKNOWN:
				return SuperuserStatus::Unknown;
			}
		}

		qWarning() << "[SuperuserExists] Failed: imtauth::ISuperuserProvider interface not found";

		return SuperuserStatus::Unknown;
	}

	bool CreateSuperuser(const QByteArray& password)
	{
		imtauth::ISuperuserController* superuserControllerPtr = m_sdk.GetInterface<imtauth::ISuperuserController>();
		if (superuserControllerPtr != nullptr){
			return superuserControllerPtr->SetSuperuserPassword(password);
		}

		qWarning() << "[CreateSuperuser] Failed: imtauth::ISuperuserController interface not found";

		return false;
	}

	QByteArrayList GetUserIds() const
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			return userManagerPtr->GetUserIds();
		}

		qWarning() << "[GetUserIds] Failed: imtauth::IUserManager interface not found";

		return QByteArrayList();
	}

	bool GetUser(const QByteArray& userId, User& userData) const
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			imtauth::IUserInfoUniquePtr userInfoPtr = userManagerPtr->GetUser(userId);
			if (!userInfoPtr.IsValid()){
				return false;
			}

			QByteArray productId = GetProductId();

			userData.name = userInfoPtr->GetName();
			userData.email = userInfoPtr->GetMail();
			userData.login = userInfoPtr->GetId();
			userData.roleIds = userInfoPtr->GetRoles(productId);
			userData.groupIds = userInfoPtr->GetGroups();
	
			return true;
		}

		qWarning() << "[GetUser] Failed: imtauth::IUserManager interface not found";

		return false;
	}

	bool GetUserByLogin(const QByteArray& login, User& userData) const
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			QByteArray objectId = userManagerPtr->GetUserObjectId(login);
			imtauth::IUserInfoUniquePtr userInfoPtr = userManagerPtr->GetUser(objectId);
			if (!userInfoPtr.IsValid()){
				return false;
			}

			QByteArray productId = GetProductId();

			userData.name = userInfoPtr->GetName();
			userData.email = userInfoPtr->GetMail();
			userData.login = userInfoPtr->GetId();
			userData.roleIds = userInfoPtr->GetRoles(productId);
			userData.groupIds = userInfoPtr->GetGroups();
	
			return true;
		}

		qWarning() << "[GetUser] Failed: imtauth::IUserManager interface not found";

		return false;
	}

	bool RemoveUser(const QByteArray& userId)
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			return userManagerPtr->RemoveUser(userId);
		}

		qWarning() << "[RemoveUser] Failed: imtauth::IUserManager interface not found";

		return false;
	}

	QByteArray CreateUser(const QString& userName, const QByteArray& login, const QByteArray& password, const QString& email)
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			return userManagerPtr->CreateUser(userName, login, password, email);
		}

		qWarning() << "[CreateUser] Failed: imtauth::IUserManager interface not found";

		return QByteArray();
	}

	bool ChangeUserPassword(const QByteArray& login, const QByteArray& oldPassword, const QByteArray& newPassword)
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			return userManagerPtr->ChangeUserPassword(login, oldPassword, newPassword);
		}

		qWarning() << "[ChangeUserPassword] Failed: imtauth::IUserManager interface not found";

		return false;
	}

	bool AddRolesToUser(const QByteArray& userId, const QByteArrayList& roleIds)
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			QByteArray productId = GetProductId();
			return userManagerPtr->AddRolesToUser(userId, productId, roleIds);
		}

		qWarning() << "[AddRolesToUser] Failed: imtauth::IUserManager interface not found";

		return false;
	}

	bool RemoveRolesFromUser(const QByteArray& userId, const QByteArrayList& roleIds)
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			QByteArray productId = GetProductId();
			return userManagerPtr->RemoveRolesFromUser(userId, productId, roleIds);
		}

		qWarning() << "[RemoveRolesFromUser] Failed: imtauth::IUserManager interface not found";

		return false;
	}

	QByteArrayList GetUserPermissions(const QByteArray& userId) const
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			return userManagerPtr->GetUserPermissions(userId, GetProductId());
		}

		qWarning() << "[GetUserPermissions] Failed: imtauth::IUserManager interface not found";

		return QByteArrayList();
	}

	SystemType GetUserAuthSystem(const QByteArray& login) const
	{
		imtauth::IUserManager* userManagerPtr = m_sdk.GetInterface<imtauth::IUserManager>();
		if (userManagerPtr != nullptr){
			imtauth::IUserInfo::SystemInfo systemInfo;
			if (!userManagerPtr->GetUserAuthSystem(login, systemInfo)){
				return SystemType::Unknown;
			}

			if (!systemInfo.systemId.isEmpty()){
				return SystemType::Ldap;
			}

			return SystemType::Local;
		}

		qWarning() << "[GetUserAuthSystem] Failed: imtauth::IUserManager interface not found";

		return SystemType::Unknown;
	}

	QByteArrayList GetRoleIds() const
	{
		imtauth::IRoleManager* roleManagerPtr = m_sdk.GetInterface<imtauth::IRoleManager>();
		if (roleManagerPtr != nullptr){
			return roleManagerPtr->GetRoleIds();
		}

		qWarning() << "[GetRoleIds] Failed: imtauth::IRoleManager interface not found";

		return QByteArrayList();
	}


	bool GetRole(const QByteArray& roleId, Role& roleData) const
	{
		imtauth::IRoleManager* roleManagerPtr = m_sdk.GetInterface<imtauth::IRoleManager>();
		if (roleManagerPtr != nullptr){
			imtauth::IRoleUniquePtr roleInfoPtr = roleManagerPtr->GetRole(roleId);
			if (!roleInfoPtr.IsValid()){
				return false;
			}

			roleData.name = roleInfoPtr->GetRoleName();
			roleData.description = roleInfoPtr->GetRoleDescription();
			roleData.permissionIds = roleInfoPtr->GetPermissions();

			return true;
		}

		qWarning() << "[GetRole] Failed: imtauth::IRoleManager interface not found";

		return false;
	}

	
	QByteArray CreateRole(
				const QString& roleName,
				const QString& roleDescription,
				const QByteArrayList& permissions)
	{
		imtauth::IRoleManager* roleManagerPtr = m_sdk.GetInterface<imtauth::IRoleManager>();
		if (roleManagerPtr != nullptr){
			QByteArray productId = GetProductId();
			return roleManagerPtr->CreateRole(productId, roleName, roleDescription, permissions);
		}

		qWarning() << "[CreateRole] Failed: imtauth::IRoleManager interface not found";

		return QByteArray();
	}


	bool RemoveRole(const QByteArray& roleId)
	{
		imtauth::IRoleManager* roleManagerPtr = m_sdk.GetInterface<imtauth::IRoleManager>();
		if (roleManagerPtr != nullptr){
			return roleManagerPtr->RemoveRole(roleId);
		}

		qWarning() << "[RemoveRole] Failed: imtauth::IRoleManager interface not found";

		return false;
	}


	QByteArrayList GetRolePermissions(const QByteArray& roleId) const
	{
		imtauth::IRoleManager* roleManagerPtr = m_sdk.GetInterface<imtauth::IRoleManager>();
		if (roleManagerPtr != nullptr){
			return roleManagerPtr->GetRolePermissions(roleId);
		}

		qWarning() << "[GetRolePermissions] Failed: imtauth::IRoleManager interface not found";

		return QByteArrayList();
	}


	bool AddPermissionsToRole(const QByteArray& roleId, const QByteArrayList& permissions)
	{
		imtauth::IRoleManager* roleManagerPtr = m_sdk.GetInterface<imtauth::IRoleManager>();
		if (roleManagerPtr != nullptr){
			return roleManagerPtr->AddPermissionsToRole(roleId, permissions);
		}

		qWarning() << "[AddPermissionsToRole] Failed: imtauth::IRoleManager interface not found";

		return false;
	}


	bool RemovePermissionsFromRole(const QByteArray& roleId, const QByteArrayList& permissions)
	{
		imtauth::IRoleManager* roleManagerPtr = m_sdk.GetInterface<imtauth::IRoleManager>();
		if (roleManagerPtr != nullptr){
			return roleManagerPtr->RemovePermissionsFromRole(roleId, permissions);
		}

		qWarning() << "[RemovePermissionsFromRole] Failed: imtauth::IRoleManager interface not found";

		return false;
	}


	QByteArrayList GetGroupIds() const
	{
		imtauth::IUserGroupManager* groupManagerPtr = m_sdk.GetInterface<imtauth::IUserGroupManager>();
		if (groupManagerPtr != nullptr){
			return groupManagerPtr->GetGroupIds();
		}

		qWarning() << "[GetGroupIds] Failed: imtauth::IUserGroupManager interface not found";

		return QByteArrayList();
	}


	QByteArray CreateGroup(const QString& groupName, const QString& description)
	{
		imtauth::IUserGroupManager* groupManagerPtr = m_sdk.GetInterface<imtauth::IUserGroupManager>();
		if (groupManagerPtr != nullptr){
			return groupManagerPtr->CreateGroup(groupName, description);
		}

		qWarning() << "[CreateGroup] Failed: imtauth::IUserGroupManager interface not found";

		return QByteArray();
	}


	bool RemoveGroup(const QByteArray& groupId)
	{
		imtauth::IUserGroupManager* groupManagerPtr = m_sdk.GetInterface<imtauth::IUserGroupManager>();
		if (groupManagerPtr != nullptr){
			return groupManagerPtr->RemoveGroup(groupId);
		}

		qWarning() << "[RemoveGroup] Failed: imtauth::IUserGroupManager interface not found";

		return false;
	}


	bool GetGroup(const QByteArray& groupId, Group& groupData) const
	{
		imtauth::IUserGroupManager* groupManagerPtr = m_sdk.GetInterface<imtauth::IUserGroupManager>();
		if (groupManagerPtr != nullptr){
			imtauth::IUserGroupInfoSharedPtr groupPtr = groupManagerPtr->GetGroup(groupId);
			if (!groupPtr.IsValid()){
				return false;
			}

			QByteArray productId = GetProductId();

			groupData.name = groupPtr->GetName();
			groupData.description = groupPtr->GetDescription();
			groupData.roleIds = groupPtr->GetRoles(productId);
			groupData.userIds = groupPtr->GetUsers();

			return true;
		}

		qWarning() << "[GetGroup] Failed: imtauth::IUserGroupManager interface not found";

		return false;
	}


	bool AddUsersToGroup(const QByteArray& groupId, const QByteArrayList& userIds)
	{
		imtauth::IUserGroupManager* groupManagerPtr = m_sdk.GetInterface<imtauth::IUserGroupManager>();
		if (groupManagerPtr != nullptr){
			return groupManagerPtr->AddUsersToGroup(groupId, userIds);
		}

		qWarning() << "[AddUsersToGroup] Failed: imtauth::IUserGroupManager interface not found";

		return false;
	}


	bool RemoveUsersFromGroup(const QByteArray& groupId, const QByteArrayList& userIds)
	{
		imtauth::IUserGroupManager* groupManagerPtr = m_sdk.GetInterface<imtauth::IUserGroupManager>();
		if (groupManagerPtr != nullptr){
			return groupManagerPtr->RemoveUsersFromGroup(groupId, userIds);
		}

		qWarning() << "[RemoveUsersFromGroup] Failed: imtauth::IUserGroupManager interface not found";

		return false;
	}


	bool AddRolesToGroup(const QByteArray& groupId, const QByteArrayList& roleIds)
	{
		imtauth::IUserGroupManager* groupManagerPtr = m_sdk.GetInterface<imtauth::IUserGroupManager>();
		if (groupManagerPtr != nullptr){
			return groupManagerPtr->AddRolesToGroup(groupId, roleIds);
		}

		qWarning() << "[AddRolesToGroup] Failed: imtauth::IUserGroupManager interface not found";

		return false;
	}


	bool RemoveRolesFromGroup(const QByteArray& groupId, const QByteArrayList& roleIds)
	{
		imtauth::IUserGroupManager* groupManagerPtr = m_sdk.GetInterface<imtauth::IUserGroupManager>();
		if (groupManagerPtr != nullptr){
			return groupManagerPtr->RemoveRolesFromGroup(groupId, roleIds);
		}

		qWarning() << "[RemoveRolesFromGroup] Failed: imtauth::IUserGroupManager interface not found";

		return false;
	}

private:
	QByteArray GetProductId() const
	{
		ibase::IApplicationInfo* applicationInfoPtr = m_sdk.GetInterface<ibase::IApplicationInfo>();
		if (applicationInfoPtr == nullptr) {
			return QByteArray();
		}

		return applicationInfoPtr->GetApplicationAttribute(ibase::IApplicationInfo::AA_APPLICATION_ID).toUtf8();
	}

	mutable CAuthClientSdk m_sdk;
};


// public methods

CAuthorizationController::CAuthorizationController()
	: m_implPtr(nullptr)
{
	m_implPtr = new CAuthorizationControllerImpl;
}


CAuthorizationController::~CAuthorizationController()
{
	delete m_implPtr;
}


bool CAuthorizationController::Login(const QString& login, const QString& password, struct Login& out) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->Login(login, password, out);
	}

	return false;
}


bool CAuthorizationController::Logout() const
{
	if (m_implPtr != nullptr){
		return m_implPtr->Logout();
	}

	return false;
}


bool CAuthorizationController::SetConnectionParam(const ServerConfig& config) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetConnectionParam(config);
	}

	return false;
}


bool CAuthorizationController::HasPermission(const QByteArray& permissionId) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->HasPermission(permissionId);
	}

	return false;
}


QByteArray CAuthorizationController::GetToken() const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetToken();
	}

	return QByteArray();
}


void CAuthorizationController::SetProductId(const QByteArray& productId) const 
{
	if (m_implPtr != nullptr){
		m_implPtr->SetProductId(productId);
	}
}


SuperuserStatus CAuthorizationController::SuperuserExists(QString& errorMessage) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SuperuserExists(errorMessage);
	}

	return SuperuserStatus::Unknown;
}


bool CAuthorizationController::CreateSuperuser(const QByteArray& password) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->CreateSuperuser(password);
	}

	return false;
}


QByteArrayList CAuthorizationController::GetUserIds() const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetUserIds();
	}

	return QByteArrayList();
}


bool CAuthorizationController::GetUser(const QByteArray& userId, User& userData) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetUser(userId, userData);
	}

	return false;
}


bool CAuthorizationController::GetUserByLogin(const QByteArray& login, User& userData) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetUserByLogin(login, userData);
	}

	return false;
}


bool CAuthorizationController::RemoveUser(const QByteArray& userId) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->RemoveUser(userId);
	}

	return false;
}


QByteArray CAuthorizationController::CreateUser(const QString& userName, const QByteArray& login, const QByteArray& password, const QString& email) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->CreateUser(userName, login, password, email);
	}

	return QByteArray();
}


bool CAuthorizationController::ChangeUserPassword(const QByteArray& login, const QByteArray& oldPassword, const QByteArray& newPassword) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->ChangeUserPassword(login, oldPassword, newPassword);
	}

	return false;
}


bool CAuthorizationController::AddRolesToUser(const QByteArray& userId, const QByteArrayList& roleIds) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->AddRolesToUser(userId, roleIds);
	}

	return false;
}


bool CAuthorizationController::RemoveRolesFromUser(const QByteArray& userId, const QByteArrayList& roleIds) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->RemoveRolesFromUser(userId, roleIds);
	}

	return false;
}


QByteArrayList CAuthorizationController::GetUserPermissions(const QByteArray& userId) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetUserPermissions(userId);
	}

	return QByteArrayList();
}


SystemType CAuthorizationController::GetUserAuthSystem(const QByteArray& login) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetUserAuthSystem(login);
	}

	return SystemType::Unknown;
}


QByteArrayList CAuthorizationController::GetRoleIds() const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetRoleIds();
	}

	return QByteArrayList();
}


bool CAuthorizationController::GetRole(const QByteArray& roleId, Role& roleData) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetRole(roleId, roleData);
	}

	return false;
}


QByteArray CAuthorizationController::CreateRole(
			const QString& roleName,
			const QString& roleDescription,
			const QByteArrayList& permissions)
{
	if (m_implPtr != nullptr){
		return m_implPtr->CreateRole(roleName, roleDescription, permissions);
	}

	return QByteArray();
}


bool CAuthorizationController::RemoveRole(const QByteArray& roleId) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->RemoveRole(roleId);
	}

	return false;
}


QByteArrayList CAuthorizationController::GetRolePermissions(const QByteArray& roleId) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetRolePermissions(roleId);
	}

	return QByteArrayList();
}


bool CAuthorizationController::AddPermissionsToRole(const QByteArray& roleId, const QByteArrayList& permissions) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->AddPermissionsToRole(roleId, permissions);
	}

	return false;
}


bool CAuthorizationController::RemovePermissionsFromRole(const QByteArray& roleId, const QByteArrayList& permissions) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->RemovePermissionsFromRole(roleId, permissions);
	}

	return false;
}


QByteArrayList CAuthorizationController::GetGroupIds() const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetGroupIds();
	}

	return QByteArrayList();
}


QByteArray CAuthorizationController::CreateGroup(const QString& groupName, const QString& description) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->CreateGroup(groupName, description);
	}

	return QByteArray();
}


bool CAuthorizationController::RemoveGroup(const QByteArray& groupId) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->RemoveGroup(groupId);
	}

	return false;
}


bool CAuthorizationController::GetGroup(const QByteArray& groupId, Group& groupData) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->GetGroup(groupId, groupData);
	}

	return false;
}


bool CAuthorizationController::AddUsersToGroup(const QByteArray& groupId, const QByteArrayList& userIds) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->AddUsersToGroup(groupId, userIds);
	}

	return false;
}


bool CAuthorizationController::RemoveUsersFromGroup(const QByteArray& groupId, const QByteArrayList& userIds) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->RemoveUsersFromGroup(groupId, userIds);
	}

	return false;
}


bool CAuthorizationController::AddRolesToGroup(const QByteArray& groupId, const QByteArrayList& roleIds) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->AddRolesToGroup(groupId, roleIds);
	}

	return false;
}


bool CAuthorizationController::RemoveRolesFromGroup(const QByteArray& groupId, const QByteArrayList& roleIds) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->RemoveRolesFromGroup(groupId, roleIds);
	}

	return false;
}


} // namespace AuthClientSdk
