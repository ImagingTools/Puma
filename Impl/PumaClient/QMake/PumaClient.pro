TARGET = PumaClient

include($(ACFDIR)/Config/QMake/ApplicationConfig.pri)
include($(ACFDIR)/Config/QMake/QtBaseConfig.pri)
include($(IMTCOREDIR)/Config/QMake/ImtCore.pri)

INCLUDEPATH += ../../../Include
INCLUDEPATH += ../../../Impl
INCLUDEPATH +=  $$AUXINCLUDEDIR

RESOURCES += $$files($$_PRO_FILE_PWD_/../*.qrc, false)

LIBS += -L$(ACFDIR)/Lib/$$COMPILER_DIR
LIBS += -L$(ACFSLNDIR)/Lib/$$COMPILER_DIR

LIBS += -L../../../Bin/$$COMPILER_DIR -lAuthClientSdk -lAuthServerSdk

QT += xml network sql quick qml websockets

# Set OS-specific build options:
win32-msvc*{
	QMAKE_CXXFLAGS += /wd4264

	# copying all Qt DLLs to destination directory
	greaterThan(QT_MAJOR_VERSION, 4): QMAKE_POST_LINK += set path=$(QTDIR)\bin;%path% && $(QTDIR)\bin\windeployqt $$DESTDIR/$${TARGET}.exe --qmldir=$(IMTCOREDIR)/Qml
}
