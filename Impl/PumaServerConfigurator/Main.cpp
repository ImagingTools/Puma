// ImtCore includes
#include <imtbase/Init.h>
#include <imtqml/CQmlProcess.h>

// Puma includes
#include <GeneratedFiles/PumaServerConfigurator/CPumaServerConfigurator.h>


int main(int argc, char *argv[])
{
	Q_INIT_RESOURCE(PumaServerConfigurator);
	Q_INIT_RESOURCE(pumaqml);
	qmlRegisterType<imtqml::CQmlProcess>("imtqml", 1, 0, "Process");
	return Run<CPumaServerConfigurator, DefaultImtCoreQmlInitializer>(argc, argv);
}


