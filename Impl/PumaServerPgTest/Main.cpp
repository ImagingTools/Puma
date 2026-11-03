// SPDX-License-Identifier: LicenseRef-Puma-Commercial


// Qt includes
#include <imtcore/CApplicationRunner.h>
#include <imtcore/CImtCoreAuthorizableServerInitializer.h>
#include <imtcore/CImtCoreNotifyInitializer.h>

// Puma includes
#include <GeneratedFiles/PumaServerPgTest/CPumaServerPgTest.h>


int main(int argc, char* argv[])
{
	InitializeImtCoreAuthorizableServer();
	ImtCoreInitNotifySqlResources();
	Q_INIT_RESOURCE(imtresthtml);
	Q_INIT_RESOURCE(PumaServerPgTest);

	CPumaServerPgTest instance;
	return imtcore::CApplicationRunner::Run(argc, argv, instance);
}
