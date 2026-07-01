// SPDX-License-Identifier: LicenseRef-Puma-Commercial


// Qt includes
#include <QtCore/QCoreApplication>
#include <cstdio>

// ACF includes
#include <ibase/IApplication.h>

// ImtCore includes
#include <GeneratedFiles/PumaServerSlTest/CPumaServerSlTest.h>


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(imtresthtml);
	Q_INIT_RESOURCE(imtdb);
	Q_INIT_RESOURCE(imtauthdb);
	Q_INIT_RESOURCE(imtbase);

	Q_INIT_RESOURCE(PumaServerSlTest);

	CPumaServerSlTest instance;

	ibase::IApplication* applicationPtr = instance.GetInterface<ibase::IApplication>();
	if (applicationPtr != nullptr){
		return applicationPtr->Execute(argc, argv);
	}

	return -1;
}
