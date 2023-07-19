TARGET = pumaqml

include($(ACFDIR)/Config/QMake/StaticConfig.pri)
include($(IMTCOREDIR)/Config/QMake/ImtCore.pri)

RESOURCES += $$files($$_PRO_FILE_PWD_/../*.qrc, false)
