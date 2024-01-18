TARGET = PumaServerConfigurator

include($(ACFDIR)/Config/QMake/ApplicationConfig.pri)
include($(ACFDIR)/Config/QMake/QtBaseConfig.pri)
include($(IMTCOREDIR)/Config/QMake/ImtCore.pri)
include($(IMTCOREDIR)/Config/QMake/OpenSSL.pri)

HEADERS =
QT += quick qml

INCLUDEPATH += ../../../Include
INCLUDEPATH += ../../../Impl
INCLUDEPATH +=  $$AUXINCLUDEDIR

RESOURCES += $$files($$_PRO_FILE_PWD_/../*.qrc, false)

LIBS += -limtbase -limtgui -limtstyle -limtqml -limtfile -limtgql -limtcom -limtdb -limtapp
LIBS += -limtcontrolsqml -limtstylecontrolsqml -limtguigqlqml -limtcolguiqml -limtdocguiqml -limtauthguiqml -limtlicguiqml -limtguiqml
LIBS += -lImtCoreLoc
LIBS += -lpumaqml

# Set OS-specific build options:
win32-msvc*{
	QMAKE_CXXFLAGS += /wd4264

	# copying all Qt DLLs to destination directory
        greaterThan(QT_MAJOR_VERSION, 4): QMAKE_POST_LINK = set path=$(QTDIR)\bin;%path% && $(QTDIR)\bin\windeployqt --qmldir=$(PUMADIR)/Impl/PumaServerConfigurator --qmldir=$(IMTCOREDIR)/Qml $$DESTDIR
}

# Set configuration of custom builds:
# ARX Compiler:
ARXC_CONFIG = $$PWD/../../../Config/PumaServer.awc
ARXC_FILES += $$PWD/../PumaServerConfigurator.acc
ARXC_OUTDIR = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET

# Conversion of resource templates:
# win*{
# 	# File transformation
# 	ACF_CONVERT_FILES = $$PWD/../VC/PumaServerConfigurator.rc.xtracf
# 	ACF_CONVERT_OUTDIR = $$AUXINCLUDEPATH/GeneratedFiles/$$TARGET
# 	ACF_CONVERT_REGISTRY =  $$PWD/../VC/FileSubstitCopyApp.acc
# 	ACF_CONVERT_CONFIG = $$PWD/../../../Config/BaseOnly.awc

#     RC_FILE = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET/PumaServerConfigurator.rc
# 	RC_INCLUDEPATH = $$_PRO_FILE_PWD_
# }

include($(ACFDIR)/Config/QMake/AcfQt.pri)
include($(ACFDIR)/Config/QMake/AcfStd.pri)
include($(ACFDIR)/Config/QMake/CustomBuild.pri)
