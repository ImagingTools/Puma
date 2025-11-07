# Root of Puma project
TEMPLATE = subdirs

# Include
SUBDIRS += pumaqml
pumaqml.file = ../../Include/pumaqml/QMake/pumaqml.pro

SUBDIRS += AuthServerSdk
AuthServerSdk.file = ../../Impl/AuthServerSdk/QMake/AuthServerSdk.pro

SUBDIRS += AuthClientSdk
AuthClientSdk.file = ../../Impl/AuthClientSdk/QMake/AuthClientSdk.pro

SUBDIRS += PumaClient
PumaClient.file = ../../Impl/PumaClient/QMake/PumaClient.pro
PumaClient.depends = AuthServerSdk AuthClientSdk

# Application
SUBDIRS += PumaServer
PumaServer.file = ../../Impl/PumaServer/QMake/PumaServer.pro
PumaServer.depends = pumaqml

SUBDIRS += PumaServerConfigurator
PumaServerConfigurator.file = ../../Impl/PumaServerConfigurator/QMake/PumaServerConfigurator.pro
PumaServerConfigurator.depends = PumaServer

# Plug-ins
SUBDIRS += PumaSettingsPlugin
PumaSettingsPlugin.file = ../../Impl/Plugins/PumaSettingsPlugin/QMake/PumaSettingsPlugin.pro
PumaSettingsPlugin.depends = PumaServerConfigurator PumaServer

