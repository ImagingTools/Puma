project(pumatest)


include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Sql Widgets Xml Core)

include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)
include(${ACFDIR}/Config/CMake/DeployQt.cmake)

deploy_qt(TARGET ${PROJECT_NAME})

