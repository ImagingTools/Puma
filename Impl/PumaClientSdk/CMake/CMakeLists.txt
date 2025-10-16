project(PumaClientSdk LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST OFF)

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)

set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../AdministrationWidget.acc")
include(${ACFDIR}/Config/CMake/CustomBuild.cmake)

set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../LoginWidget.acc")
include(${ACFDIR}/Config/CMake/CustomBuild.cmake)

set(TARGET_EXT ".plugin")

include(${ACFDIR}/Config/CMake/SharedLibraryConfig.cmake)

include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)

file(GLOB SOURCES
    "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/*.cpp"
    "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/*.h"
)

target_sources(PumaClientSdk PRIVATE ${SOURCES})

target_link_libraries(${PROJECT_NAME} imeas iproc iqtgui iauth icomp idoc i2d icomm)
target_link_libraries(${PROJECT_NAME} imtbase imtbasesdl imtauthsdl imtgql imtcom imtservergql imtserverapp imtauthgql imtclientgql imtlic imtcol)
target_link_libraries(${PROJECT_NAME} imtauthgui imtqml)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)


