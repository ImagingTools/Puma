project(PumaServerSlTest LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")

if(WIN32)
#	set(CMAKE_WIN32_EXECUTABLE "on")
	set(ACF_CONVERT_FILES "${PROJECT_SOURCE_DIR}/../VC/PumaServerSlTest.rc.xtracf")
	set(ACF_CONVERT_REGISTRY "${PROJECT_SOURCE_DIR}/../VC/FileSubstitCopyApp.acc")
	set(ACF_CONVERT_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/BaseOnly.awc")
	set(RC_FILE "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/PumaServerSlTest.rc")
endif()

if(APPLE)
	set(CMAKE_MACOSX_BUNDLE ON)
endif()

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)

include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)
include(${IMTCOREDIR}/Config/CMake/OpenSSL.cmake)
include(${IMTCOREDIR}/Config/CMake/Quazip.cmake)
include(${ACFDIR}/Config/CMake/DeployQt.cmake)

if(WIN32)
	if(NOT USE_FIND_PACKAGE)
		file(COPY ${OPENSSLDIR}/lib/x64/libcrypto-1_1-x64.dll DESTINATION ${PROJECT_SOURCE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})
		file(COPY ${OPENSSLDIR}/lib/x64/libssl-1_1-x64.dll DESTINATION ${PROJECT_SOURCE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})
	endif()
endif()

target_link_libraries(PumaServerSlTest PRIVATE
	ImtCore::imtserverapp
	ImtCore::imtauthgql
	ImtCore::imtauthdb
	ImtCore::imtlog
	ImtCore::imtchat
	ImtCore::ImtCoreLoc
	Acf::iqtdoc
	AcfSln::AcfSlnLoc
	Acf::AcfLoc
)

include(${IACFDIR}/Config/CMake/zlib.cmake)

set(DEPLOY_QT_QML_DIRS "${IMTCOREDIR}/Qml")
deploy_qt(TARGET ${PROJECT_NAME})