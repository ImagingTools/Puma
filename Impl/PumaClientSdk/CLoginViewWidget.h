#pragma once


// Qt includes
#include <QtWidgets/QWidget>

// PumaClientSdk includes
#include <PumaClientSdk/PumaClientSdk.h>


namespace PumaClientSdk
{


class CLoginViewWidgetImpl;


class CLoginViewWidget: public QWidget
{
public:
	CLoginViewWidget();
	~CLoginViewWidget();

	bool SetConnectionParam(const QString& host, int httpPort, int wsPort);
	bool SetLoginParam(Login param);

private:
	CLoginViewWidgetImpl* m_implPtr;
};


} // namespace PumaClientSdk


