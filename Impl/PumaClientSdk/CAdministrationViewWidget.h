#pragma once


// Qt includes
#include <QtWidgets/QWidget>


namespace PumaClientSdk
{


class CAdministrationViewWidgetImpl;


class CAdministrationViewWidget: public QWidget
{
public:
	CAdministrationViewWidget();
	~CAdministrationViewWidget();

	QWidget* GetWidget() const;
	bool SetConnectionParam(const QString& host, int httpPort, int wsPort);
	bool SetProductId(const QByteArray& productId);

private:
	CAdministrationViewWidgetImpl* m_implPtr;
};


} // namespace PumaClientSdk


