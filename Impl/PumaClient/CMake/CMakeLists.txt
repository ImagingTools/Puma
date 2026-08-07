project(PumaClient LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../PumaClient.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST ON)

if(WIN32)
	set(CMAKE_WIN32_EXECUTABLE "on")
	set(ACF_CONVERT_FILES "${PROJECT_SOURCE_DIR}/../VC/PumaClient.rc.xtracf")
	set(ACF_CONVERT_REGISTRY "${PROJECT_SOURCE_DIR}/../VC/FileSubstitCopyApp.acc")
	set(ACF_CONVERT_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/BaseOnly.awc")
	set(RC_FILE "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/PumaClient.rc")
endif()

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)
include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)
include(${ACFDIR}/Config/CMake/AcfQt.cmake)
include(${ACFDIR}/Config/CMake/AcfStdGui.cmake)
include(${ACFDIR}/Config/CMake/WindeployQt.cmake)

if(WIN32)
	list(APPEND listOptions "--qmldir=${IMTCOREDIR}/Qml")
	set(listFiles \"${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME}/${PROJECT_NAME}.exe\")

	windeploy(${PROJECT_NAME} "${listOptions}" "${listFiles}")
	unset(listOptions)
endif()

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)

if(DEFINED WEB_COMPILE AND  WEB_COMPILE)
	add_compile_definitions(WEB_COMPILE)
endif()

target_link_libraries(PumaClient
	AuthClientSdk 
	AuthServerSdk
	ImtCore::imt3dview 
	ImtCore::imtrepo
)

include(${IACFDIR}/Config/CMake/zlib.cmake)

if(UNIX)
	target_link_libraries(PumaClient ${ACF_APPLICATION_LINK_SCOPE} -lz)
endif()
