#pragma once


// Qt includes
#include <QtCore/QString>
#include <QtCore/QByteArray>


namespace AuthServerSdk
{


class CAuthorizableServerImpl;


/**
*	\ingroup AuthServerSdk
*/
class CAuthorizableServer
{
public:
	CAuthorizableServer();
	virtual ~CAuthorizableServer();
	virtual bool Start(int httpPort, int wsPort);
	virtual bool Stop();
	virtual bool SetFeaturesFilePath(const QString& filePath);
	virtual bool SetPumaConnectionParam(const QString& host, int httpPort, int wsPort);
	virtual bool SetProductId(const QByteArray& productId);
	virtual bool SetDatabaseSettings(const QString& host, int port, const QString& dbName, const QString& username, const QString& password);

private:
	CAuthorizableServerImpl* m_implPtr;
};


} // namespace AuthServerSdk


/**
\defgroup AuthServerSdk Autorizable Server SDK

\mainpage
\section Introduction
*/

