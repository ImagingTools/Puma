#pragma once


// Qt includes
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

// ACF includes
#include <iser/CJsonMemWriteArchive.h>
#include <iser/CJsonMemReadArchive.h>


// ImtCore includes
#include <imtgql/CGqlRequest.h>
#include <imtgql/CGqlHandlerTest.h>
#include <imtdb/IDatabaseEngine.h>
#include <imtauth/CUserInfo.h>


class CGetUserListControllerTest: public imtgql::CGqlHandlerTest
{
public:
	// reimplemented (imtgql::CGqlHandlerTest)
	virtual imtgql::CGqlRequest* CreateGqlRequest() const override;
	virtual imtbase::CTreeItemModel* CreateExpectedModel() const override;
	virtual bool VerifyResponse(const imtbase::CTreeItemModel& actualModel, const imtbase::CTreeItemModel& expectedModel) const override;

	virtual void initTestCase() override
	{
		QString pumaDir = qEnvironmentVariable("PUMADIR");
		m_registryFile = pumaDir + QString("/Partitura/PumaTestVoce.arp/PumaHandlersTest.acc");
		m_configFile = pumaDir + "/Config/PumaServer.awc";

		ipackage::CComponentAccessor* accessorPtr = GetComponentAccessor();
		if (accessorPtr != nullptr){
			imtdb::IDatabaseEngine* databaseEnginePtr = accessorPtr->GetComponentInterface<imtdb::IDatabaseEngine>();
			if (databaseEnginePtr != nullptr){
				cleanupTestCase();

				QByteArray query;
				for (int i = 0; i < 10; i++){
					imtauth::CIdentifiableUserInfo userInfo;
					userInfo.SetObjectUuid(QUuid::createUuid().toByteArray(QUuid::WithoutBraces));
					userInfo.SetId("test" + QString::number(i).toUtf8());
					userInfo.SetPasswordHash("5a105e8b9d40e1329780d62ea2265d8a");
					userInfo.SetDescription("Test" + QString::number(i));

					QByteArray userJsonData;
					{
						iser::CJsonMemWriteArchive archive(userJsonData);
						if (!userInfo.Serialize(archive)){
							qDebug() << QString("Unable to serialize a change object collection");
						}
					}

					QByteArray insertQuery = QString("INSERT INTO \"Users\"(\"DocumentId\", \"Document\", \"RevisionNumber\", \"LastModified\", \"Checksum\", \"IsActive\") VALUES('%1', '%2', '%3', '%4', '%5', true);")
								.arg(userInfo.GetObjectUuid())
								.arg(userJsonData)
								.arg(1)
								.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs))
								.arg(0).toUtf8();

					query += insertQuery + '\n';
				}

				databaseEnginePtr->ExecSqlQuery(query);
			}
		}
	}

	virtual void cleanupTestCase() override
	{
		ipackage::CComponentAccessor* accessorPtr = GetComponentAccessor();
		if (accessorPtr != nullptr){
			imtdb::IDatabaseEngine* databaseEnginePtr = accessorPtr->GetComponentInterface<imtdb::IDatabaseEngine>();
			if (databaseEnginePtr != nullptr){
				databaseEnginePtr->ExecSqlQuery(QByteArray("DELETE FROM \"Users\";"));
			}
		}
	}
};


