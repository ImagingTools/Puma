#pragma once


// ACF includes
#include <iser/CJsonMemWriteArchive.h>
#include <iser/CJsonMemReadArchive.h>

// ImtCore includes
#include <imtgql/CGqlRequest.h>
#include <imtgql/CGqlHandlerTest.h>
#include <imtgql/IGqlRequestHandler.h>
#include <imtdb/IDatabaseEngine.h>
#include <imtauth/CUserInfo.h>


class CAuthorizationControllerTest: public imtgql::CGqlHandlerTest
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
				databaseEnginePtr->ExecSqlQuery(QByteArray("DELETE FROM \"Users\" WHERE \"Document\"->>'Id' = 'test';"));

				imtauth::CIdentifiableUserInfo userInfo;
				userInfo.SetObjectUuid(QUuid::createUuid().toByteArray(QUuid::WithoutBraces));
				userInfo.SetId("test");
				userInfo.SetPasswordHash("5a105e8b9d40e1329780d62ea2265d8a");
				userInfo.SetDescription("Test");

				QByteArray userJsonData;
				{
					iser::CJsonMemWriteArchive archive(userJsonData);
					if (!userInfo.Serialize(archive)){
						qDebug() << QString("Unable to serialize a change object collection");
					}
				}

				QByteArray insertQuery = QString("INSERT INTO \"Users\"(\"DocumentId\", \"Document\", \"RevisionNumber\", \"LastModified\", \"Checksum\", \"IsActive\") VALUES('%1', '%2', '%3', '%4', '%5', true);")
							.arg(qPrintable(userInfo.GetObjectUuid()))
							.arg(qPrintable(userJsonData))
							.arg(1)
							.arg(QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs))
							.arg(0).toUtf8();

				databaseEnginePtr->ExecSqlQuery(insertQuery);
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


