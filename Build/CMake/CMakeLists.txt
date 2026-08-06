cmake_minimum_required(VERSION 3.26)

project(PumaAll)

if(NOT DEFINED QT_VERSION_MAJOR)
	find_package(QT NAMES Qt6 Qt5 COMPONENTS Core)
	find_package("Qt${QT_VERSION_MAJOR}" COMPONENTS Core Widgets QuickWidgets Core Gui Xml Network Svg Sql WebSockets Qml Concurrent REQUIRED)
	if(QT_VERSION_MAJOR EQUAL 6)
		find_package("Qt${QT_VERSION_MAJOR}" COMPONENTS Core5Compat REQUIRED)
	endif()
endif()

if(NOT DEFINED IMTCOREDIR)
	set(IMTCOREDIR "$ENV{IMTCOREDIR}")
endif()

if(NOT DEFINED BUILDDIR)
	set(BUILDDIR "${CMAKE_CURRENT_LIST_DIR}/../../..")
endif()

# Enable modern target-based architecture:
# find_package(ImtCore) provides ImtCore:: imported targets (and, transitively through it,
# Acf::/AcfSln::/IAcf::) with transitive include dirs and link deps. Legacy global
# include_directories()/link_directories() in the Env chain are skipped.
set(ACF_MODERN_CMAKE ON)

# Collect Puma libraries in their own export set. Read by
# StaticConfig.cmake / acf_register_library() (from Acf).
set(ACF_PACKAGE_NAME "Puma")

include(${IMTCOREDIR}/Config/CMake/ImtCoreEnv.cmake)

message("Puma IMTCOREDIR: ${IMTCOREDIR} IMTCOREDIR_BUILD ${IMTCOREDIR_BUILD}")

# Discover the ImtCore package published by its build tree. ImtCore transitively pulls in the
# Acf, AcfSln and IAcf packages, so the Acf::/AcfSln::/IAcf::/ImtCore:: imported targets
# referenced by the Puma dependency graph resolve.
if(ACF_MODERN_CMAKE AND NOT TARGET ImtCore::imtbase)
	set(ImtCore_DIR "${IMTCOREDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}/cmake" CACHE PATH "Path to the ImtCore build-tree CMake package")
	find_package(ImtCore REQUIRED GLOBAL)
endif()

set(INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Include")
set(IMPL_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Impl")

set(AUX_INCLUDE_DIR "${BUILDDIR}/Puma/AuxInclude/${TARGETNAME}/GeneratedFiles")

include_directories("${AUX_INCLUDE_DIR}/../")
include_directories("${AUX_INCLUDE_DIR}/")
include_directories("${IMTCOREDIR}/Include")

# Link scopes for the modern keyword target_link_libraries() signature. Inter-library
# dependencies are declared centrally in PumaLibraryDependencies.cmake; the per-library
# CMake files no longer carry them.
acf_define_link_scope_var(ACF_QT_MODULE_LINK_SCOPE  "PRIVATE" "Link scope used for Qt module dependencies")
acf_define_link_scope_var(ACF_LIBRARY_LINK_SCOPE     "PUBLIC"  "Link scope used by Puma inter-library dependencies")
acf_define_link_scope_var(ACF_PACKAGE_LINK_SCOPE     "PRIVATE" "Link scope used by package (Pck) libraries linking their dependencies")
acf_define_link_scope_var(ACF_APPLICATION_LINK_SCOPE "PRIVATE" "Link scope used by executables linking their dependencies")

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
include("${CMAKE_CURRENT_LIST_DIR}/../../Config/CMake/PumaLibraryDependencies.cmake")


