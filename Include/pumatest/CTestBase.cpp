#include "CTestBase.h"


// ACF includes
#include <istd/TDelPtr.h>
#include <iser/CJsonMemWriteArchive.h>
#include <iser/CJsonMemReadArchive.h>
#include <itest/CStandardTestExecutor.h>

// ImtCore includes
#include <imtauth/CUserInfo.h>


namespace pumatest
{


static const QString s_usersTableName = "Users";
static const QString s_rolesTableName = "Roles";
static const QString s_groupsTableName = "UserGroups";


// private slots

void CTestBase::initTestCase()
{
	QString pumaDir = qEnvironmentVariable("PUMADIR");
	m_registryFile = pumaDir + QString("/Partitura/PumaVoce.arp/PumaTestHandler.acc");
	m_configFile = pumaDir + "/Config/PumaServer.awc";

	ClearData();

	SetData();
}


void CTestBase::cleanupTestCase()
{
	ClearData();
}


// protected methods

void CTestBase::SetData() const
{
}


void CTestBase::ClearData()
{
	QSqlError sqlError;
	ExecuteQuery(QByteArray("DELETE FROM \"Users\";"), &sqlError);
	ExecuteQuery(QByteArray("DELETE FROM \"Roles\";"), &sqlError);
	ExecuteQuery(QByteArray("DELETE FROM \"UserGroups\";"), &sqlError);
}


bool CTestBase::GetObjectFromTable(const QString& tableName, const QByteArray& userId, iser::ISerializable& object) const
{
	QString query = QString(R"(SELECT * FROM "%1" WHERE "IsActive" = true AND "DocumentId" = '%2')").arg(tableName, qPrintable(userId));

	QSqlError sqlError;
	QSqlQuery sqlQuery = ExecuteQuery(query.toUtf8(), &sqlError);
	if (sqlError.type() != QSqlError::NoError){
		qDebug() << sqlError.text();
		return false;
	}

	if (!sqlQuery.next()){
		return false;
	}

	imtbase::IIdentifiable* identifiableObjectPtr = dynamic_cast<imtbase::IIdentifiable*>(&object);
	if (identifiableObjectPtr == nullptr){
		return false;
	}

	QSqlRecord record = sqlQuery.record();

	QByteArray documentId;
	if (record.contains("DocumentId")){
		documentId = record.value("DocumentId").toByteArray();
	}

	identifiableObjectPtr->SetObjectUuid(documentId);

	if (record.contains("Document")){
		QByteArray documentContent = record.value("Document").toByteArray();

		iser::CJsonMemReadArchive archive(documentContent, false);
		if (!object.Serialize(archive)){
			return false;
		}
	}

	return true;
}


bool CTestBase::InsertObjectToTable(const QString& tableName, iser::ISerializable& object) const
{
	imtbase::IIdentifiable* identifiableObject = dynamic_cast<imtbase::IIdentifiable*>(&object);
	if (identifiableObject == nullptr){
		return false;
	}

	QByteArray jsonData;
	{
		iser::CJsonMemWriteArchive archive(nullptr);
		if (!object.Serialize(archive)){
			qDebug() << QString("Unable to serialize a change object collection");
		}

		jsonData = archive.GetData();
	}

	QByteArray insertQuery = QString("INSERT INTO \"%0\"(\"DocumentId\", \"Document\", \"RevisionNumber\", \"LastModified\", \"Checksum\", \"IsActive\") VALUES('%1', '%2', '%3', '%4', '%5', true);")
								 .arg(tableName)
								 .arg(qPrintable(identifiableObject->GetObjectUuid()))
								 .arg(qPrintable(jsonData))
								 .arg(1)
								 .arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs))
								 .arg(0).toUtf8();

	QSqlError sqlError;
	ExecuteQuery(insertQuery, &sqlError);

	bool ok = sqlError.type() == QSqlError::NoError;
	if (!ok){
		qDebug() << sqlError.text();
	}

	return ok;
}


bool CTestBase::RemoveObjectFromTable(const QString& tableName, const QByteArray& userId) const
{
	QString query = QString(R"(DELETE FROM "%1" WHERE "DocumentId" = '%2';)").arg(tableName).arg(qPrintable(userId));

	QSqlError sqlError;
	ExecuteQuery(query.toUtf8(), &sqlError);

	return sqlError.type() == QSqlError::NoError;
}


bool CTestBase::AddUser(const imtauth::CIdentifiableUserInfo& userData) const
{
	return InsertObjectToTable("Users", const_cast<imtauth::CIdentifiableUserInfo&>(userData));
}


bool CTestBase::AddRole(const imtauth::CIdentifiableRoleInfo& roleData) const
{
	return InsertObjectToTable("Roles", const_cast<imtauth::CIdentifiableRoleInfo&>(roleData));
}


bool CTestBase::AddGroup(const imtauth::CIdentifiableUserGroupInfo& groupData) const
{
	return InsertObjectToTable("UserGroups", const_cast<imtauth::CIdentifiableUserGroupInfo&>(groupData));
}


imtauth::IUserInfo* CTestBase::CreateUserInfo(
			const QString& username,
			const QString& password,
			const QString& name,
			const QString& email) const
{
	istd::TDelPtr<imtauth::CIdentifiableUserInfo> userInfoPtr;
	userInfoPtr.SetCastedOrRemove(new imtauth::CIdentifiableUserInfo);

	QByteArray uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).toUtf8();
	userInfoPtr->SetObjectUuid(uuid);
	userInfoPtr->SetId(username.toUtf8());

	QByteArray passwordHash = m_hashCalculator.GenerateHash(username.toUtf8() + password.toUtf8());
	userInfoPtr->SetPasswordHash(passwordHash);
	userInfoPtr->SetMail(email);
	userInfoPtr->SetName(name);

	return userInfoPtr.PopPtr();
}


sdl::imtauth::Users::CUserData::V1_0* CTestBase::CreateUserDataFromUserInfo(
			const imtauth::CIdentifiableUserInfo& userInfo,
			 const QByteArray& productId) const
{
	istd::TDelPtr<sdl::imtauth::Users::CUserData::V1_0> userDataPtr;
	userDataPtr.SetCastedOrRemove(new sdl::imtauth::Users::CUserData::V1_0);

	userDataPtr->Id = userInfo.GetObjectUuid();
	userDataPtr->Name = userInfo.GetName();
	userDataPtr->Username = userInfo.GetId();
	userDataPtr->Password = userInfo.GetPasswordHash();
	userDataPtr->Email = userInfo.GetMail();
	userDataPtr->ProductId = productId;

	QByteArrayList groupList = userInfo.GetGroups();
	groupList.removeAll("");

	userDataPtr->Groups = groupList.join(';');

	if (!productId.isEmpty()){
		QByteArrayList roleList = userInfo.GetRoles(productId);
		roleList.removeAll("");

		userDataPtr->Roles = roleList.join(';');
	}

	QList<sdl::imtauth::Users::CSystemInfo::V1_0> list;
	imtauth::IUserInfo::SystemInfoList systemInfoList = userInfo.GetSystemInfos();
	for (const imtauth::IUserInfo::SystemInfo& systemInfo : systemInfoList){
		sdl::imtauth::Users::CSystemInfo::V1_0 info;

		info.Id = QByteArray(systemInfo.systemId);

		if (systemInfo.systemId.isEmpty()){
			info.Name = QString("Internal");
		}
		else{
			info.Name = QString(systemInfo.systemName);
		}

		info.Enabled = bool(systemInfo.enabled);

		list << info;
	}
	userDataPtr->SystemInfos = std::make_optional<QList<sdl::imtauth::Users::CSystemInfo::V1_0>>(list);

	return userDataPtr.PopPtr();
}


imtauth::IRole* CTestBase::CreateRoleInfo(
	const QString& roleId,
	const QString& roleName,
	const QString& productId,
	QByteArrayList features) const
{
	istd::TDelPtr<imtauth::CIdentifiableRoleInfo> roleInfoPtr;
	roleInfoPtr.SetCastedOrRemove(new imtauth::CIdentifiableRoleInfo);

	roleInfoPtr->SetRoleId(roleId.toUtf8());
	roleInfoPtr->SetRoleName(roleName);
	roleInfoPtr->SetProductId(productId.toUtf8());
	roleInfoPtr->SetLocalPermissions(features);

	return roleInfoPtr.PopPtr();
}


sdl::imtauth::Roles::CRoleData::V1_0* CTestBase::CreateRoleDataFromUserInfo(const imtauth::CIdentifiableRoleInfo& roleInfo) const
{
	istd::TDelPtr<sdl::imtauth::Roles::CRoleData::V1_0> roleDataPtr;
	roleDataPtr.SetCastedOrRemove(new sdl::imtauth::Roles::CRoleData::V1_0);

	roleDataPtr->Id = roleInfo.GetObjectUuid();
	roleDataPtr->RoleId = roleInfo.GetRoleId();
	roleDataPtr->ProductId = roleInfo.GetProductId();
	roleDataPtr->Name = roleInfo.GetRoleName();
	roleDataPtr->Description = roleInfo.GetRoleDescription();

	QByteArrayList parentRoles = roleInfo.GetIncludedRoles();
	parentRoles.removeAll("");
	roleDataPtr->ParentRoles = parentRoles.join(';');

	QByteArrayList features = roleInfo.GetPermissions();
	roleDataPtr->Permissions = features.join(';');

	roleDataPtr->IsDefault = false;
	roleDataPtr->IsGuest = false;

	return roleDataPtr.PopPtr();
}


} // namespace pumatest
