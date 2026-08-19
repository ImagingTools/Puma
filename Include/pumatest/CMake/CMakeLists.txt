project(pumatest)

include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Sql Widgets Xml Core)

include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)
include(${ACFDIR}/Config/CMake/WindeployQt.cmake)

if(WIN32)
	set(listFiles \"${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME}/${PROJECT_NAME}.exe\")
	windeploy(${PROJECT_NAME} "" "${listFiles}")
endif()

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
