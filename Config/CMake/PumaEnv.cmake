cmake_minimum_required(VERSION 3.26)


if(NOT DEFINED IMTCOREDIR)
	file(TO_CMAKE_PATH "$ENV{IMTCOREDIR}" IMTCOREDIR)
	if(IMTCOREDIR STREQUAL "")
		set(IMTCOREDIR ${PUMA_DIR}/../ImtCore)
	endif()
endif()

include(${IMTCOREDIR}/Config/CMake/ImtCoreEnv.cmake)

if(NOT DEFINED PUMADIR)
	file(TO_CMAKE_PATH "$ENV{PUMADIR}" PUMADIR)
	if(PUMADIR STREQUAL "")
		set(PUMADIR ${IMTCOREDIR}/../Puma)
	endif()
endif()

if(NOT DEFINED PUMA_DIR)
	set(PUMA_DIR "${PUMADIR}")
endif()

if(NOT DEFINED IMTCOREDIR_BUILD)
	file(TO_CMAKE_PATH "$ENV{IMTCOREDIR_BUILD}" IMTCOREDIR_BUILD)
	if(IMTCOREDIR_BUILD STREQUAL "")
		set(IMTCOREDIR_BUILD ${IMTCOREDIR})
	endif()
endif()

if(NOT DEFINED PUMA_BUILD_DIR)
	file(TO_CMAKE_PATH "$ENV{PUMA_BUILD_DIR}" PUMA_BUILD_DIR)
	if(DEFINED PUMA_DIR AND PUMA_BUILD_DIR STREQUAL "")
		set(PUMA_BUILD_DIR ${PUMA_DIR})
	endif()
endif()

message(STATUS "PUMA_DIR: ${PUMA_DIR}")
message(STATUS "PUMA_BUILD_DIR: ${PUMA_BUILD_DIR}")

include_directories("${PUMA_BUILD_DIR}/AuxInclude/${TARGETNAME}/GeneratedFiles")
include_directories("${PUMA_BUILD_DIR}/AuxInclude/${TARGETNAME}")

if(NOT ACF_MODERN_CMAKE)
	# Legacy mode: global include/link dirs for repos that haven't migrated to
	# find_package(ImtCore) + target-based deps yet. Skipped when ACF_MODERN_CMAKE is ON.
	include_directories("${PUMA_DIR}/Include")
	include_directories("${PUMA_DIR}/Impl")

	link_directories("${PUMA_BUILD_DIR}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")
elseif(NOT TARGET ImtCore::imtbase)
	# Discover the ImtCore package published by its build tree. ImtCore transitively pulls in the
	# Acf, AcfSln, IAcf (optional) packages, so the Acf::/AcfSln::/IAcf::/ImtCore::
	# imported targets referenced by the Puma dependency graph resolve.
	set(ImtCore_DIR "${IMTCOREDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}/cmake" CACHE PATH "Path to the ImtCore build-tree CMake package")
	message(STATUS "ImtCore_DIR: ${ImtCore_DIR}")
	find_package(ImtCore REQUIRED GLOBAL)
endif()

