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

# Compatibility mode: some CI agents can combine a modern Puma checkout with
# older Acf/ImtCore helper CMake files that still use plain
# target_link_libraries() calls. In that case, keyword signatures from
# GeneralConfig/OpenSSL conflict with plain signatures from AcfQt/AcfStd.
set(PUMA_FORCE_PLAIN_TLL "AUTO" CACHE STRING "Force plain target_link_libraries signatures: AUTO|ON|OFF")
set_property(CACHE PUMA_FORCE_PLAIN_TLL PROPERTY STRINGS "AUTO" "ON" "OFF")

set(_puma_force_plain_tll OFF)
if(PUMA_FORCE_PLAIN_TLL STREQUAL "ON")
	set(_puma_force_plain_tll ON)
elseif(PUMA_FORCE_PLAIN_TLL STREQUAL "AUTO")
	set(_acf_qt_cmake "${ACFDIR}/Config/CMake/AcfQt.cmake")
	if(EXISTS "${_acf_qt_cmake}")
		file(READ "${_acf_qt_cmake}" _acf_qt_cmake_text)
		string(FIND "${_acf_qt_cmake_text}" "ACF_LIBRARY_LINK_SCOPE" _acf_qt_uses_scope)
		if(_acf_qt_uses_scope EQUAL -1)
			set(_puma_force_plain_tll ON)
			message(WARNING "Puma: detected legacy AcfQt.cmake without ACF_LIBRARY_LINK_SCOPE. Falling back to plain target_link_libraries signatures for compatibility.")
		endif()
	endif()
endif()

if(_puma_force_plain_tll)
	set(ACF_QT_MODULE_LINK_SCOPE "" CACHE STRING "Link scope used for Qt module dependencies" FORCE)
	set(ACF_LIBRARY_LINK_SCOPE "" CACHE STRING "Link scope used by Puma inter-library dependencies" FORCE)
	set(ACF_PACKAGE_LINK_SCOPE "" CACHE STRING "Link scope used by package (Pck) libraries linking their dependencies" FORCE)
	set(ACF_APPLICATION_LINK_SCOPE "" CACHE STRING "Link scope used by executables linking their dependencies" FORCE)

	# Legacy Acf*.cmake modules link against short names like "iqtdoc".
	# In modern package mode, these libraries are exposed as namespaced imported
	# targets (Acf::iqtdoc, ImtCore::imtbase, ...). Create short-name aliases so
	# plain target_link_libraries() resolves to targets instead of *.lib filenames.
	function(puma_create_legacy_short_target_aliases)
		get_property(_all_targets GLOBAL PROPERTY TARGETS)
		foreach(_t IN LISTS _all_targets)
			if(_t MATCHES "^(Acf|AcfSln|IAcf|ImtCore)::(.+)$")
				set(_alias "${CMAKE_MATCH_2}")
				if(NOT TARGET ${_alias})
					get_target_property(_type ${_t} TYPE)
					if(_type STREQUAL "STATIC_LIBRARY" OR _type STREQUAL "SHARED_LIBRARY" OR _type STREQUAL "INTERFACE_LIBRARY")
						add_library(${_alias} ALIAS ${_t})
					endif()
				endif()
			endif()
		endforeach()
	endfunction()

	puma_create_legacy_short_target_aliases()
else()
	# TeamCity can reuse an old CMakeCache.txt where these cache entries were left
	# as empty strings (legacy plain target_link_libraries signature). Normalize
	# empty values to modern keyword defaults to keep signatures consistent.
	if("${ACF_QT_MODULE_LINK_SCOPE}" STREQUAL "")
		set(ACF_QT_MODULE_LINK_SCOPE "PRIVATE" CACHE STRING "Link scope used for Qt module dependencies" FORCE)
	endif()
	if("${ACF_LIBRARY_LINK_SCOPE}" STREQUAL "")
		set(ACF_LIBRARY_LINK_SCOPE "PUBLIC" CACHE STRING "Link scope used by Puma inter-library dependencies" FORCE)
	endif()
	if("${ACF_PACKAGE_LINK_SCOPE}" STREQUAL "")
		set(ACF_PACKAGE_LINK_SCOPE "PRIVATE" CACHE STRING "Link scope used by package (Pck) libraries linking their dependencies" FORCE)
	endif()
	if("${ACF_APPLICATION_LINK_SCOPE}" STREQUAL "")
		set(ACF_APPLICATION_LINK_SCOPE "PRIVATE" CACHE STRING "Link scope used by executables linking their dependencies" FORCE)
	endif()
endif()

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


