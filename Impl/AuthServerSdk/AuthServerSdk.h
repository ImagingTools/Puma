#pragma once


#if defined _MSC_VER || defined __MINGW32__ || defined __MINGW64__
#define AUTH_SERVER_SDK_EXPORT __declspec(dllexport)
#elif defined __GNUC__
#define AUTH_CLIENT_SDK_EXPORT __attribute__((visibility("default")))
#else
#define AUTH_CLIENT_SDK_EXPORT
#endif


// Qt includes
#include <QtCore/QString>
#include <QtCore/QByteArray>


namespace AuthServerSdk
{


class CAuthorizableServerImpl;


/**
*	\ingroup AuthServerSdk
*/
class AUTH_SERVER_SDK_EXPORT CAuthorizableServer
{
public:
	CAuthorizableServer();
	virtual ~CAuthorizableServer();
	virtual bool Start(int httpPort, int wsPort);
	virtual bool Stop();
	virtual bool SetFeaturesFilePath(const QString& filePath);
	virtual bool SetPumaConnectionParam(const QString& host, int httpPort, int wsPort);
	virtual bool SetProductId(const QByteArray& productId);

private:
	CAuthorizableServerImpl* m_implPtr;
};


} // namespace AuthServerSdk


/**
\defgroup AuthServerSdk Autorizable Server SDK

\mainpage
\section Introduction
*/

