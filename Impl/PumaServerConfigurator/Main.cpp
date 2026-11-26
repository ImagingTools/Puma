// SPDX-License-Identifier: LicenseRef-Puma-Commercial


// Qt includes
#include <QtQml/QQmlEngine>

// ImtCore includes
#include <imtcore/CApplicationRunner.h>
#include <imtcore/CImtCoreAuthInitializer.h>
#include <imtcore/CImtCoreBaseInitializer.h>
#include <imtcore/CImtCoreLocalizationInitializer.h>
#include <imtcore/CImtCoreStyleInitializer.h>
#include <imtauthgui/CLdapGroupTreeController.h>

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

	// Preview of the LDAP groups selected by the LDAP group filter (see PumaServerConfigurator.qml).
	qmlRegisterType<imtauthgui::CLdapGroupTreeController>("com.imtcore.imtauth", 1, 0, "LdapGroupTreeController");

	CPumaServerConfigurator instance;
	return imtcore::CApplicationRunner::Run(argc, argv, instance);
}
