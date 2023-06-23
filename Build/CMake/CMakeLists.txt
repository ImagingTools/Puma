cmake_minimum_required(VERSION 3.1.0)

project(PumaAll)

if(NOT DEFINED IMTCOREDIR)
	set(IMTCOREDIR "$ENV{IMTCOREDIR}")
endif()

include(${IMTCOREDIR}/Config/CMake/ImtCoreEnv.cmake)

set(INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Include")
set(IMPL_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Impl")
set(AUX_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../AuxInclude/${TARGETNAME}/GeneratedFiles")

include_directories("${AUX_INCLUDE_DIR}/../")

# Include
#add_subdirectory("${INCLUDE_DIR}/pumagql/CMake" "${AUX_INCLUDE_DIR}/pumagql")

# Impl
add_subdirectory("${IMPL_DIR}/PumaServer/CMake" "${AUX_INCLUDE_DIR}/PumaServer")
#add_subdirectory("${IMPL_DIR}/PumaGqlPck/CMake" "${AUX_INCLUDE_DIR}/PumaGqlPck")


file(COPY ../Migrations DESTINATION ${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})

