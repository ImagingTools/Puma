TARGET = PumaSettingsPlugin

include(../../../../Config/QMake/AculaPlugin.pri)
include($(IACFDIR)/Config/QMake/OpenCV_4_5_3.pri)
include($(IMTCOREDIR)/Config/QMake/ImtCore.pri)

QT += sql

DESTDIR = $$OUT_PWD/../../../../Bin/$$COMPILER_DIR/Plugins
OBJECTS_DIR = ../$$AUXINCLUDEPATH/GeneratedFiles/$$CONFIGURATION_NAME/"$$TARGET"

LIBS += -L../../../Lib/$$COMPILER_DIR -limeas -liproc -liqtmeas -liipr -liinsp -liqtinsp -liqtipr -licam -liqtcam -liqtmm -liprocgui -licalib -liocv -liedge -liedgegui -lAcfSlnLoc -lAcfLoc

LIBS += -limtbase -limtdb
LIBS += -laculatask -laculainsp -laculaipr -laculaprod

# Set configuration of custom builds:
# ARX Compiler:
ARXC_FILES += $$PWD/../PumaSettingsPlugin.acc
ARXC_CONFIG = $$PWD/../../../../Config/Puma.awc
ARXC_OUTDIR = $$OUT_PWD/$$AUXINCLUDEPATH/GeneratedFiles/$$TARGET

include($(ACFDIR)/Config/QMake/AcfQt.pri)
include($(ACFDIR)/Config/QMake/AcfStd.pri)
include($(ACFDIR)/Config/QMake/CustomBuild.pri)

