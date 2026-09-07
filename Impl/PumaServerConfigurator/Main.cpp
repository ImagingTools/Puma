// SPDX-License-Identifier: LicenseRef-Puma-Commercial


// ImtCore includes
#include <imtcore/CApplicationRunner.h>
#include <imtcore/CImtCoreAuthInitializer.h>
#include <imtcore/CImtCoreBaseInitializer.h>
#include <imtcore/CImtCoreLocalizationInitializer.h>
#include <imtcore/CImtCoreStyleInitializer.h>

// Puma includes
#include <GeneratedFiles/PumaServerConfigurator/CPumaServerConfigurator.h>


static void InitializePumaServerConfiguratorResources()
{
	ImtCoreInitLocalizationResources();
	ImtCoreInitBaseResources();

	ImtCoreInitStyleResources();
	ImtCoreInitAuthStyleResources();

	ImtCoreInitQmlApplicationCoreResources();
	ImtCoreInitQmlDocumentManagementResources();
	ImtCoreInitAuthQmlResources();

	InitializeImtCoreStyle();
}


int main(int argc, char* argv[])
{
	Q_INIT_RESOURCE(PumaServerConfigurator);
	Q_INIT_RESOURCE(pumaqml);

	InitializePumaServerConfiguratorResources();

	CPumaServerConfigurator instance;
	return imtcore::CApplicationRunner::Run(argc, argv, instance);
}
