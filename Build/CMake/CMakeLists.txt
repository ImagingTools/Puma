cmake_minimum_required(VERSION 3.1.0)

project(PumaAll)

if(NOT DEFINED QT_VERSION_MAJOR)
	find_package(QT NAMES Qt6 Qt5 COMPONENTS Core)
	find_package("Qt${QT_VERSION_MAJOR}" COMPONENTS Core Widgets Core Gui Xml Network Svg Concurrent REQUIRED)
endif()

if(NOT DEFINED IMTCOREDIR)
	set(IMTCOREDIR "$ENV{IMTCOREDIR}")
endif()

include(${IMTCOREDIR}/Config/CMake/ImtCoreEnv.cmake)

set(INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Include")
set(IMPL_DIR "${CMAKE_CURRENT_LIST_DIR}/../../Impl")
set(AUX_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../AuxInclude/${TARGETNAME}/GeneratedFiles")

include_directories("${AUX_INCLUDE_DIR}/../")

# Include
add_subdirectory("${INCLUDE_DIR}/pumaqml/CMake" "${AUX_INCLUDE_DIR}/pumaqml")

# Plug-ins
add_subdirectory("${IMPL_DIR}/Plugins/PumaSettingsPlugin/CMake" "${AUX_INCLUDE_DIR}/PumaSettingsPlugin")

# Impl
add_subdirectory("${IMPL_DIR}/PumaServer/CMake" "${AUX_INCLUDE_DIR}/PumaServer")
add_subdirectory("${IMPL_DIR}/PumaServerConfigurator/CMake" "${AUX_INCLUDE_DIR}/PumaServerConfigurator")

file(COPY ../Migrations DESTINATION ${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})
file(COPY ${IMTCOREDIR}/Install/Database/LogMigrations DESTINATION ${PROJECT_SOURCE_DIR}/../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})

