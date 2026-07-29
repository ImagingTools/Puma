project(PumaClient LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../PumaClient.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST ON)

if(WIN32)
	set(CMAKE_WIN32_EXECUTABLE "on")
	set(ACF_CONVERT_FILES "${PROJECT_SOURCE_DIR}/../VC/PumaClient.rc.xtracf")
	set(ACF_CONVERT_REGISTRY "${PROJECT_SOURCE_DIR}/../VC/FileSubstitCopyApp.acc")
	set(ACF_CONVERT_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/BaseOnly.awc")
	set(RC_FILE "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/PumaClient.rc")
endif()

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)
include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)
include(${ACFDIR}/Config/CMake/WindeployQt.cmake)

target_link_libraries(${PROJECT_NAME} ${ACF_APPLICATION_LINK_SCOPE}
	AcfSln::iauth AcfSln::imeas AcfSln::iqtmeas AcfSln::iqtinsp AcfSln::iproc AcfSln::iinsp AcfSln::iipr AcfSln::iprocgui AcfSln::isig AcfSln::iqtsig AcfSln::icalibgui AcfSln::icalib AcfSln::icam AcfSln::iqtcam AcfSln::AcfSlnLoc Acf::AcfLoc AcfSln::iedge AcfSln::iedgegui ImtCore::imtrepo Acf::iqtgui Acf::iview Acf::iqtprm Acf::iwidgets Acf::iloggui Acf::iqtdoc Acf::ifilegui Acf::istd Acf::icomp Acf::iprm Acf::imod Acf::iser Acf::iqt Acf::ilog Acf::ibase AcfSln::icomm
	ImtCore::imtbase ImtCore::imtdesign ImtCore::imtdev ImtCore::imtgui ImtCore::imtlog ImtCore::imtloggui ImtCore::imtwidgets ImtCore::imtstyle ImtCore::imtguigql ImtCore::imtrest ImtCore::imt3d ImtCore::imtlic ImtCore::imtlicgui ImtCore::imt3dgui ImtCore::imt3dview ImtCore::ImtCoreLoc Acf::i2d Acf::iqt2d AcfSln::iedge
	ImtCore::imtlicgui ImtCore::imtauthgui ImtCore::imtclientgql ImtCore::imtgql ImtCore::imtbase ImtCore::imtauth ImtCore::imtauthgui ImtCore::imtauthgql ImtCore::imtcrypt ImtCore::imtlicgui ImtCore::imtlicgql ImtCore::imtapp ImtCore::imtcolorsdl ImtCore::imtdb ImtCore::imtauthdb
	ImtCore::imtqml ImtCore::imtdocguiqml ImtCore::imtauthguiqml ImtCore::imtlicguiqml ImtCore::imtcolguiqml ImtCore::imtguigqlqml ImtCore::imtguiqml ImtCore::imtcontrolsqml ImtCore::imtstylecontrolsqml ImtCore::imtcom ImtCore::imtauthsdl ImtCore::imtappsdl ImtCore::imtbasesdl ImtCore::imtcol ImtCore::imtzip
	Qt${QT_VERSION_MAJOR}::QuickWidgets
	Qt${QT_VERSION_MAJOR}::WebSockets
	Qt${QT_VERSION_MAJOR}::Sql
	AuthClientSdk AuthServerSdk
)

acf_use_qt_base_modules()
acf_use_qt_graphics_modules()

if(WIN32)
	list(APPEND listOptions "--qmldir=${IMTCOREDIR}/Qml")
	set(listFiles \"${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME}/${PROJECT_NAME}.exe\")

	windeploy(${PROJECT_NAME} "${listOptions}" "${listFiles}")
	unset(listOptions)
endif()

add_compile_definitions(UNICODE _UNICODE)

if(DEFINED WEB_COMPILE AND WEB_COMPILE)
	add_compile_definitions(WEB_COMPILE)
endif()

include(${IACFDIR}/Config/CMake/zlib.cmake)
if(UNIX)
	target_link_libraries(${PROJECT_NAME} ${ACF_APPLICATION_LINK_SCOPE} -lz)
endif()

