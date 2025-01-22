#include "CTestBase.h"


// ACF includes
#include <istd/TDelPtr.h>
#include <iser/CJsonMemWriteArchive.h>
#include <iser/CJsonMemReadArchive.h>
#include <itest/CStandardTestExecutor.h>

// ImtCore includes
#include <imtauth/CUserInfo.h>
#include <imtgql/Test/CGqlSdlRequestTest.h>


namespace pumatest
{


// private slots

void CTestBase::initTestCase()
{
	QString pumaDir = qEnvironmentVariable("PUMADIR");
	m_registryFile = pumaDir + QString("/Partitura/PumaTestVoce.arp/PumaHandlersTest.acc");
	m_configFile = pumaDir + "/Config/PumaServer.awc";

	ClearData();
}


void CTestBase::cleanupTestCase()
{
	ClearData();
}


// protected methods

void CTestBase::ClearData()
{
	QSqlError sqlError;
	ExecuteQuery(QByteArray("DELETE FROM \"Users\";"), &sqlError);
	ExecuteQuery(QByteArray("DELETE FROM \"Roles\";"), &sqlError);
	ExecuteQuery(QByteArray("DELETE FROM \"UserGroups\";"), &sqlError);
}


bool CTestBase::GetObjectFromTable(const QString& tableName, const QByteArray& userId, iser::ISerializable& object) const
{
	QString query = QString(R"(SELECT * FROM "%1" WHERE "IsActive" = true AND "DocumentId" = '%2')").arg(tableName).arg(qPrintable(userId));

	QSqlError sqlError;
	QSqlQuery sqlQuery = ExecuteQuery(query.toUtf8(), &sqlError);
	if (sqlError.type() != QSqlError::NoError){
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

	return sqlError.type() != QSqlError::NoError;
}


bool CTestBase::RemoveObjectFromTable(const QString& tableName, const QByteArray& userId) const
{
	QString query = QString(R"(DELETE FROM "%1" WHERE "DocumentId" = '%2';)").arg(tableName).arg(qPrintable(userId));

	QSqlError sqlError;
	ExecuteQuery(query.toUtf8(), &sqlError);

	return sqlError.type() != QSqlError::NoError;
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


} // namespace pumatest
