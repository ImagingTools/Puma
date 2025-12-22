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
#include <QtNetwork/QSslConfiguration>


namespace AuthServerSdk
{


class CAuthorizableServerImpl;


struct SslConfig
{
	// Local (server) certificate
	QString localCertificatePath;
	QSsl::EncodingFormat localCertificateFormat = QSsl::Pem;

	// Private key
	QString privateKeyPath;
	QSsl::KeyAlgorithm privateKeyAlgorithm = QSsl::Rsa;
	QSsl::EncodingFormat privateKeyFormat = QSsl::Pem;
	QByteArray privateKeyPassPhrase;

	// Trusted CAs
	QList<QString> caCertificatePaths; 
	QSsl::EncodingFormat caCertificateFormat = QSsl::Pem;

	QSslSocket::PeerVerifyMode verifyMode = QSslSocket::PeerVerifyMode::AutoVerifyPeer;
	QSsl::SslProtocol protocol = QSsl::TlsV1_2;
};


struct ServerConfig
{
	int httpPort = 80;
	int wsPort = 90;
	QString host = "localhost";
	std::optional<SslConfig> sslConfig;
};


/**
*	\ingroup AuthServerSdk
*/
class AUTH_SERVER_SDK_EXPORT CAuthorizableServer
{
public:
	CAuthorizableServer();
	virtual ~CAuthorizableServer();
	virtual bool Start(const ServerConfig& serverConfig) const;
	virtual bool Stop() const;
	virtual bool SetFeaturesFilePath(const QString& filePath) const;
	virtual bool SetPumaConnectionParam(const ServerConfig& serverConfig) const;
	virtual bool SetProductId(const QByteArray& productId) const;

private:
	CAuthorizableServerImpl* m_implPtr;
};


} // namespace AuthServerSdk


/**
\defgroup AuthServerSdk Autorizable Server SDK

\mainpage
\section Introduction
*/

