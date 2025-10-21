project(AuthServerSdk LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST ON)

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)
set(TARGET_EXT ".dll")
include(${ACFDIR}/Config/CMake/SharedLibraryConfig.cmake)
include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

target_link_libraries(${PROJECT_NAME} iauth imeas iqtmeas iqtinsp iproc iinsp iipr iprocgui isig iqtsig icalibgui icalib icam iqtcam AcfSlnLoc AcfLoc iedge iedgegui imtrepo iqtgui iview iqtprm iwidgets iloggui iqtdoc ifilegui icomm idoc)
target_link_libraries(${PROJECT_NAME} imtbase imtdesign imtdev imtgui imtdb imtdbgui imtlog imtloggui imtwidgets imtstyle imtguigql imtrest imt3d imtlic imtlicgui imt3dgui imt3dview ImtCoreLoc i2d iqt2d iedge)
target_link_libraries(${PROJECT_NAME} imtlicgui imtauthgui imtclientgql imtgql imtbase imtauth imtauthgui imtauthdb imtauthgql imtcrypt imtlicgui imtlicgql imtapp imtcolorsdl imtserverapp)
target_link_libraries(${PROJECT_NAME} imtqml imtdocguiqml imtauthguiqml imtlicguiqml imtcolguiqml imtguigqlqml imtguiqml imtcontrolsqml imtstylecontrolsqml imtcom imtauthsdl imtappsdl imtservergql imtbasesdl imtcol imtzip)
target_link_libraries(${PROJECT_NAME}  Qt${QT_VERSION_MAJOR}::QuickWidgets  Qt${QT_VERSION_MAJOR}::WebSockets)

ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_UNICODE)
