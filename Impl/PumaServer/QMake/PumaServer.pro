TARGET = PumaServer

include($(ACFDIR)/Config/QMake/ApplicationConfig.pri)
include($(ACFDIR)/Config/QMake/QtBaseConfig.pri)
include($(PUMADIR)/Config/QMake/Puma.pri)
include($(IMTCOREDIR)/Config/QMake/Quazip.pri)
include($(IMTCOREDIR)/Config/QMake/OpenSSL.pri)

INCLUDEPATH += ../../../Include
INCLUDEPATH += ../../../Impl
INCLUDEPATH +=  $$AUXINCLUDEDIR

RESOURCES += $$files($$_PRO_FILE_PWD_/../*.qrc, false)

LIBS += -L../../../Lib/$$COMPILER_DIR -liauth -liservice -lAcfSlnLoc -lAcfLoc
LIBS +=  -limtbase -limtdev -limtdb -limtgui -limtqml -limtlog -limtloggui -limtwidgets -limtstyle -limtrest -limt3d -limtlic -lImtCoreLoc -limtcrypt -limtauth -limtauthgql -limtbase -limtgql -limtauthdb -limtcom -limtlicgql -limtguigql 


HEADERS =

QT += xml network sql quick qml


# Set OS-specific build options:
win32-msvc*{
	QMAKE_CXXFLAGS += /wd4264

	# copying all Qt DLLs to destination directory
	greaterThan(QT_MAJOR_VERSION, 4): QMAKE_POST_LINK = set path=$(QTDIR)\bin;%path% && $(QTDIR)\bin\windeployqt --qmldir=$$(PUMADIR)\..\Puma\Impl\PumaServer\Resources\qml\ --qmldir=$$PWD\..\Resources\qml\ $$DESTDIR
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
	ACF_CONVERT_FILES = $$PWD/../VC/PumaServer.rc.xtracf
	ACF_CONVERT_OUTDIR = $$AUXINCLUDEPATH/GeneratedFiles/$$TARGET
	ACF_CONVERT_REGISTRY =  $$PWD/../VC/FileSubstitCopyApp.acc
	ACF_CONVERT_CONFIG = $$PWD/../../../Config/BaseOnly.awc

	RC_FILE = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET/PumaServer.rc
	RC_INCLUDEPATH = $$_PRO_FILE_PWD_
}

include($(ACFDIR)/Config/QMake/AcfQt.pri)
include($(ACFDIR)/Config/QMake/AcfStd.pri)
include($(ACFDIR)/Config/QMake/CustomBuild.pri)

OTHER_FILES += \
    $$PWD/../Migrations

win32 {
    PWD_WIN = $${PWD}/../Migrations
    DESTDIR_WIN = $${DESTDIR}/Migrations
    PWD_WIN ~= s,/,\\,g
    DESTDIR_WIN ~= s,/,\\,g
    message(PWD_WIN = $${PWD_WIN})
    message(DESTDIR_WIN = $${DESTDIR_WIN})
    message(cmd = $$quote(cmd /c xcopy /S /I $${PWD_WIN} $${DESTDIR_WIN}))
    copyfiles.commands = $$quote(cmd /c xcopy /S /I /Y $${PWD_WIN} $${DESTDIR_WIN})
}
macx {
    copyfiles.commands = cp -r $$PWD/../Migrations/ $$OUT_PWD
}
linux {
    copyfiles.commands = cp -r $$PWD/Migrations/ $$OUT_PWD
}

QMAKE_EXTRA_TARGETS += copyfiles
POST_TARGETDEPS += copyfiles

