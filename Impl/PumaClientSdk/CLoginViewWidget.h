#pragma once


// Qt includes
#include <QtWidgets/QWidget>


namespace PumaClientSdk
{


class CLoginViewWidgetImpl;


class CLoginViewWidget: public QWidget
{
public:
	CLoginViewWidget();
	~CLoginViewWidget();

private:
	CLoginViewWidgetImpl* m_implPtr;
};


} // namespace PumaClientSdk


