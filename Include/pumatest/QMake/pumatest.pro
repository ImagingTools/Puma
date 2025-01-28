TARGET = pumatest

include($(ACFDIR)/Config/QMake/ApplicationConfig.pri)
include($(IMTCOREDIR)/Config/QMake/ImtCore.pri)
include($(PUMADIR)/Config/QMake/Puma.pri)

RESOURCES += $$files($$_PRO_FILE_PWD_/../*.qrc, false)

LIBS += -L$(ACFDIR)/Lib/$$COMPILER_DIR -litest
LIBS += -L$(ACFSLNDIR)/Lib/$$COMPILER_DIR -litest -liauth -liservice -lifile -liser
LIBS += -L$(IMTCOREDIR)/Lib/$$COMPILER_DIR -limtbase -limtauth -limtcrypt -limtservergql -limtdb -limtgql -limtgqltest -limtauthgql -limtbasesdl -limtauthsdl -limtcol

QT += xml test sql widgets core
