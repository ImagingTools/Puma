TARGET = AuthServerSdk

include($(ACFDIR)/Config/QMake/SharedLibraryConfig.pri)
include($(PUMADIR)/Config/QMake/Puma.pri)

INCLUDEPATH += $(ACFDIR)/Include
INCLUDEPATH += $(ACFDIR)/Impl

LIBS += -L$(ACFDIR)/Lib/$$COMPILER_DIR
LIBS += -L$(ACFSLNDIR)/Lib/$$COMPILER_DIR

LIBS += -L../../../Lib/$$COMPILER_DIR -licomm -limeas -lAcfSlnLoc -lAcfLoc
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtbase -limtdev -limtdb -limtqml -limtlog -limtstyle -limtrest -limtlic -lImtCoreLoc -limtcrypt -limtauth -limtauthgql -limtauthdb -limtcom -limtlicgql -limtgql -limtrepo -limtdesign -limtapp -limtcol
LIBS += -limtguiqml -limtdocguiqml -limtauthguiqml -limtlicguiqml -limtcolguiqml -limtguigqlqml -limtcontrolsqml -limtstylecontrolsqml -limtdocguiqml
LIBS += -limtauthsdl -limtappsdl -limtlicsdl -limtbasesdl -limtcolorsdl
LIBS += -limtservergql -limtserverapp -limtclientgql

# Set configuration of custom builds:
# ARX Compiler:
ARXC_CONFIG = $$PWD/../../../Config/PumaServer.awc
ARXC_FILES += $$PWD/../AuthServerSdk.acc
ARXC_OUTDIR = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET

include($(ACFDIR)/Config/QMake/AcfQt.pri)
include($(ACFDIR)/Config/QMake/AcfStd.pri)
include($(ACFDIR)/Config/QMake/CustomBuild.pri)
