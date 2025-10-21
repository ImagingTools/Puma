#pragma once


// Qt includes
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QByteArrayList>


namespace AuthClientSdk
{


class CAuthorizationControllerImpl;


struct Login
{
	QByteArray accessToken;
	QString userName;
	QByteArray productId;
	QByteArrayList permissions;
};


/**
*	\ingroup AuthClientSdk
*/
class CAuthorizationController
{
public:
	CAuthorizationController();

	virtual ~CAuthorizationController();

	virtual bool Login(const QString& login, const QString& password, Login& out);
	virtual bool Logout();
	virtual bool HasPermission(const QByteArray& permissionId);
	virtual QByteArray GetToken() const;
	virtual void SetProductId(const QByteArray& productId);

private:
	CAuthorizationControllerImpl* m_implPtr;
};


} // namespace AuthClientSdk


