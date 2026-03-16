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
SUBDIRS += PumaServerPg
PumaServerPg.file = ../../Impl/PumaServerPg/QMake/PumaServerPg.pro
PumaServerPg.depends = pumaqml

SUBDIRS += PumaServerSl
PumaServerSl.file = ../../Impl/PumaServerSl/QMake/PumaServerSl.pro
PumaServerSl.depends = pumaqml

SUBDIRS += PumaServerConfigurator
PumaServerConfigurator.file = ../../Impl/PumaServerConfigurator/QMake/PumaServerConfigurator.pro
PumaServerConfigurator.depends = PumaServerPg PumaServerSl

# Plug-ins
SUBDIRS += PumaSettingsPlugin
PumaSettingsPlugin.file = ../../Impl/Plugins/PumaSettingsPlugin/QMake/PumaSettingsPlugin.pro
PumaSettingsPlugin.depends = PumaServerConfigurator PumaServerPg PumaServerSl

