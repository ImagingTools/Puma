# Root of Puma project
TEMPLATE = subdirs

# Include
SUBDIRS += pumaqml
pumaqml.file = ../../Include/pumaqml/QMake/pumaqml.pro

# Application
SUBDIRS += PumaServer
PumaServer.file = ../../Impl/PumaServer/QMake/PumaServer.pro

SUBDIRS += PumaServerConfigurator
PumaServerConfigurator.file = ../../Impl/PumaServerConfigurator/QMake/PumaServerConfigurator.pro
PumaServerConfigurator.depends = PumaServer

SUBDIRS += PumaClient
PumaClient.file = ../../Impl/PumaClient/QMake/PumaClient.pro

SUBDIRS += AuthServerSdk
AuthServerSdk.file = ../../Impl/AuthServerSdk/QMake/AuthServerSdk.pro

SUBDIRS += AuthClientSdk
AuthClientSdk.file = ../../Impl/AuthClientSdk/QMake/AuthClientSdk.pro

# Plug-ins
SUBDIRS += PumaSettingsPlugin
PumaSettingsPlugin.file = ../../Impl/Plugins/PumaSettingsPlugin/QMake/PumaSettingsPlugin.pro
PumaSettingsPlugin.depends = PumaServerConfigurator PumaServer

