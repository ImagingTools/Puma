project(AuthClientSdk LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST OFF)

set(ACC_FILES
	"${PROJECT_SOURCE_DIR}/../AuthClientSdk.acc"
	"${PROJECT_SOURCE_DIR}/../AdministrationWidget.acc"
	"${PROJECT_SOURCE_DIR}/../LoginWidget.acc"
)

set(TARGET_EXT ".dll")
include(${ACFDIR}/Config/CMake/SharedLibraryConfig.cmake)

foreach(acc ${ACC_FILES})
	get_filename_component(acc_name "${acc}" NAME_WE)

	set(ARXC_FILES "${acc}")
	include(${ACFDIR}/Config/CMake/CustomBuild.cmake)

	target_sources(${PROJECT_NAME}
		PRIVATE
		"${AUX_INCLUDE_DIR}/${PROJECT_NAME}/C${acc_name}.cpp"
		"${AUX_INCLUDE_DIR}/${PROJECT_NAME}/C${acc_name}.h"
	)
endforeach()

include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)

target_link_libraries(${PROJECT_NAME} ${ACF_LIBRARY_LINK_SCOPE}
	AcfSln::AcfSlnLoc Acf::AcfLoc ImtCore::ImtCoreLoc
	AcfSln::imeas AcfSln::iproc Acf::iqtgui AcfSln::iauth Acf::icomp Acf::idoc Acf::i2d AcfSln::icomm
	ImtCore::imtbasesdl ImtCore::imtbase ImtCore::imtauthsdl ImtCore::imtgql ImtCore::imtcom ImtCore::imtservergql ImtCore::imtserverapp ImtCore::imtauthgql ImtCore::imtclientgql ImtCore::imtlic ImtCore::imtcol ImtCore::imtdb ImtCore::imtrest ImtCore::imtauth ImtCore::imtapp ImtCore::imtappsdl
	ImtCore::imtstyle ImtCore::imtcontrolsqml ImtCore::imtguiqml ImtCore::imtauthguiqml ImtCore::imtguigqlqml ImtCore::imtstylecontrolsqml ImtCore::imtlicguiqml ImtCore::imtdocguiqml ImtCore::imtcolguiqml ImtCore::imtlicgui ImtCore::imtgui ImtCore::imtcolorsdl ImtCore::imt2dsdl
	ImtCore::imtauthgui ImtCore::imtqml ImtCore::imtauthdb ImtCore::imtchatdb ImtCore::imtdeskdb
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Gui
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Network
	Qt${QT_VERSION_MAJOR}::WebSockets
	Qt${QT_VERSION_MAJOR}::Qml
	Qt${QT_VERSION_MAJOR}::QuickWidgets
)

add_compile_definitions(UNICODE _UNICODE)

