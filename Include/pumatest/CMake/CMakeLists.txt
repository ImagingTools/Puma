project(pumatest)

include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)
include(${ACFDIR}/Config/CMake/WindeployQt.cmake)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)

target_link_libraries(${PROJECT_NAME} ${ACF_APPLICATION_LINK_SCOPE}
	Qt${QT_VERSION_MAJOR}::Test
	Qt${QT_VERSION_MAJOR}::Sql
	Qt${QT_VERSION_MAJOR}::Xml
	Qt${QT_VERSION_MAJOR}::Core
	Qt${QT_VERSION_MAJOR}::Widgets
	itest ipackage ImtCore::imtbasesdl ImtCore::imtbase ImtCore::imtauth ImtCore::imtcrypt ImtCore::imtservergql ImtCore::imtdb ImtCore::imtgql ImtCore::imtgqltest ImtCore::imtauthgql ifile iser ImtCore::imtauthsdl ImtCore::imtcol
	ImtCore::imtserverapp
	AuthClientSdk AuthServerSdk
)

include(${IMTCOREDIR}/Config/CMake/ImtCore.cmake)

if(WIN32)
	set(listFiles \"${AUX_INCLUDE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME}/${PROJECT_NAME}.exe\")
	windeploy(${PROJECT_NAME} "" "${listFiles}")
endif()

