TARGET = pumatest

# Console subsystem: without this, QTest's output goes to OutputDebugString on
# Windows and is invisible when the process is launched with stdout/stderr
# redirected to a file (the normal way to capture a CI/CLI test run).
CONFIG += console

include($(ACFDIR)/Config/QMake/ApplicationConfig.pri)
include($(IMTCOREDIR)/Config/QMake/ImtCore.pri)
include($(PUMADIR)/Config/QMake/Puma.pri)

RESOURCES += $$files($$_PRO_FILE_PWD_/../*.qrc, false)

LIBS += -L$(ACFDIR)/Lib/$$COMPILER_DIR -litest
LIBS += -L$(ACFSLNDIR)/Lib/$$COMPILER_DIR -litest -liauth -liservice -lifile -liser
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtbase -limtauth -limtcrypt -limtservergql -limtdb -limtgql -limtauthgql -limtbasesdl -limtauthsdl -limtcol

QT += xml test sql widgets core
