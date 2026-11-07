project(pumaqml LANGUAGES CXX)


include(${IMTCOREDIR}/Config/CMake/WebCompiler.cmake)
include(${ACFDIR}/Config/CMake/StaticConfig.cmake)

set(buildwebdir ${PUMADIR_BUILD}/AuxInclude/Web)

list(APPEND webdirs ${PUMADIR}/Include/pumaqml/Qml)
list(APPEND webdirs ${buildwebdir}/src)

list(APPEND webdirs ${buildwebdir}/Resources)

getImtCoreQmlWebDirs(webdirs "${buildwebdir}")

if(DEFINED WEB_COMPILE AND WEB_COMPILE)
	set(resname "puma")
	set(inputjs "${PUMADIR}/Include/pumaqml/puma.json")
	set(appicon "../Icons/PumaIcon.svg")
	set(dataroot "/Puma/Views/")
	set(startqml "${PUMADIR}/Include/pumaqml/Qml/PumaWeb.qml")
	jq_compile_web("${webdirs}", "${buildwebdir}", "${resname}", "${inputjs}", "${appicon}", "${dataroot}", "${startqml}")
endif()

