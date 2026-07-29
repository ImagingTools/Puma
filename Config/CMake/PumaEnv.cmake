cmake_minimum_required(VERSION 3.26)

if(NOT DEFINED IMTCOREDIR)
	set(IMTCOREDIR "$ENV{IMTCOREDIR}")
endif()

include(${IMTCOREDIR}/Config/CMake/ImtCoreEnv.cmake)

if(NOT DEFINED PUMADIR)
	set(PUMADIR "$ENV{PUMADIR}")
endif()

if(PUMADIR STREQUAL "")
	set(PUMADIR ${IMTCOREDIR}/../Puma)
endif()

if(NOT DEFINED PUMA_DIR)
	set(PUMA_DIR "${PUMADIR}")
endif()

if(NOT DEFINED PUMA_BUILD_DIR)
	if(DEFINED ENV{PUMA_BUILD_DIR})
		set(PUMA_BUILD_DIR "$ENV{PUMA_BUILD_DIR}")
	elseif(DEFINED BUILDDIR)
		set(PUMA_BUILD_DIR "${BUILDDIR}/Puma")
	else()
		set(PUMA_BUILD_DIR "${PUMA_DIR}")
	endif()
endif()

file(TO_CMAKE_PATH "${PUMA_BUILD_DIR}" PUMA_BUILD_DIR)

message(STATUS "PUMA_DIR: ${PUMA_DIR}")
message(STATUS "PUMA_BUILD_DIR: ${PUMA_BUILD_DIR}")

include_directories("${PUMA_BUILD_DIR}/AuxInclude/${TARGETNAME}/GeneratedFiles")
include_directories("${PUMA_BUILD_DIR}/AuxInclude/${TARGETNAME}")

if(NOT ACF_MODERN_CMAKE)
	include_directories("${PUMA_DIR}/Include")
	include_directories("${PUMA_DIR}/Impl")
	link_directories("${PUMA_BUILD_DIR}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")
endif()

