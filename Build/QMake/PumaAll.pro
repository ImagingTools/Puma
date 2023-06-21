# Root of Acula project
TEMPLATE = subdirs

SUBDIRS += aculatask
aculatask.file = ../../Include/aculatask/QMake/aculatask.pro

SUBDIRS += aculainsp
aculainsp.file = ../../Include/aculainsp/QMake/aculainsp.pro

SUBDIRS += aculainspgui
aculainspgui.file = ../../Include/aculainspgui/QMake/aculainspgui.pro
#aculainspgui.depends = DesignTokenCreatorExe

SUBDIRS += aculataskgui
aculataskgui.file = ../../Include/aculataskgui/QMake/aculataskgui.pro

SUBDIRS += aculaipr
aculaipr.file = ../../Include/aculaipr/QMake/aculaipr.pro

SUBDIRS += aculaprod
aculaprod.file = ../../Include/aculaprod/QMake/aculaprod.pro

SUBDIRS += aculaprodgui
aculaprodgui.file = ../../Include/aculaprodgui/QMake/aculaprodgui.pro
aculaprodgui.depends = aculastatgui

SUBDIRS += acula3d
acula3d.file = ../../Include/acula3d/QMake/acula3d.pro

SUBDIRS += aculagql
aculagql.file = ../../Include/aculagql/QMake/aculagql.pro

SUBDIRS += aculaqml
aculaqml.file = ../../Include/aculaqml/QMake/aculaqml.pro

SUBDIRS += aculapcl
aculapcl.file = ../../Include/aculapcl/QMake/aculapcl.pro

SUBDIRS += aculacam
aculacam.file = ../../Include/aculacam/QMake/aculacam.pro

SUBDIRS += aculagui
aculagui.file = ../../Include/aculagui/QMake/aculagui.pro
#aculagui.depends = DesignTokenCreatorExe

SUBDIRS += aculars
aculars.file = ../../Include/aculars/QMake/aculars.pro

SUBDIRS += aculacv
aculacv.file = ../../Include/aculacv/QMake/aculacv.pro

SUBDIRS += aculastat
aculastat.file = ../../Include/aculastat/QMake/aculastat.pro

SUBDIRS += aculastatgui
aculastatgui.file = ../../Include/aculastatgui/QMake/aculastatgui.pro
#aculastatgui.depends = DesignTokenCreatorExe

SUBDIRS += aculagev
aculagev.file = ../../Include/aculagev/QMake/aculagev.pro

SUBDIRS += aculagenicam
aculagenicam.file = ../../Include/aculagenicam/QMake/aculagenicam.pro

# Component packages
SUBDIRS += AculaTaskPck
AculaTaskPck.file = ../../Impl/AculaTaskPck/QMake/AculaTaskPck.pro
AculaTaskPck.depends = aculatask

SUBDIRS += AculaTaskGuiPck
AculaTaskGuiPck.file = ../../Impl/AculaTaskGuiPck/QMake/AculaTaskGuiPck.pro
AculaTaskGuiPck.depends = aculataskgui aculatask

SUBDIRS += AculaInspectionPck
AculaInspectionPck.file = ../../Impl/AculaInspectionPck/QMake/AculaInspectionPck.pro
AculaInspectionPck.depends = aculainsp

SUBDIRS += AculaInspectionGuiPck
AculaInspectionGuiPck.file = ../../Impl/AculaInspectionGuiPck/QMake/AculaInspectionGuiPck.pro
AculaInspectionGuiPck.depends = aculainspgui aculainsp

SUBDIRS += AculaIprPck
AculaIprPck.file = ../../Impl/AculaIprPck/QMake/AculaIprPck.pro
AculaIprPck.depends = aculaipr

SUBDIRS += AculaProductionPck
AculaProductionPck.file = ../../Impl/AculaProductionPck/QMake/AculaProductionPck.pro
AculaProductionPck.depends = aculaprod

SUBDIRS += AculaProductionGuiPck
AculaProductionGuiPck.file = ../../Impl/AculaProductionGuiPck/QMake/AculaProductionGuiPck.pro
AculaProductionGuiPck.depends = aculaprodgui

SUBDIRS += AculaGqlPck
AculaGqlPck.file = ../../Impl/AculaGqlPck/QMake/AculaGqlPck.pro
AculaGqlPck.depends = aculagql

!linux{
	SUBDIRS += Acula3dPck
	Acula3dPck.file = ../../Impl/Acula3dPck/QMake/Acula3dPck.pro
	Acula3dPck.depends = acula3d

	SUBDIRS += AculaPclPck
	AculaPclPck.file = ../../Impl/AculaPclPck/QMake/AculaPclPck.pro
	AculaPclPck.depends = acula3d aculapcl
}

SUBDIRS += AculaCamPck
AculaCamPck.file = ../../Impl/AculaCamPck/QMake/AculaCamPck.pro
AculaCamPck.depends = aculacam

SUBDIRS += AculaGeniCamPck
AculaGeniCamPck.file = ../../Impl/AculaGeniCamPck/QMake/AculaGeniCamPck.pro
AculaGeniCamPck.depends = aculacam aculagenicam aculagev

!linux{
	SUBDIRS += AculaRealSensePck
	AculaRealSensePck.file = ../../Impl/AculaRealSensePck/QMake/AculaRealSensePck.pro
	AculaRealSensePck.depends = aculars
}

SUBDIRS += AculaOpenCvPck
AculaOpenCvPck.file = ../../Impl/AculaOpenCvPck/QMake/AculaOpenCvPck.pro
AculaOpenCvPck.depends = aculaipr aculacv

SUBDIRS += AculaGuiPck
AculaGuiPck.file = ../../Impl/AculaGuiPck/QMake/AculaGuiPck.pro
AculaGuiPck.depends = aculagui

SUBDIRS += AculaStatPck
AculaStatPck.file = ../../Impl/AculaStatPck/QMake/AculaStatPck.pro
AculaStatPck.depends = aculastat

SUBDIRS += AculaStatGuiPck
AculaStatGuiPck.file = ../../Impl/AculaStatGuiPck/QMake/AculaStatGuiPck.pro
AculaStatGuiPck.depends = aculastatgui

SUBDIRS += AculaLoc
AculaLoc.file = ../../Impl/AculaLoc/QMake/AculaLoc.pro

# Plug-ins
SUBDIRS += AculaSurfaceInspectionPlugin
AculaSurfaceInspectionPlugin.file = ../../Impl/Plugins/AculaSurfaceInspectionPlugin/QMake/AculaSurfaceInspectionPlugin.pro
AculaSurfaceInspectionPlugin.depends = AculaStatPck AculaGuiPck AculaProductionPck AculaInspectionPck AculaTaskPck

SUBDIRS += AculaSurfaceInspectionEditorPlugin
AculaSurfaceInspectionEditorPlugin.file = ../../Impl/Plugins/AculaSurfaceInspectionEditorPlugin/QMake/AculaSurfaceInspectionEditorPlugin.pro
AculaSurfaceInspectionEditorPlugin.depends = AculaStatGuiPck AculaStatPck AculaGuiPck AculaProductionPck AculaProductionGuiPck AculaInspectionPck AculaInspectionGuiPck AculaTaskPck AculaTaskGuiPck

# Application
SUBDIRS += Acula
Acula.file = ../../Impl/AculaExe/QMake/Acula.pro
Acula.depends = AculaSurfaceInspectionPlugin AculaSurfaceInspectionEditorPlugin AculaStatGuiPck AculaStatPck AculaGuiPck AculaProductionPck AculaProductionGuiPck AculaInspectionPck AculaInspectionGuiPck AculaTaskPck AculaTaskGuiPck

