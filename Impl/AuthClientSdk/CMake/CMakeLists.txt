project(AuthClientSdk LANGUAGES CXX)

set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")
set(ARX_ENABLE_GENERATE_DEPENDENCIES_LIST OFF)

include(${ACFDIR}/Config/CMake/StaticConfig.cmake)

set(ACC_FILES
    "${PROJECT_SOURCE_DIR}/../AuthClientSdk.acc"
    "${PROJECT_SOURCE_DIR}/../AdministrationWidget.acc"
    "${PROJECT_SOURCE_DIR}/../LoginWidget.acc"
)

foreach(acc ${ACC_FILES})
    get_filename_component(acc_name "${acc}" NAME_WE)

    set(ARXC_FILES "${acc}")
    include(${ACFDIR}/Config/CMake/CustomBuild.cmake)

    target_sources(${PROJECT_NAME}
        PRIVATE
        "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/C${acc_name}.cpp"
        "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/C${acc_name}.h"
    )
endforeach()

include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)

target_link_libraries(${PROJECT_NAME}
    AcfSlnLoc AcfLoc ImtCoreLoc
    imeas iproc iqtgui iauth icomp idoc i2d icomm
    imtbase imtbasesdl imtauthsdl imtgql imtcom imtservergql imtserverapp imtauthgql imtclientgql imtlic imtcol imtdb
    imtstyle imtcontrolsqml imtguiqml imtauthguiqml imtguigqlqml imtstylecontrolsqml imtlicguiqml imtdocguiqml imtcolguiqml imtlicgui imtgui imtcolorsdl
    imtauthgui imtqml
)

add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)
