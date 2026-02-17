// SPDX-License-Identifier: MIT
// Qt includes
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>

// ACF includes
#include <ibase/IApplication.h>

// ImtCore includes
#include <GeneratedFiles/PumaServer/CPumaServer.h>


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(imtresthtml);
	Q_INIT_RESOURCE(imtdb);
	Q_INIT_RESOURCE(imtbase);

	CPumaServer instance;

	ibase::IApplication* applicationPtr = instance.GetInterface<ibase::IApplication>();
	if (applicationPtr != nullptr){
		return applicationPtr->Execute(argc, argv);
	}

	return -1;
}


