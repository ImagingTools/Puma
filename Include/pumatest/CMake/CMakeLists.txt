project(pumatest)

include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Sql Widgets Xml)

target_link_libraries(
    ${PROJECT_NAME}
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Xml
    Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(${PROJECT_NAME} itest ipackage imtbase imtauth imtdb imtgql imtauthgql ifile iser)

include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)
include(${ACFDIR}/Config/CMake/AcfStdGui.cmake)
include(${ACFDIR}/Config/CMake/WindeployQt.cmake)

if(WIN32)
	set(listFiles \"${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME}/${PROJECT_NAME}.exe\")
	windeploy(${PROJECT_NAME} "" "${listFiles}")
endif()
