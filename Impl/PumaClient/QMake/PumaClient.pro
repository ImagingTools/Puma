TARGET = PumaClient

include($(ACFDIR)/Config/QMake/ApplicationConfig.pri)
include($(ACFDIR)/Config/QMake/QtBaseConfig.pri)

INCLUDEPATH += ../../../Include
INCLUDEPATH += ../../../Impl
INCLUDEPATH +=  $$AUXINCLUDEDIR

RESOURCES += $$files($$_PRO_FILE_PWD_/../*.qrc, false)

LIBS += -L../../../Lib/$$COMPILER_DIR -liauth -limeas -liqtmeas -liqtinsp -liproc -liinsp -liipr -liprocgui -lisig -liqtsig -licalibgui -licalib -licam -liqtcam -lAcfSlnLoc -lAcfLoc -liedge -liedgegui -li2d -liqt2d -liedge 
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtbase -limtdev -limtdb -limtgui -limtqml -limtlog -limtdocguiqml -limtloggui -limtwidgets -limtstyle -limtrest -limt3d -limtlic -limtlicgui -limt3dgui -limt3dview -lImtCoreLoc -limtauthgui -limtcrypt -limtdbgui -limtauth -limtauthgql -limtguigql -limtauthdb -limtcom -limtlicgql -limtgql -limtrepo -limtdesign -limtclientgql -limtapp -limtcol -limtservergql
LIBS += -limtguiqml -limtdocguiqml -limtauthguiqml -limtlicguiqml -limtcolguiqml -limtguigqlqml -limtcontrolsqml -limtstylecontrolsqml
LIBS += -limtauthsdl -limtappsdl -limtlicsdl -limtbasesdl -limtcolorsdl


# Set OS-specific build options:
win32-msvc*{
	QMAKE_CXXFLAGS += /wd4264

	# copying all Qt DLLs to destination directory
	greaterThan(QT_MAJOR_VERSION, 4): QMAKE_POST_LINK = set path=$(QTDIR)\bin;%path% && $(QTDIR)\bin\windeployqt --qmldir=$(IMTCOREDIR)/Qml/imtgui --qmldir=$(IMTCOREDIR)/Qml/imtlicgui $$DESTDIR
}

# Set configuration of custom builds:
# ARX Compiler:
ARXC_CONFIG = $$PWD/../../../Config/PumaServer.awc
ARXC_FILES += $$PWD/../PumaClient.acc
ARXC_OUTDIR = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET

# Conversion of resource templates:
win*{
	# File transformation
	ACF_CONVERT_FILES = $$PWD/../VC/PumaClient.rc.xtracf
	ACF_CONVERT_OUTDIR = $$AUXINCLUDEPATH/GeneratedFiles/$$TARGET
	ACF_CONVERT_REGISTRY =  $$PWD/../VC/FileSubstitCopyApp.acc
	ACF_CONVERT_CONFIG = $$PWD/../../../Config/BaseOnly.awc

	RC_FILE = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET/PumaClient.rc
	RC_INCLUDEPATH = $$_PRO_FILE_PWD_
}

include($(ACFDIR)/Config/QMake/AcfQt.pri)
include($(ACFDIR)/Config/QMake/AcfStd.pri)
include($(ACFDIR)/Config/QMake/CustomBuild.pri)
