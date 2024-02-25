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

# Plug-ins
SUBDIRS += PumaSettingsPlugin
PumaSettingsPlugin.file = ../../Impl/Plugins/PumaSettingsPlugin/QMake/PumaSettingsPlugin.pro


