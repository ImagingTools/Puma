#include <AuthServerSdk/AuthServerSdk.h>


// Qt includes
#include <QtCore/QFile>

// ACF includes
#include <iauth/ILogin.h>
#include <iauth/IRightsProvider.h>
#include <istd/CChangeGroup.h>
#include <iser/CCompactXmlMemReadArchive.h>

// ImtCore includes
#include <imtbase/IApplicationInfoController.h>
#include <imtlic/IProductInfo.h>
#include <imtdb/IDatabaseLoginSettings.h>
#include <imtcom/IServerConnectionInterface.h>
#include <imtcom/IServerDispatcher.h>

// Local includes
#include <GeneratedFiles/AuthServerSdk/CAuthServerSdk.h>


namespace AuthServerSdk
{


class CAuthorizableServerImpl
{
public:
	CAuthorizableServerImpl()
	{
		m_sdk.EnsureAutoInitComponentsCreated();
	}


	bool Start(int httpPort, int wsPort)
	{
		imtcom::IServerDispatcher* serverControllerPtr = m_sdk.GetInterface<imtcom::IServerDispatcher>();
		if (serverControllerPtr == nullptr){
			return false;
		}

		if (!SetConnectionParam("localhost", httpPort, wsPort)){
			return false;
		}

		if (!serverControllerPtr->StartServer(imtcom::IServerConnectionInterface::PT_HTTP)){
			return false;
		}

		if (!serverControllerPtr->StartServer(imtcom::IServerConnectionInterface::PT_WEBSOCKET)){
			return false;
		}

		return true;
	}


	bool Stop()
	{
		imtcom::IServerDispatcher* serverControllerPtr = m_sdk.GetInterface<imtcom::IServerDispatcher>();
		if (serverControllerPtr == nullptr){
			return false;
		}

		if (!serverControllerPtr->StopServer(imtcom::IServerConnectionInterface::PT_HTTP)){
			return false;
		}

		if (!serverControllerPtr->StopServer(imtcom::IServerConnectionInterface::PT_WEBSOCKET)){
			return false;
		}

		return true;
	}


	bool SetFeaturesFilePath(const QString& filePath)
	{
		imtlic::IProductInfo* productInfoPtr = m_sdk.GetInterface<imtlic::IProductInfo>();
		if (productInfoPtr == nullptr){
			return false;
		}

		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly)){
			return false;
		}

		QByteArray data = file.readAll();
		file.close();

		iser::CCompactXmlMemReadArchive archive(data);

		if (!productInfoPtr->Serialize(archive)){
			return false;
		}

		return true;
	}


	bool SetConnectionParam(const QString& host, int httpPort, int wsPort)
	{
		return SetConnectionParamInternal(host, httpPort, wsPort, "ServerConnectionInterfaceParam");
	}


	bool SetPumaConnectionParam(const QString& host, int httpPort, int wsPort)
	{
		return SetConnectionParamInternal(host, httpPort, wsPort, "PumaConnectionInterfaceParam");
	}


	bool SetProductId(const QByteArray& productId)
	{
		imtbase::IApplicationInfoController* applicationInfoControllerPtr = m_sdk.GetInterface<imtbase::IApplicationInfoController>();
		if (applicationInfoControllerPtr == nullptr){
			return false;
		}

		applicationInfoControllerPtr->SetApplicationAttribute(imtbase::IApplicationInfoController::ApplicationAttribute::AA_APPLICATION_ID, productId);

		return true;
	}

private:
	bool SetConnectionParamInternal(const QString& host, int httpPort, int wsPort, const QByteArray& componentId)
	{
		imtcom::IServerConnectionInterface* serverConnectionParamPtr = m_sdk.GetInterface<imtcom::IServerConnectionInterface>(componentId);
		if (serverConnectionParamPtr != nullptr){
			serverConnectionParamPtr->SetHost(host);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_HTTP, httpPort);
			serverConnectionParamPtr->SetPort(imtcom::IServerConnectionInterface::PT_WEBSOCKET, wsPort);

			return true;
		}

		return false;
	}

private:
	mutable CAuthServerSdk m_sdk;
};


// public methods

CAuthorizableServer::CAuthorizableServer()
	: m_implPtr(nullptr)
{
	m_implPtr = new CAuthorizableServerImpl;
}


CAuthorizableServer::~CAuthorizableServer()
{
	delete m_implPtr;
}


bool CAuthorizableServer::Start(int httpPort, int wsPort) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->Start(httpPort, wsPort);
	}

	return false;
}


bool CAuthorizableServer::Stop() const
{
	if (m_implPtr != nullptr){
		return m_implPtr->Stop();
	}

	return false;
}


bool CAuthorizableServer::SetFeaturesFilePath(const QString& filePath) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetFeaturesFilePath(filePath);
	}

	return false;
}


bool CAuthorizableServer::SetPumaConnectionParam(const QString& host, int httpPort, int wsPort) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetPumaConnectionParam(host, httpPort,  wsPort);
	}

	return false;
}


bool CAuthorizableServer::SetProductId(const QByteArray& productId) const
{
	if (m_implPtr != nullptr){
		return m_implPtr->SetProductId(productId);
	}

	return false;
}


} // namespace AuthServerSdk


