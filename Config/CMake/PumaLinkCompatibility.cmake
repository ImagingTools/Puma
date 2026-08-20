# Puma link compatibility helpers for modern target-based CMake.
#
# Public entrypoint:
#   puma_apply_link_compatibility()
#
# Expected preconditions:
#   - ACF_MODERN_CMAKE is ON and ImtCoreEnv.cmake has been included.
#   - Environment variables/derived vars from ImtCoreEnv.cmake are available.



# Discover the ImtCore package published by its build tree. ImtCore transitively pulls in the
# Acf, AcfSln and IAcf packages, so the Acf::/AcfSln::/IAcf::/ImtCore:: imported targets
# referenced by the Puma dependency graph resolve.
if(ACF_MODERN_CMAKE AND NOT TARGET ImtCore::imtbase)
	set(ImtCore_DIR "${IMTCOREDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}/cmake" CACHE PATH "Path to the ImtCore build-tree CMake package")
	find_package(ImtCore REQUIRED GLOBAL)
endif()

function(puma_apply_link_compatibility)
	# Link-scope defaults are initialized centrally in ImtCoreEnv.cmake.
endfunction()
