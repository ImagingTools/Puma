project(PumaSettingsPlugin LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST ON)

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)

set(TARGET_EXT ".plugin")

include(${ACFDIR}/Config/CMake/SharedLibraryConfig.cmake)
include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)

target_link_libraries(${PROJECT_NAME} ${ACF_PACKAGE_LINK_SCOPE}
	i2d ImtCore::imtbasesdl ImtCore::imtbase ImtCore::imtapp ImtCore::imtdb ImtCore::imtfile ImtCore::imtservice ImtCore::imtauth ImtCore::imtrest ImtCore::imtcol ImtCore::imtcom ImtCore::imtgql ImtCore::imtserverapp
	ifile istd idoc
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Sql
	Qt${QT_VERSION_MAJOR}::Xml
)

