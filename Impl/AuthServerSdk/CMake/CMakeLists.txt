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

target_link_libraries(${PROJECT_NAME} ${ACF_LIBRARY_LINK_SCOPE}
	iauth iser imeas iqtmeas iqtinsp iproc iinsp iipr iprocgui isig iqtsig icalibgui icalib icam iqtcam AcfSlnLoc AcfLoc iedge iedgegui imtrepo iqtgui iview iqtprm iwidgets iloggui iqtdoc ifilegui icomm idoc
	imtbase imtdesign imtdev imtgui imtdb imtdbgui imtlog imtloggui imtwidgets imtstyle imtguigql imtrest imt3d imtlic imtlicgui imt3dgui imt3dview ImtCoreLoc i2d iqt2d iedge
	imtlicgui imtauthgui imtclientgql imtgql imtbase imtauth imtauthgui imtauthdb imtauthgql imtcrypt imtlicgui imtlicgql imtapp imtcolorsdl imtserverapp
	imtqml imtdocguiqml imtauthguiqml imtlicguiqml imtcolguiqml imtguigqlqml imtguiqml imtcontrolsqml imtstylecontrolsqml imtcom imtauthsdl imtappsdl imtservergql imtbasesdl imtbase imtcol imtzip
	imtchat imtchatdb imtdesk imtdeskdb imtdeskgql imtchatgql imtchatsdl imtdesksdl imtdoc
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Gui
	Qt${QT_VERSION_MAJOR}::Sql
	Qt${QT_VERSION_MAJOR}::Xml
	Qt${QT_VERSION_MAJOR}::Widgets
	Qt${QT_VERSION_MAJOR}::Network
	Qt${QT_VERSION_MAJOR}::WebSockets
	Qt${QT_VERSION_MAJOR}::Qml
	Qt${QT_VERSION_MAJOR}::QuickWidgets
)

include(${IACFDIR}/Config/CMake/zlib.cmake)
add_compile_definitions(UNICODE _UNICODE)

