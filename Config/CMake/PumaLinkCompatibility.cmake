# Puma compatibility helpers for mixed modern/legacy ACF CMake stacks.
#
# Public entrypoint:
#   puma_apply_link_compatibility()
#
# Expected preconditions:
#   - acf_define_link_scope_var(...) has already initialized link-scope vars.
#   - Environment variables/derived vars from ImtCoreEnv.cmake are available.



# Discover the ImtCore package published by its build tree. ImtCore transitively pulls in the
# Acf, AcfSln and IAcf packages, so the Acf::/AcfSln::/IAcf::/ImtCore:: imported targets
# referenced by the Puma dependency graph resolve.
if(ACF_MODERN_CMAKE AND NOT TARGET ImtCore::imtbase)
	set(ImtCore_DIR "${IMTCOREDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}/cmake" CACHE PATH "Path to the ImtCore build-tree CMake package")
	find_package(ImtCore REQUIRED GLOBAL)
endif()

# Link scopes for the modern keyword target_link_libraries() signature. Inter-library
# dependencies are declared centrally in PumaLibraryDependencies.cmake; the per-library
# CMake files no longer carry them.
acf_define_link_scope_var(ACF_QT_MODULE_LINK_SCOPE  "PRIVATE" "Link scope used for Qt module dependencies")
acf_define_link_scope_var(ACF_LIBRARY_LINK_SCOPE     "PUBLIC"  "Link scope used by Puma inter-library dependencies")
acf_define_link_scope_var(ACF_PACKAGE_LINK_SCOPE     "PRIVATE" "Link scope used by package (Pck) libraries linking their dependencies")
acf_define_link_scope_var(ACF_APPLICATION_LINK_SCOPE "PRIVATE" "Link scope used by executables linking their dependencies")

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

function(puma_apply_link_compatibility)
	# Compatibility mode: some CI agents can combine a modern Puma checkout with
	# older Acf/ImtCore helper CMake files that still use plain
	# target_link_libraries() calls.
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

		# In ACF_MODERN_CMAKE mode global link_directories() are disabled, but
		# legacy Acf*.cmake modules still add plain names like "iqtdoc" that
		# require these directories to resolve to concrete .lib files.
		if(DEFINED ACFDIR_BUILD AND NOT ACFDIR_BUILD STREQUAL "")
			link_directories("${ACFDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")
		endif()
		if(DEFINED ACFSLNDIR_BUILD AND NOT ACFSLNDIR_BUILD STREQUAL "")
			link_directories("${ACFSLNDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")
		endif()
		if(DEFINED IACFDIR_BUILD AND NOT IACFDIR_BUILD STREQUAL "")
			link_directories("${IACFDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")
		endif()
		if(DEFINED IMTCOREDIR_BUILD AND NOT IMTCOREDIR_BUILD STREQUAL "")
			link_directories("${IMTCOREDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")
		endif()

		# Legacy Acf*.cmake modules link against short names like "iqtdoc".
		# Create short-name aliases so plain target_link_libraries() resolves to
		# targets instead of *.lib filenames.
		puma_create_legacy_short_target_aliases()
	else()
		# TeamCity can reuse an old CMakeCache.txt where these cache entries were
		# left as empty strings. Normalize to modern keyword defaults.
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
endfunction()
