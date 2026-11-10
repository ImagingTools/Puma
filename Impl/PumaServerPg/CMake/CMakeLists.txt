project(PumaServerPg LANGUAGES CXX)


set(ARXC_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/PumaServer.awc")
set(ARXC_FILES "${PROJECT_SOURCE_DIR}/../${PROJECT_NAME}.acc")
set(ARXC_OUTDIR "${AUX_INCLUDE_DIR}")

if(WIN32)
#	set(CMAKE_WIN32_EXECUTABLE "on")
	set(ACF_CONVERT_FILES "${PROJECT_SOURCE_DIR}/../VC/PumaServerPg.rc.xtracf")
	set(ACF_CONVERT_REGISTRY "${PROJECT_SOURCE_DIR}/../VC/FileSubstitCopyApp.acc")
	set(ACF_CONVERT_CONFIG "${PROJECT_SOURCE_DIR}/../../../Config/BaseOnly.awc")
	set(RC_FILE "${AUX_INCLUDE_DIR}/${PROJECT_NAME}/PumaServerPg.rc")
endif()

include(${ACFDIR}/Config/CMake/CustomBuild.cmake)
include(${ACFDIR}/Config/CMake/ApplicationConfig.cmake)
include(${IMTCOREDIR}/Config/CMake/OpenSSL.cmake)
include(${IMTCOREDIR}/Config/CMake/Quazip.cmake)
include(${ACFDIR}/Config/CMake/DeployQt.cmake)

if(WIN32 AND NOT USE_FIND_PACKAGE)
	file(COPY ${OPENSSLDIR}/lib/x64/libcrypto-3-x64.dll DESTINATION ${PROJECT_SOURCE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})
	file(COPY ${OPENSSLDIR}/lib/x64/libssl-3-x64.dll DESTINATION ${PROJECT_SOURCE_DIR}/../../../Bin/${CMAKE_BUILD_TYPE}_${TARGETNAME})
endif()

target_link_libraries(PumaServerPg PRIVATE
	ImtCore::imtserverapp
	ImtCore::imtauthgql
	ImtCore::imtauthdb
	ImtCore::imtnotifydb
	ImtCore::imtlog
	ImtCore::imtchat
	ImtCore::ImtCoreLoc
	Acf::iqtdoc
)

include(${IACFDIR}/Config/CMake/zlib.cmake)

set(DEPLOY_QT_QML_DIRS "${IMTCOREDIR}/Qml")
deploy_qt(TARGET ${PROJECT_NAME})
unset(DEPLOY_QT_QML_DIRS)

# Self-contained install tree on Linux, for containers and systemd:
#   cmake --install <build dir> --component PumaServerPg --prefix <dir>
# Flat layout: libraries next to the executable, Qt plugins in bin/plugins, qt.conf pointing there.
# Linux has no Qt deploy tool, so Qt's CMake deployment API collects the dependencies.
# Windows and macOS keep their current deployment.
# TODO: other headless servers need the same install-time deployment (plugin options, RUNPATH fix):
# move it to a common helper, e.g. next to deploy_qt() in Acf/Config/CMake/DeployQt.cmake.
if(UNIX AND NOT APPLE)
	# The Qt PostgreSQL plugin is dlopen-ed and would otherwise find the system libpq, whose dependencies
	# (libldap, krb5) need the system OpenSSL, while the process already has the conan OpenSSL loaded:
	# "libldap.so.2: undefined symbol: EVP_md2". Link the conan libpq to the executable, so the loader
	# preloads it through the executable's RUNPATH and the install collects it with its dependencies.
	find_package(PostgreSQL REQUIRED)
	target_link_libraries(${PROJECT_NAME} PRIVATE PostgreSQL::PostgreSQL)
	# No code calls libpq, so --as-needed would drop it from DT_NEEDED: Main.cpp references PQlibVersion to keep it.

	set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")
	install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin COMPONENT ${PROJECT_NAME})

	qt_generate_deploy_script(
		TARGET ${PROJECT_NAME}
		OUTPUT_SCRIPT deploy_script
		CONTENT "
		# Plugin RUNPATHs are computed from these variables, not from the arguments below
		set(QT_DEPLOY_BIN_DIR bin)
		set(QT_DEPLOY_LIB_DIR bin)
		set(QT_DEPLOY_PLUGINS_DIR bin/plugins)
		# Headless server. By default Qt deploys every plugin type of every linked Qt module, and the xcb platform for QtGui.
		# Excluded are only the plugins that bring Qt libraries or system libraries nothing else needs.
		qt_deploy_runtime_dependencies(
			EXECUTABLE \"$<TARGET_FILE:${PROJECT_NAME}>\"
			BIN_DIR bin
			LIB_DIR bin
			PLUGINS_DIR bin/plugins
			GENERATE_QT_CONF
			NO_TRANSLATIONS
			EXCLUDE_PLUGIN_TYPES
				# QtGui: window systems (xcb, EGLFS) and platform themes (GTK3)
				egldeviceintegrations platformthemes xcbglintegrations
				# QtGui: Wayland
				wayland-decoration-client wayland-graphics-integration-client wayland-shell-integration
				# QtQml: QML debugging and profiling (Quick, QmlModels...)
				qmltooling
			EXCLUDE_PLUGINS
				# QtGui: the xcb platform, the server uses offscreen
				qxcb
				# QtSql: every driver but PostgreSQL and SQLite which we actually use
				qsqlibase qsqlmimer qsqlmysql qsqloci qsqlodbc
			INCLUDE_PLUGINS
				# QtGui: the server still links QtGui (QtWidgets, through SingletonDocApplication), so it needs a platform plugin
				qoffscreen
		)
	")
	install(SCRIPT "${deploy_script}" COMPONENT ${PROJECT_NAME})

	# Fix RUNPATH for shared libraries copied as runtime dependencies: the conan libraries have none,
	# with $ORIGIN they find each other in bin.
	# Must run after every copy into bin, as each copy restores the original file.
	# Only the libraries directly in bin, plugins have their RUNPATH from Qt already.
	find_program(PATCHELF_EXECUTABLE patchelf REQUIRED)
	install(CODE "set(patchelf \"${PATCHELF_EXECUTABLE}\")" COMPONENT ${PROJECT_NAME})
	install(CODE [=[
		file(GLOB files LIST_DIRECTORIES false "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/bin/*.so*")
		foreach(file IN LISTS files)
			if(IS_SYMLINK "${file}")
				continue()
			endif()
			execute_process(COMMAND "${patchelf}" --set-rpath [[$ORIGIN]] "${file}" COMMAND_ERROR_IS_FATAL ANY)
		endforeach()
	]=] COMPONENT ${PROJECT_NAME})
endif()
