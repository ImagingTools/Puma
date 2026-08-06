# ---------------------------------------------------------------------------
# Clean, target-based inter-library dependency graph for Puma.
#
# Instead of relying on the final executable/package link to resolve symbols and
# on a hand-tuned build order (the inline target_link_libraries() that used to be
# spread across the per-library CMake files), the dependencies between the Puma
# libraries, applications and packages - and their dependencies onto the
# underlying ImtCore::, Acf::, AcfSln:: and IAcf:: libraries - are declared here
# as target usage requirements. Include paths and link order then propagate
# transitively and automatically for the in-tree build.
#
# Every ImtCore::/Acf::/AcfSln:: imported target exposes its whole source include
# tree (acf_register_library() adds INCLUDE_DIR/IMPL_DIR as PUBLIC include
# directories), so a single ImtCore:: dependency transitively provides the full
# ImtCore, Acf, AcfSln and IAcf header search paths to the consuming target.
#
# Dependencies are declared *minimally*: each target lists only its direct
# dependencies; indirect ones propagate automatically through the graph (for
# example ImtCore::imtserverapp already pulls in imtdb/imtqml/imtauth/imtrest/...
# so those are not repeated). Do not add a dependency that is already reachable
# through another listed target.
#
# The target_link_libraries() signature is controlled by ACF_LIBRARY_LINK_SCOPE:
#  * when empty, the plain signature is used (matching the legacy Puma CMake),
#  * when set to PUBLIC/PRIVATE/INTERFACE, the keyword signature is used.
# CMake forbids mixing the plain and keyword signatures on the same target.
#
# Included once, centrally, from Build/CMake/CMakeLists.txt after all library
# targets have been created.
# ---------------------------------------------------------------------------

# Declare the dependencies of a Puma library, ignoring any entry whose target
# does not exist in the current configuration (for example Windows-only SDK
# libraries, or ImtCore::/Acf::/AcfSln::/IAcf:: targets that are not available
# because the legacy shim is used instead of find_package).
function(puma_declare_library_dependencies target)
	cmake_parse_arguments(ARG "" "LINK_SCOPE" "" ${ARGN})

	if(NOT ARG_LINK_SCOPE)
		set(ARG_LINK_SCOPE ${ACF_LIBRARY_LINK_SCOPE})
	endif()

	if(NOT TARGET ${target})
		return()
	endif()

	# target_link_libraries() is illegal on an ALIAS target (e.g. in-tree ImtCore::
	# aliases in unified builds), and augmenting the real target can inject dependency
	# cycles through the Qt autogen targets. Never target an alias.
	get_target_property(_puma_aliased ${target} ALIASED_TARGET)
	if(_puma_aliased)
		return()
	endif()

	foreach(dependency IN LISTS ARG_UNPARSED_ARGUMENTS)
		if(TARGET ${dependency})
			target_link_libraries(${target} ${ARG_LINK_SCOPE} ${dependency})
		endif()
	endforeach()
endfunction()


# --- QML web-resource libraries ---------------------------------------------
if(QT_VERSION_MAJOR EQUAL 6)
	puma_declare_library_dependencies(pumaqml	LINK_SCOPE PUBLIC	Qt${QT_VERSION_MAJOR}::Core5Compat)
endif()

# --- Arxc-generated SDK shared libraries (Windows-only) ---------------------
# AuthClientSdk: client-side authentication/administration SDK. Linking
# ImtCore::imtserverapp already provides the imtdb/imtqml/imtauth/imtrest/... core
# transitively, so only the additional roots are listed.
puma_declare_library_dependencies(AuthClientSdk	LINK_SCOPE PUBLIC
	Acf::i2d Acf::icomp Acf::idoc Acf::iqtgui Acf::AcfLoc
	AcfSln::iauth AcfSln::icomm AcfSln::imeas AcfSln::iproc AcfSln::AcfSlnLoc)
puma_declare_library_dependencies(AuthClientSdk	LINK_SCOPE PUBLIC
	ImtCore::imtserverapp ImtCore::imtauthgql ImtCore::imtauthdb ImtCore::imtchatdb ImtCore::imtdeskdb ImtCore::imt2dsdl
	ImtCore::imtcontrolsqml ImtCore::imtguiqml ImtCore::imtauthguiqml ImtCore::imtguigqlqml ImtCore::imtstylecontrolsqml
	ImtCore::imtlicguiqml ImtCore::imtdocguiqml ImtCore::imtcolguiqml ImtCore::ImtCoreLoc)
puma_declare_library_dependencies(AuthClientSdk	LINK_SCOPE PUBLIC
	Qt${QT_VERSION_MAJOR}::WebSockets Qt${QT_VERSION_MAJOR}::QuickWidgets)

# AuthServerSdk: server-side authentication/administration SDK. This is a
# server/console-oriented component and deliberately does NOT depend on the 3D
# stack (imt3d/imt3dgui/imt3dview). Linking ImtCore::imtserverapp provides the
# db/qml/auth/rest core transitively.
puma_declare_library_dependencies(AuthServerSdk	LINK_SCOPE PUBLIC
	Acf::iser Acf::i2d Acf::idoc Acf::iqt2d Acf::iqtgui Acf::iqtdoc Acf::iqtprm Acf::iwidgets Acf::ifilegui
	Acf::iloggui Acf::iview Acf::AcfLoc)
puma_declare_library_dependencies(AuthServerSdk	LINK_SCOPE PUBLIC
	AcfSln::iauth AcfSln::imeas AcfSln::iqtmeas AcfSln::iqtinsp AcfSln::iproc AcfSln::iinsp AcfSln::iipr AcfSln::iprocgui
	AcfSln::isig AcfSln::iqtsig AcfSln::icalib AcfSln::icalibgui AcfSln::icam AcfSln::iqtcam AcfSln::iedge AcfSln::iedgegui
	AcfSln::icomm AcfSln::AcfSlnLoc)
puma_declare_library_dependencies(AuthServerSdk	LINK_SCOPE PUBLIC
	ImtCore::imtserverapp ImtCore::imtdev ImtCore::imtdbgui ImtCore::imtloggui ImtCore::imtauthdb ImtCore::imtauthgql
	ImtCore::imtlicgql ImtCore::imtzip ImtCore::imtchatdb ImtCore::imtdeskdb ImtCore::imtdeskgql ImtCore::imtchatgql
	ImtCore::imtcontrolsqml ImtCore::imtstylecontrolsqml ImtCore::imtguiqml ImtCore::imtguigqlqml ImtCore::imtauthguiqml
	ImtCore::imtlicguiqml ImtCore::imtcolguiqml ImtCore::imtdocguiqml ImtCore::ImtCoreLoc)
puma_declare_library_dependencies(AuthServerSdk	LINK_SCOPE PUBLIC
	Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::WebSockets Qt${QT_VERSION_MAJOR}::QuickWidgets)


# --- Plug-ins ---------------------------------------------------------------
puma_declare_library_dependencies(PumaSettingsPlugin	LINK_SCOPE PRIVATE
	ImtCore::imtserverapp Acf::i2d Acf::ifile Acf::istd Acf::idoc
	Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Xml)


# --- Server applications ----------------------------------------------------
# PumaServerPg / PumaServerSl and their *Test twins share the same closure.
foreach(_puma_server PumaServerPg PumaServerSl PumaServerPgTest PumaServerSlTest)
	puma_declare_library_dependencies(${_puma_server}	LINK_SCOPE PRIVATE
		ImtCore::imtserverapp ImtCore::imtlicdb ImtCore::imtauthdb ImtCore::imtauthgql ImtCore::imtlicgql
		ImtCore::imtgui ImtCore::imtchat ImtCore::imtzip ImtCore::imtrepo ImtCore::imtlog
		AcfSln::iauth AcfSln::iservice
		Qt${QT_VERSION_MAJOR}::WebSockets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Qml Qt${QT_VERSION_MAJOR}::Widgets)
endforeach()

puma_declare_library_dependencies(PumaServerConfigurator	LINK_SCOPE PRIVATE
	pumaqml Acf::AcfLoc AcfSln::AcfSlnLoc ImtCore::ImtCoreLoc
	ImtCore::imtserverapp ImtCore::imt2dsdl ImtCore::imtauthdb ImtCore::imtchatdb ImtCore::imtdeskdb
	ImtCore::imtcontrolsqml ImtCore::imtstylecontrolsqml ImtCore::imtguiqml ImtCore::imtguigqlqml
	ImtCore::imtauthguiqml ImtCore::imtcolguiqml ImtCore::imtdocguiqml ImtCore::imtlicguiqml
	Qt${QT_VERSION_MAJOR}::QuickWidgets Qt${QT_VERSION_MAJOR}::WebSockets Qt${QT_VERSION_MAJOR}::Sql)


# --- Windows GUI client -----------------------------------------------------
# Vision client: keeps the 3D and measurement/vision stack. Links the two SDK
# shared libraries and the additional ImtCore/Acf roots it uses directly.
puma_declare_library_dependencies(PumaClient	LINK_SCOPE PRIVATE
	AuthClientSdk AuthServerSdk)
puma_declare_library_dependencies(PumaClient	LINK_SCOPE PRIVATE
	AcfSln::iauth AcfSln::imeas AcfSln::iqtmeas AcfSln::iqtinsp AcfSln::iproc AcfSln::iinsp AcfSln::iipr AcfSln::iprocgui
	AcfSln::isig AcfSln::iqtsig AcfSln::icalib AcfSln::icalibgui AcfSln::icam AcfSln::iqtcam AcfSln::iedge AcfSln::iedgegui
	AcfSln::icomm AcfSln::AcfSlnLoc
	Acf::iqtgui Acf::iview Acf::iqtprm Acf::iwidgets Acf::iloggui Acf::iqtdoc Acf::ifilegui Acf::istd Acf::icomp
	Acf::iprm Acf::imod Acf::iser Acf::iqt Acf::ilog Acf::ibase Acf::i2d Acf::iqt2d Acf::AcfLoc)
puma_declare_library_dependencies(PumaClient	LINK_SCOPE PRIVATE
	ImtCore::imtrest ImtCore::imtdev ImtCore::imtrepo ImtCore::imtloggui ImtCore::imt3dview
	ImtCore::imtauthgql ImtCore::imtlicgql ImtCore::imtauthdb ImtCore::imtzip
	ImtCore::imtcontrolsqml ImtCore::imtstylecontrolsqml ImtCore::imtguiqml ImtCore::imtguigqlqml
	ImtCore::imtauthguiqml ImtCore::imtlicguiqml ImtCore::imtcolguiqml ImtCore::imtdocguiqml ImtCore::ImtCoreLoc)
puma_declare_library_dependencies(PumaClient	LINK_SCOPE PRIVATE
	Qt${QT_VERSION_MAJOR}::QuickWidgets Qt${QT_VERSION_MAJOR}::WebSockets Qt${QT_VERSION_MAJOR}::Sql)


# --- Test harness -----------------------------------------------------------
puma_declare_library_dependencies(pumatest	LINK_SCOPE PUBLIC
	AuthClientSdk AuthServerSdk
	ImtCore::imtserverapp ImtCore::imtgqltest ImtCore::imtauthgql
	Acf::itest Acf::ipackage Acf::ifile Acf::iser
	Qt${QT_VERSION_MAJOR}::Test Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets)
