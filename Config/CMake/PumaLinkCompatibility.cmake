# Puma link compatibility helpers for modern target-based CMake.
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

function(puma_apply_link_compatibility)
	# Enforce modern keyword signatures and package-bound targets.
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
endfunction()
