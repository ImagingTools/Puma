TARGET = AuthClientSdk

include($(ACFDIR)/Config/QMake/SharedLibraryConfig.pri)
include($$PWD/../../../Config/QMake/Puma.pri)


DESTDIR = $$OUT_PWD/../../../Bin/$$COMPILER_DIR
OBJECTS_DIR = ../$$AUXINCLUDEPATH/GeneratedFiles/$$CONFIGURATION_NAME/"$$TARGET"

LIBS += -L../../../Lib/$$COMPILER_DIR -liproc

LIBS += -limtbase -limtlic -limtcrypt

# Set configuration of custom builds:
# ARX Compiler:
ARXC_FILES += $$PWD/../AuthClientSdk.acc
ARXC_CONFIG = $$PWD/../../../Config/PumaServer.awc
ARXC_OUTDIR = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET

include($(ACFDIR)/Config/QMake/AcfQt.pri)
include($(ACFDIR)/Config/QMake/AcfStd.pri)
include($(ACFDIR)/Config/QMake/CustomBuild.pri)

LIBS += -L$(ACFDIR)/Lib/$$COMPILER_DIR -limeas -liproc -liqtgui -liauth -licomp
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtbase -limtbasesdl -limtauthsdl -limtgql -limtcom -limtservergql -limtauth -limtlic -limtauthgql -limtclientgql -limtrest -limtserverapp -limtcol
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtauthgui -limtqml

