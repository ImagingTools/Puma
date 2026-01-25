project(PumaServer LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")

if(WIN32)
#	set(CMAKE_WIN32_EXECUTABLE "on")
	set(ACF_CONVERT_FILES "${PROJECT_SOURCE_DIR}/../VC/PumaServer.rc.xtracf")
	set(ACF_CONVERT_REGISTRY "${PROJECT_SOURCE_DIR}/../VC/FileSubstitCopyApp.acc")
	set(ACF_CONVERT_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/BaseOnly.awc")
	set(RC_FILE "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/PumaServer.rc")
endif()

if(APPLE)
	set(CMAKE_MACOSX_BUNDLE ON)
endif()

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)

include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)
include(${ACFDIR}/Config/CMake/AcfQt.cmake)
include(${IMTCOREDIR}/Config/CMake/OpenSSL.cmake)
include(${IMTCOREDIR}/Config/CMake/Quazip.cmake)
include(${ACFDIR}/Config/CMake/WindeployQt.cmake)

target_link_libraries(${PROJECT_NAME} imtauth imtmail imtlicdb imtlic imtzip imtcrypt imtrepo imtdb imtfile imtqml imtlog)
target_link_libraries(${PROJECT_NAME} imtlicgql imtgql imtauthgql imtauthdb imtcom imtguigql imtgui imtclientgql imtservice imtauthsdl imtservergql imtcol imtbasesdl)
target_link_libraries(${PROJECT_NAME} imtbase imtbasesdl imtserverapp imtapp imtrest)
target_link_libraries(${PROJECT_NAME} iauth iservice)
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::WebSockets)
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Sql)
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Qml)
target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets)

include(${ACFDIR}/Config/CMake/AcfStdGui.cmake)
include(${ACFDIR}/Config/CMake/AcfStd.cmake)

if(WIN32)
	list(APPEND listOptions "--qmldir=${IMTCOREDIR}/Qml")
	set(files \"${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME}/${PROJECT_NAME}.exe\")

	windeploy(${PROJECT_NAME} "${listOptions}" "${files}")
	unset(listOptions)

	file(COPY ${OPENSSLDIR}/lib/x64/libcrypto-1_1-x64.dll DESTINATION ${PROJECT_SOURCE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})
	file(COPY ${OPENSSLDIR}/lib/x64/libssl-1_1-x64.dll DESTINATION ${PROJECT_SOURCE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})
endif()

# for OSX if make bundle, copy migrations into a bundle
if (APPLE AND CMAKE_MACOSX_BUNDLE)
	get_property(OUTPUT_DIRECTORY
		TARGET ${PROJECT_NAME}
		PROPERTY RUNTIME_OUTPUT_DIRECTORY)

	get_filename_component(OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}" ABSOLUTE)
	set(BUNDLE_MIGRATIONS_PATH "${OUTPUT_DIRECTORY}/${PROJECT_NAME}.app/Contents/MacOS")

	file(MAKE_DIRECTORY "${BUNDLE_MIGRATIONS_PATH}")

	message(VERBOSE "copying migrations inside OSX bundle. Path: '${BUNDLE_MIGRATIONS_PATH}' target: '${PROJECT_NAME}'")

	file(COPY "${CMAKE_CURRENT_LIST_DIR}/../../../Build/Migrations" DESTINATION "${BUNDLE_MIGRATIONS_PATH}")
endif()

include(${IACFDIR}/Config/CMake/zlib.cmake)
