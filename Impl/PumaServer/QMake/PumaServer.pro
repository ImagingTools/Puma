TARGET = PumaServer

include($(ACFDIR)/Config/QMake/ApplicationConfig.pri)
include($(ACFDIR)/Config/QMake/QtBaseConfig.pri)
include($(PUMADIR)/Config/QMake/Puma.pri)
include($(IMTCOREDIR)/Config/QMake/Quazip.pri)
include($(IMTCOREDIR)/Config/QMake/OpenSSL.pri)

RESOURCES += $$files($$_PRO_FILE_PWD_/../*.qrc, false)

LIBS += -L$(ACFDIR)/Lib/$$COMPILER_DIR -lAcfLoc
LIBS += -L$(ACFSLNDIR)/Lib/$$COMPILER_DIR -liauth -liservice -lAcfSlnLoc
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtbase -limtdb -limtgui -limtqml -limtlog -limtloggui -limtstyle -limtrest -limtlic -lImtCoreLoc -limtcrypt -limtauth -limtauthgql -limtbase -limtgql -limtauthdb -limtcom -limtauthsdl
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtlicgql -limtguigql -limtapp -limtclientgql -limtservice

HEADERS =

QT += xml network sql quick qml websockets


# Set OS-specific build options:
win32-msvc*{
	QMAKE_CXXFLAGS += /wd4264

	# copying all Qt DLLs to destination directory
	greaterThan(QT_MAJOR_VERSION, 4): QMAKE_POST_LINK = set path=$(QTDIR)\bin;%path% && $(QTDIR)\bin\windeployqt $$DESTDIR
}

!macx-ios*{
	mac{
	#	ICON += $$PWD/../Mac/PumaServer.icns
	#	QMAKE_INFO_PLIST = $$PWD/../Mac/Info.plist
	}
}

# Set configuration of custom builds:
# ARX Compiler:
ARXC_CONFIG = $$PWD/../../../Config/PumaServer.awc
ARXC_FILES += $$PWD/../PumaServer.acc
ARXC_OUTDIR = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET

# Conversion of resource templates:
win*{
# File transformation
	# ACF_CONVERT_FILES = $$PWD/../VC/PumaServer.rc.xtracf
	ACF_CONVERT_OUTDIR = $$AUXINCLUDEPATH/GeneratedFiles/$$TARGET
	ACF_CONVERT_REGISTRY =  $$PWD/../VC/FileSubstitCopyApp.acc
	ACF_CONVERT_CONFIG = $$PWD/../../../Config/BaseOnly.awc

# RC_FILE = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET/PumaServer.rc
	RC_INCLUDEPATH = $$_PRO_FILE_PWD_
}

include($(ACFDIR)/Config/QMake/AcfQt.pri)
include($(ACFDIR)/Config/QMake/AcfStd.pri)
include($(ACFDIR)/Config/QMake/CustomBuild.pri)

