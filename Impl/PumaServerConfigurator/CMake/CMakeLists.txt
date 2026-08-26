project(PumaServerConfigurator LANGUAGES CXX)


set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")

if(WIN32)
	set(CMAKE_WIN32_EXECUTABLE "on")
	set(ACF_CONVERT_FILES "${PROJECT_SOURCE_DIR}/../VC/PumaServerConfigurator.rc.xtracf")
	set(ACF_CONVERT_REGISTRY "${PROJECT_SOURCE_DIR}/../VC/FileSubstitCopyApp.acc")
	set(ACF_CONVERT_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/BaseOnly.awc")
	set(RC_FILE "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/PumaServerConfigurator.rc")
endif()

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)
include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)

include(${IMTCOREDIR}/Config/CMake/OpenSSL.cmake)
include(${IMTCOREDIR}/Config/CMake/Quazip.cmake)
include(${ACFDIR}/Config/CMake/DeployQt.cmake)

target_link_libraries(PumaServerConfigurator PRIVATE
	pumaqml
	ImtCore::imtcontrolsqml
	ImtCore::imtguigqlqml
	ImtCore::imtstylecontrolsqml
	ImtCore::imtguiqml
	ImtCore::imtserverapp
	ImtCore::ImtCoreLoc
	AcfSln::AcfSlnLoc
	Acf::AcfLoc
)

deploy_qt(TARGET ${PROJECT_NAME})

