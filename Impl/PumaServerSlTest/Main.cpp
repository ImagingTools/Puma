// SPDX-License-Identifier: LicenseRef-Puma-Commercial


// ImtCore includes
#include <imtcore/CApplicationRunner.h>
#include <imtcore/CImtCoreAuthorizableServerInitializer.h>

// Puma includes
#include <GeneratedFiles/PumaServerSlTest/CPumaServerSlTest.h>


int main(int argc, char* argv[])
{
	InitializeImtCoreAuthorizableServer();
	Q_INIT_RESOURCE(imtresthtml);
	Q_INIT_RESOURCE(PumaServerSlTest);

	CPumaServerSlTest instance;
	return imtcore::CApplicationRunner::Run(argc, argv, instance);
}
