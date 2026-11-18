// SPDX-License-Identifier: LicenseRef-Puma-Commercial


// Qt includes
#include <imtcore/CApplicationRunner.h>
#include <imtlic/IProductInfo.h>
#include <imtcore/CImtCoreAuthorizableServerInitializer.h>
#include <imtcore/CImtCoreNotifyInitializer.h>

// Puma includes
#include <puma/PumaFeatures.h>
#include <GeneratedFiles/PumaServerPgTest/CPumaServerPgTest.h>


int main(int argc, char* argv[])
{
	InitializeImtCoreAuthorizableServer();
	ImtCoreInitNotifySqlResources();
	Q_INIT_RESOURCE(imtresthtml);
	Q_INIT_RESOURCE(PumaServerPgTest);
#ifdef WEB_COMPILE
	Q_INIT_RESOURCE(pumaWeb);
#endif
	Q_INIT_RESOURCE(pumaqml);
	Q_INIT_RESOURCE(imtstylecontrolsqml);
	ImtCoreInitStyleResources();
	ImtCoreInitAuthStyleResources();

	CPumaServerPgTest instance;

	auto* productInfoPtr = instance.GetInterface<imtlic::IProductInfo>();
	if (productInfoPtr != nullptr){
		puma::FillProduct(*productInfoPtr);
	}

	return imtcore::CApplicationRunner::Run(argc, argv, instance);
}
