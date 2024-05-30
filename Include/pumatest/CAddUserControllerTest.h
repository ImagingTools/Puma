#pragma once


// ImtCore includes
#include <imtgql/CGqlRequest.h>
#include <imtgql/CGqlHandlerTest.h>
#include <imtdb/IDatabaseEngine.h>


class CAddUserControllerTest: public imtgql::CGqlHandlerTest
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
				databaseEnginePtr->ExecSqlQuery(QByteArray("DELETE FROM \"Users\";"));
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


