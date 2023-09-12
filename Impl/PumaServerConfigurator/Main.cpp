// Qt includes
#include <QtQml/QQmlEngine>
#include <QtCore/QCoreApplication>
#include <QtWidgets/QApplication>
#include <QtQml>

// ACF includes
#include <ibase/IApplication.h>

// ImtCore includes
#include <GeneratedFiles/PumaServerConfigurator/CPumaServerConfigurator.h>
#include <imtbase/CTreeItemModel.h>
#include <imtqml/CGqlModel.h>
#include <imtqml/CRemoteFileController.h>


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(PumaServerConfigurator);
	Q_INIT_RESOURCE(imtgui);
	Q_INIT_RESOURCE(imtqml);
	Q_INIT_RESOURCE(imtstyleqml);
	Q_INIT_RESOURCE(pumaqml);
	Q_INIT_RESOURCE(imtguiTheme);

	qmlRegisterType<imtbase::CTreeItemModel>("Acf", 1, 0, "TreeItemModel");
	qmlRegisterType<imtqml::CGqlModel>("Acf", 1, 0, "GqlModel");
	qmlRegisterType<imtqml::CRemoteFileController>("Acf", 1, 0, "RemoteFileController");

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
	qmlRegisterModule("QtGraphicalEffects", 1, 12);
	qmlRegisterModule("QtGraphicalEffects", 1, 0);
	qmlRegisterModule("QtQuick.Dialogs", 1, 3);
#else
	qmlRegisterModule("QtQuick.Dialogs", 6, 2);
	qmlRegisterModule("Qt5Compat.GraphicalEffects", 1, 0);
#endif

	CPumaServerConfigurator instance;

	ibase::IApplication* applicationPtr = instance.GetInterface<ibase::IApplication>();
	if (applicationPtr != nullptr){
		return applicationPtr->Execute(argc, argv);
	}

	return -1;
}


