TARGET = PumaSettingsPlugin

ARXC_CONFIG = $$PWD/../../../../Config/PumaServer.awc
ARXC_FILES += $$PWD/../PumaSettingsPlugin.acc

include($(IMTCOREDIR)/Config/QMake/Plugin.pri)
include($(IMTCOREDIR)/Config/QMake/ImtCore.pri)

DESTDIR = $$OUT_PWD/../../../../Bin/$$COMPILER_DIR/Plugins
OBJECTS_DIR = ../$$AUXINCLUDEPATH/GeneratedFiles/$$CONFIGURATION_NAME/"$$TARGET"

QT += sql

LIBS += -L$(ACFDIR)/Lib/$$COMPILER_DIR -lifile -listd -lidoc
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtauth -limtbase -limtapp -limtdb -limtfile -limtservice

