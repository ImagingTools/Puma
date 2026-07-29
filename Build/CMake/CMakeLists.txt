cmake_minimum_required(VERSION 3.26)

project(Puma)


option(BUILD_TESTING "Build test targets (pumatest)" ON)
option(WEB_COMPILE "Build with web compatibility" ON)


if(NOT DEFINED QT_VERSION_MAJOR)
	find_package(QT NAMES Qt6 Qt5 COMPONENTS Core)
	find_package("Qt${QT_VERSION_MAJOR}" COMPONENTS Core Concurrent Gui Network Qml QuickWidgets Sql Svg WebSockets Widgets Xml REQUIRED)
	if(QT_VERSION_MAJOR EQUAL 6)
		find_package("Qt${QT_VERSION_MAJOR}" COMPONENTS Core5Compat REQUIRED)
	endif()
endif()

get_filename_component(PUMA_DIR "../.." ABSOLUTE BASE_DIR ${CMAKE_CURRENT_LIST_DIR})

# Enable modern target-based architecture:
# find_package(ImtCore) provides ImtCore:: imported targets (and, transitively through it,
# Acf::/AcfSln::/IAcf::) with transitive include dirs and link deps. Legacy global
# include_directories()/link_directories() in the Env chain are skipped.
set(ACF_MODERN_CMAKE ON)

# Collect Puma libraries in their own export set. Read by
# StaticConfig.cmake / acf_register_library() (from Acf).
set(ACF_PACKAGE_NAME "Puma")

include(${PUMA_DIR}/Config/CMake/PumaEnv.cmake)

acf_define_link_scope_var(ACF_QT_MODULE_LINK_SCOPE "PRIVATE" "Link scope used for Qt module dependencies")
acf_define_link_scope_var(ACF_LIBRARY_LINK_SCOPE "PUBLIC" "Link scope used by Puma inter-library dependencies")
acf_define_link_scope_var(ACF_PACKAGE_LINK_SCOPE "PRIVATE" "Link scope used by package (Pck) libraries linking their dependencies")
acf_define_link_scope_var(ACF_APPLICATION_LINK_SCOPE "PRIVATE" "Link scope used by executables linking their dependencies")

set(INCLUDE_DIR		"${PUMA_DIR}/Include")
set(IMPL_DIR		"${PUMA_DIR}/Impl")
set(AUX_INCLUDE_DIR "${PUMA_BUILD_DIR}/AuxInclude/${TARGETNAME}/GeneratedFiles")

# Include
add_subdirectory("${INCLUDE_DIR}/pumaqml/CMake" "${AUX_INCLUDE_DIR}/pumaqml")

# Plug-ins
add_subdirectory("${IMPL_DIR}/Plugins/PumaSettingsPlugin/CMake" "${AUX_INCLUDE_DIR}/PumaSettingsPlugin")

# Impl
add_subdirectory("${IMPL_DIR}/PumaServerPg/CMake" "${AUX_INCLUDE_DIR}/PumaServerPg")
imt_chain_windeploy_target(PumaServerPg)
add_subdirectory("${IMPL_DIR}/PumaServerSl/CMake" "${AUX_INCLUDE_DIR}/PumaServerSl")
imt_chain_windeploy_target(PumaServerSl)
add_subdirectory("${IMPL_DIR}/PumaServerPgTest/CMake" "${AUX_INCLUDE_DIR}/PumaServerPgTest")
imt_chain_windeploy_target(PumaServerPgTest)
add_subdirectory("${IMPL_DIR}/PumaServerSlTest/CMake" "${AUX_INCLUDE_DIR}/PumaServerSlTest")
imt_chain_windeploy_target(PumaServerSlTest)
add_subdirectory("${IMPL_DIR}/PumaServerConfigurator/CMake" "${AUX_INCLUDE_DIR}/PumaServerConfigurator")
imt_chain_windeploy_target(PumaServerConfigurator)

if(WIN32)
	add_subdirectory("${IMPL_DIR}/PumaClient/CMake" "${AUX_INCLUDE_DIR}/PumaClient")
	imt_chain_windeploy_target(PumaClient)
	add_subdirectory("${IMPL_DIR}/AuthClientSdk/CMake" "${AUX_INCLUDE_DIR}/AuthClientSdk")
	add_subdirectory("${IMPL_DIR}/AuthServerSdk/CMake" "${AUX_INCLUDE_DIR}/AuthServerSdk")

	# Tests
	if (BUILD_TESTING)
		add_subdirectory("${INCLUDE_DIR}/pumatest/CMake" "${AUX_INCLUDE_DIR}/pumatest")
	endif()
endif()

# Declare target-based deps so include paths/link order propagate transitively across Puma
# libraries and onto the underlying ImtCore::/Acf::/AcfSln::/IAcf:: libraries.
include("${PUMA_DIR}/Config/CMake/PumaLibraryDependencies.cmake")
