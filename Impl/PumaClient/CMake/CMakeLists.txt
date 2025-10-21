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
include(${IACFDIR}/Config/CMake/Qwt.cmake)
include(${ACFDIR}/Config/CMake/AcfStdGui.cmake)
include(${ACFDIR}/Config/CMake/WindeployQt.cmake)

target_link_libraries(${PROJECT_NAME} iauth imeas iqtmeas iqtinsp iproc iinsp iipr iprocgui isig iqtsig icalibgui icalib icam iqtcam AcfSlnLoc AcfLoc iedge iedgegui imtrepo iqtgui iview iqtprm iwidgets iloggui iqtdoc ifilegui)
target_link_libraries(${PROJECT_NAME} imtbase imtdesign imtdev imtgui imtdb imtdbgui imtlog imtloggui imtwidgets imtstyle imtguigql imtrest imt3d imtlic imtlicgui imt3dgui imt3dview ImtCoreLoc i2d iqt2d iedge)
target_link_libraries(${PROJECT_NAME} imtlicgui imtauthgui imtclientgql imtgql imtbase imtauth imtauthgui imtauthdb imtauthgql imtcrypt imtlicgui imtlicgql imtapp imtcolorsdl)
target_link_libraries(${PROJECT_NAME} imtqml imtdocguiqml imtauthguiqml imtlicguiqml imtcolguiqml imtguigqlqml imtguiqml imtcontrolsqml imtstylecontrolsqml imtcom imtauthsdl imtappsdl imtservergql imtbasesdl imtcol imtzip)
target_link_libraries(${PROJECT_NAME}  Qt${QT_VERSION_MAJOR}::QuickWidgets  Qt${QT_VERSION_MAJOR}::WebSockets)

# target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/../3rdParty/AuthClientSdk/include")
# target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/../3rdParty/AuthServerSdk/include")

# target_link_libraries(PumaClient ${PROJECT_SOURCE_DIR}/../3rdParty/AuthClientSdk/lib/AuthClientSdk.lib)
# target_link_libraries(PumaClient ${PROJECT_SOURCE_DIR}/../3rdParty/AuthServerSdk/lib/AuthServerSdk.lib)

if(QT_VERSION_MAJOR EQUAL 6)
	# target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

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
