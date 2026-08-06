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

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core)
find_package("Qt${QT_VERSION_MAJOR}" COMPONENTS Widgets Core Gui Network WebSockets QuickWidgets Qml REQUIRED)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
