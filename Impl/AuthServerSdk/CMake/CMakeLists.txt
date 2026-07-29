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
	AcfSln::iauth Acf::iser AcfSln::imeas AcfSln::iqtmeas AcfSln::iqtinsp AcfSln::iproc AcfSln::iinsp AcfSln::iipr AcfSln::iprocgui AcfSln::isig AcfSln::iqtsig AcfSln::icalibgui AcfSln::icalib AcfSln::icam AcfSln::iqtcam AcfSln::AcfSlnLoc Acf::AcfLoc AcfSln::iedge AcfSln::iedgegui ImtCore::imtrepo Acf::iqtgui Acf::iview Acf::iqtprm Acf::iwidgets Acf::iloggui Acf::iqtdoc Acf::ifilegui AcfSln::icomm Acf::idoc
	ImtCore::imtbase ImtCore::imtdesign ImtCore::imtdev ImtCore::imtgui ImtCore::imtdb ImtCore::imtdbgui ImtCore::imtlog ImtCore::imtloggui ImtCore::imtwidgets ImtCore::imtstyle ImtCore::imtguigql ImtCore::imtrest ImtCore::imt3d ImtCore::imtlic ImtCore::imtlicgui ImtCore::imt3dgui ImtCore::imt3dview ImtCore::ImtCoreLoc Acf::i2d Acf::iqt2d AcfSln::iedge
	ImtCore::imtlicgui ImtCore::imtauthgui ImtCore::imtclientgql ImtCore::imtgql ImtCore::imtbase ImtCore::imtauth ImtCore::imtauthgui ImtCore::imtauthdb ImtCore::imtauthgql ImtCore::imtcrypt ImtCore::imtlicgui ImtCore::imtlicgql ImtCore::imtapp ImtCore::imtcolorsdl ImtCore::imtserverapp
	ImtCore::imtqml ImtCore::imtdocguiqml ImtCore::imtauthguiqml ImtCore::imtlicguiqml ImtCore::imtcolguiqml ImtCore::imtguigqlqml ImtCore::imtguiqml ImtCore::imtcontrolsqml ImtCore::imtstylecontrolsqml ImtCore::imtcom ImtCore::imtauthsdl ImtCore::imtappsdl ImtCore::imtservergql ImtCore::imtbasesdl ImtCore::imtbase ImtCore::imtcol ImtCore::imtzip
	ImtCore::imtchat ImtCore::imtchatdb ImtCore::imtdesk ImtCore::imtdeskdb ImtCore::imtdeskgql ImtCore::imtchatgql ImtCore::imtchatsdl ImtCore::imtdesksdl ImtCore::imtdoc
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

