#pragma once


#if defined _MSC_VER || defined __MINGW32__ || defined __MINGW64__
#define PUMA_CLIENT_SDK_EXPORT __declspec(dllexport)
#elif defined __GNUC__
#define PUMA_CLIENT_SDK_EXPORT __attribute__((visibility("default")))
#else
#define PUMA_CLIENT_SDK_EXPORT
#endif


// Qt includes
#include <QtWidgets/QWidget>

// PumaClientSdk includes
#include <PumaClientSdk/PumaClientSdk.h>


namespace PumaClientSdk
{


class CLoginViewWidgetImpl;


class PUMA_CLIENT_SDK_EXPORT CLoginViewWidget: public QWidget
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


