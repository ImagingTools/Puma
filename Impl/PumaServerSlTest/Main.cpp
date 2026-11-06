// SPDX-License-Identifier: LicenseRef-Puma-Commercial


// ImtCore includes
#include <imtcore/CApplicationRunner.h>
#include <imtlic/IProductInfo.h>
#include <imtcore/CImtCoreAuthorizableServerInitializer.h>
#include <imtcore/CImtCoreNotifyInitializer.h>

// Puma includes
#include <puma/PumaFeatures.h>
#include <GeneratedFiles/PumaServerSlTest/CPumaServerSlTest.h>


int main(int argc, char* argv[])
{
	InitializeImtCoreAuthorizableServer();
	ImtCoreInitNotifySqlResources();
	Q_INIT_RESOURCE(imtresthtml);
	Q_INIT_RESOURCE(PumaServerSlTest);
#ifdef WEB_COMPILE
	Q_INIT_RESOURCE(pumaWeb);
#endif
	Q_INIT_RESOURCE(pumaqml);
	Q_INIT_RESOURCE(imtstylecontrolsqml);
	ImtCoreInitStyleResources();
	ImtCoreInitAuthStyleResources();

	CPumaServerSlTest instance;

	auto* productInfoPtr = instance.GetInterface<imtlic::IProductInfo>();
	if (productInfoPtr != nullptr){
		puma::FillProduct(*productInfoPtr);
	}

	return imtcore::CApplicationRunner::Run(argc, argv, instance);
}
