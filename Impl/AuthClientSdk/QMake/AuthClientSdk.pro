TARGET = AuthClientSdk

include($(ACFDIR)/Config/QMake/SharedLibraryConfig.pri)
include($(PUMADIR)/Config/QMake/Puma.pri)

INCLUDEPATH += $(ACFDIR)/Include
INCLUDEPATH += $(ACFDIR)/Impl

LIBS += -L$(ACFDIR)/Lib/$$COMPILER_DIR
LIBS += -L../../../Lib/$$COMPILER_DIR

# Set configuration of custom builds:
# ARX Compiler:
ARXC_FILES += $$PWD/../AuthClientSdk.acc
ARXC_FILES += $$PWD/../AdministrationWidget.acc
ARXC_FILES += $$PWD/../LoginWidget.acc
ARXC_CONFIG = $$PWD/../../../Config/PumaServer.awc
ARXC_OUTDIR = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET

LIBS += -L../../../Lib/$$COMPILER_DIR
LIBS += -L$(ACFSLNDIR)/Lib/$$COMPILER_DIR -liauth -licomm
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtservergql -limtauth -limtbase -limtdoc -limtdb -limtgui -limtauthgui -limtauthgql -limtqml -limtlog -limtdocguiqml -limtloggui -limtwidgets -limtstyle -limtrest -limt3d -limtlic -limtlicgui -lImtCoreLoc -limtauthgui -limtcrypt -limtdbgui -limtauthgql -limtguigql -limtcom -limtlicgql -limtgql -limtdesign -limtclientgql -limtapp -limtcol
LIBS += -limtguiqml -limtdocguiqml -limtauthguiqml -limtlicguiqml -limtcolguiqml -limtguigqlqml -limtcontrolsqml -limtstylecontrolsqml
LIBS += -limtauthsdl -limtappsdl -limtlicsdl -limtbasesdl -limtcolorsdl -limtserverapp

include($(ACFDIR)/Config/QMake/AcfQt.pri)
include($(ACFDIR)/Config/QMake/AcfStd.pri)
include($(ACFDIR)/Config/QMake/CustomBuild.pri)
