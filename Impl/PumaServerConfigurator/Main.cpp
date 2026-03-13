// SPDX-License-Identifier: LicenseRef-Puma-Commercial
// ImtCore includes
#include <imtbase/Init.h>

// Puma includes
#include <GeneratedFiles/PumaServerConfigurator/CPumaServerConfigurator.h>


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(PumaServerConfigurator);
	Q_INIT_RESOURCE(pumaqml);
	return Run<CPumaServerConfigurator, DefaultImtCoreQmlInitializer>(argc, argv);
}


