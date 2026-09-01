cmake_minimum_required(VERSION 3.26)

if(NOT DEFINED PUMADIR)
	get_filename_component(PUMADIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
endif()

if(NOT DEFINED IMTCOREDIR)
	file(TO_CMAKE_PATH "$ENV{IMTCOREDIR}" IMTCOREDIR)
	if(IMTCOREDIR STREQUAL "")
		set(IMTCOREDIR "${PUMADIR}/../ImtCore")
	endif()
endif()

include("${IMTCOREDIR}/Config/CMake/ImtCoreEnv.cmake")

if(NOT DEFINED PUMADIR_BUILD)
	file(TO_CMAKE_PATH "$ENV{PUMADIR_BUILD}" PUMADIR_BUILD)
	if(PUMADIR_BUILD STREQUAL "")
		set(PUMADIR_BUILD "${PUMADIR}")
	endif()
endif()

include_directories("${PUMADIR_BUILD}/AuxInclude/${TARGETNAME}/GeneratedFiles")
include_directories("${PUMADIR_BUILD}/AuxInclude/${TARGETNAME}")

if(NOT ACF_MODERN_CMAKE)
	include_directories("${PUMADIR}/Include")
	include_directories("${PUMADIR}/Impl")

	link_directories("${PUMADIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}")
elseif(NOT TARGET ImtCore::imtbase)
	set(ImtCore_DIR "${IMTCOREDIR_BUILD}/Lib/${CMAKE_BUILD_TYPE}_${TARGETNAME}/cmake")
	message(STATUS "ImtCore_DIR (find_package): ${ImtCore_DIR}")

	find_package(ImtCore REQUIRED GLOBAL)
endif()
