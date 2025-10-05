#pragma once


#if defined _MSC_VER || defined __MINGW32__ || defined __MINGW64__
#define PUMA_CLIENT_SDK_EXPORT __declspec(dllexport)
#elif defined __GNUC__
#define PUMA_CLIENT_SDK_EXPORT __attribute__((visibility("default")))
#else
#define PUMA_CLIENT_SDK_EXPORT
#endif

// Qt includes
#include <QtCore/QString>
#include <QtCore/QByteArray>


namespace PumaClientSdk
{


class CAuthorizationControllerImpl;


/**
*	\ingroup PumaClientSdk
*/
class PUMA_CLIENT_SDK_EXPORT CAuthorizationController
{
public:
	CAuthorizationController();

	virtual ~CAuthorizationController();

	virtual bool Login(const QString& login, const QString& password);
	virtual bool Logout();
	virtual bool HasPermission(const QByteArray& permissionId);
	virtual QByteArray GetToken() const;
	virtual void SetProductId(const QByteArray& productId);

private:
	CAuthorizationControllerImpl* m_implPtr;
};


} // namespace PumaClientSdk


/**
\defgroup PumaClientSdk PUMA SDK

\mainpage
\section Introduction
*/

