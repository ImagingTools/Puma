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
include(${ACFDIR}/Config/CMake/WindeployQt.cmake)

target_link_libraries(${PROJECT_NAME} ${ACF_APPLICATION_LINK_SCOPE}
	ImtCore::imtauth ImtCore::imtmail ImtCore::imtlicdb ImtCore::imtlic ImtCore::imtzip ImtCore::imtcrypt ImtCore::imtrepo ImtCore::imtdb ImtCore::imtfile ImtCore::imtqml ImtCore::imtlog
	ImtCore::imtlicgql ImtCore::imtgql ImtCore::imtauthgql ImtCore::imtauthdb ImtCore::imtcom ImtCore::imtguigql ImtCore::imtgui ImtCore::imtclientgql ImtCore::imtservice ImtCore::imtauthsdl ImtCore::imtservergql ImtCore::imtcol
	ImtCore::imtserverapp ImtCore::imtbasesdl ImtCore::imtbase ImtCore::imtapp ImtCore::imtrest ImtCore::imtchat ImtCore::imtdoc
	iauth iservice
	Qt${QT_VERSION_MAJOR}::WebSockets
	Qt${QT_VERSION_MAJOR}::Sql
	Qt${QT_VERSION_MAJOR}::Qml
	Qt${QT_VERSION_MAJOR}::Widgets
)

acf_use_qt_base_modules()

if(WIN32)
	list(APPEND listOptions "--qmldir=${IMTCOREDIR}/Qml")
	set(files \"${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME}/${PROJECT_NAME}.exe\")

	windeploy(${PROJECT_NAME} "${listOptions}" "${files}")
	unset(listOptions)

	if(NOT USE_FIND_PACKAGE)
		file(COPY ${OPENSSLDIR}/lib/x64/libcrypto-1_1-x64.dll DESTINATION ${PROJECT_SOURCE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})
		file(COPY ${OPENSSLDIR}/lib/x64/libssl-1_1-x64.dll DESTINATION ${PROJECT_SOURCE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})
	endif()
endif()

include(${IACFDIR}/Config/CMake/zlib.cmake)
