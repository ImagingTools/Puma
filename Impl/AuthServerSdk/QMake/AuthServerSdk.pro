TARGET = AuthServerSdk

include($(ACFDIR)/Config/QMake/StaticConfig.pri)
include($(PUMADIR)/Config/QMake/Puma.pri)

INCLUDEPATH += $(ACFDIR)/Include
INCLUDEPATH += $(ACFDIR)/Impl

LIBS += -L$(ACFDIR)/Lib/$$COMPILER_DIR
LIBS += -L$(ACFSLNDIR)/Lib/$$COMPILER_DIR

LIBS += -L../../../Lib/$$COMPILER_DIR -liauth -limeas -liqtmeas -liqtinsp -liproc -liinsp -liipr -liprocgui -lisig -liqtsig -licalibgui -licalib -licam -liqtcam -lAcfSlnLoc -lAcfLoc -liedge -liedgegui -li2d -liqt2d -liedge 
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtbase -limtdev -limtdb -limtgui -limtqml -limtlog -limtdocguiqml -limtloggui -limtwidgets -limtstyle -limtrest -limt3d -limtlic -limtlicgui -limt3dgui -limt3dview -lImtCoreLoc -limtauthgui -limtcrypt -limtdbgui -limtauth -limtauthgql -limtguigql -limtauthdb -limtcom -limtlicgql -limtgql -limtrepo -limtdesign -limtclientgql -limtapp -limtcol -limtservergql
LIBS += -limtguiqml -limtdocguiqml -limtauthguiqml -limtlicguiqml -limtcolguiqml -limtguigqlqml -limtcontrolsqml -limtstylecontrolsqml
LIBS += -limtauthsdl -limtappsdl -limtlicsdl -limtbasesdl -limtcolorsdl

# Set configuration of custom builds:
# ARX Compiler:
ARXC_CONFIG = $$PWD/../../../Config/PumaServer.awc
ARXC_FILES += $$PWD/../AuthServerSdk.acc
ARXC_OUTDIR = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET

include($(ACFDIR)/Config/QMake/AcfQt.pri)
include($(ACFDIR)/Config/QMake/AcfStd.pri)
include($(ACFDIR)/Config/QMake/CustomBuild.pri)
