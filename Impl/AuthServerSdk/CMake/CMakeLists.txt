project(AuthServerSdk LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST ON)

set(TARGET_EXT ".dll")
include(${ACFDIR}/Config/CMake/SharedLibraryConfig.cmake)

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)

target_sources(${PROJECT_NAME}
	PRIVATE
	"${AUX_INCLUDE_DIR}/${PROJECT_NAME}/C${PROJECT_NAME}.cpp"
	"${AUX_INCLUDE_DIR}/${PROJECT_NAME}/C${PROJECT_NAME}.h"
)

include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

target_link_libraries(AuthServerSdk PUBLIC
	ImtCore::imtdeskgql
	ImtCore::imtauthgql
	ImtCore::imtchatgql
	ImtCore::imtauthdb
	ImtCore::imtchatdb
	ImtCore::imtdeskdb
	Qt${QT_VERSION_MAJOR}::Widgets
)

include(${IACFDIR}/Config/CMake/zlib.cmake)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
