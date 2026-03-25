// SPDX-License-Identifier: LicenseRef-Puma-Commercial
// Qt includes
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>

// ACF includes
#include <ibase/IApplication.h>

// ImtCore includes
#include <GeneratedFiles/PumaServerPg/CPumaServerPg.h>


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(imtresthtml);
	Q_INIT_RESOURCE(imtdb);
	Q_INIT_RESOURCE(imtauthdb);
	Q_INIT_RESOURCE(imtbase);

	Q_INIT_RESOURCE(PumaServerPg);

	CPumaServerPg instance;

	ibase::IApplication* applicationPtr = instance.GetInterface<ibase::IApplication>();
	if (applicationPtr != nullptr){
		return applicationPtr->Execute(argc, argv);
	}

	return -1;
}

