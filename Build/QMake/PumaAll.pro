# Root of Puma project
TEMPLATE = subdirs

# Application
SUBDIRS += PumaServer
PumaServer.file = ../../Impl/PumaServer/QMake/PumaServer.pro

SUBDIRS += PumaServerConfigurator
PumaServerConfigurator.file = ../../Impl/PumaServer/QMake/PumaServerConfigurator.pro

