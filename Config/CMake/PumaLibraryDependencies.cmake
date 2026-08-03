# ---------------------------------------------------------------------------
# Clean, target-based inter-library dependency graph for Puma.
#
# This mirrors the approach introduced for the ACF foundation (Acf) in
# Config/CMake/AcfLibraryDependencies.cmake, for AcfSln in
# Config/CMake/AcfSlnLibraryDependencies.cmake, for IAcf in
# Config/CMake/IAcfLibraryDependencies.cmake, for ImtCore in
# Config/CMake/ImtCoreLibraryDependencies.cmake and for IotPlatform in
# Config/CMake/IotPlatformLibraryDependencies.cmake: instead of relying on the
# final executable/package link to resolve symbols and on a hand-tuned build
# order (the inline target_link_libraries() spread across the per-library CMake
# files), the dependencies between the Puma libraries - and their dependencies
# onto the underlying ImtCore::, Acf::, AcfSln:: and IAcf:: libraries - are
# declared here as target usage requirements. Include paths and link order then
# propagate transitively and automatically for the in-tree build.
#
# Every ImtCore::/Acf::/AcfSln:: imported target exposes its whole source include
# tree (acf_register_library() adds INCLUDE_DIR/IMPL_DIR as PUBLIC include
# directories), so a single ImtCore:: dependency transitively provides the full
# ImtCore, Acf, AcfSln and IAcf header search paths to the consuming library.
#
# Unlike ImtCore/IotPlatform, Puma has almost no reusable static library layer of
# its own: pumaqml is a web-resource library and the substantive dependency graph
# lives in the Arxc-generated SDK shared libraries (AuthClientSdk / AuthServerSdk,
# Windows-only). Those are declared here as well so their historical link closure
# is centralised in one place instead of being duplicated inline.
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
# AuthClientSdk: client-side authentication/administration SDK.
puma_declare_library_dependencies(AuthClientSdk	LINK_SCOPE PUBLIC
	Acf::i2d Acf::icomp Acf::idoc Acf::iqtgui Acf::AcfLoc)
puma_declare_library_dependencies(AuthClientSdk	LINK_SCOPE PUBLIC
	AcfSln::iauth AcfSln::icomm AcfSln::imeas AcfSln::iproc AcfSln::AcfSlnLoc)
puma_declare_library_dependencies(AuthClientSdk	LINK_SCOPE PUBLIC
	ImtCore::imtbasesdl ImtCore::imtbase ImtCore::imtauthsdl ImtCore::imtappsdl ImtCore::imtcolorsdl ImtCore::imt2dsdl
	ImtCore::imtgql ImtCore::imtcom ImtCore::imtservergql ImtCore::imtserverapp ImtCore::imtauthgql ImtCore::imtclientgql
	ImtCore::imtlic ImtCore::imtcol ImtCore::imtdb ImtCore::imtrest ImtCore::imtauth ImtCore::imtapp ImtCore::imtqml
	ImtCore::imtgui ImtCore::imtstyle ImtCore::imtlicgui ImtCore::imtauthgui ImtCore::imtauthdb ImtCore::imtchatdb ImtCore::imtdeskdb
	ImtCore::imtcontrolsqml ImtCore::imtguiqml ImtCore::imtauthguiqml ImtCore::imtguigqlqml ImtCore::imtstylecontrolsqml
	ImtCore::imtlicguiqml ImtCore::imtdocguiqml ImtCore::imtcolguiqml ImtCore::ImtCoreLoc)
puma_declare_library_dependencies(AuthClientSdk	LINK_SCOPE PUBLIC
	Qt${QT_VERSION_MAJOR}::WebSockets Qt${QT_VERSION_MAJOR}::QuickWidgets)

# AuthServerSdk: server-side authentication SDK.
puma_declare_library_dependencies(AuthServerSdk	LINK_SCOPE PUBLIC
	Acf::iser Acf::i2d Acf::idoc Acf::iqt2d Acf::iqtgui Acf::iqtdoc Acf::iqtprm Acf::iwidgets Acf::ifilegui
	Acf::iloggui Acf::iview Acf::AcfLoc)
puma_declare_library_dependencies(AuthServerSdk	LINK_SCOPE PUBLIC
	AcfSln::iauth AcfSln::imeas AcfSln::iqtmeas AcfSln::iqtinsp AcfSln::iproc AcfSln::iinsp AcfSln::iipr AcfSln::iprocgui
	AcfSln::isig AcfSln::iqtsig AcfSln::icalib AcfSln::icalibgui AcfSln::icam AcfSln::iqtcam AcfSln::iedge AcfSln::iedgegui
	AcfSln::icomm AcfSln::AcfSlnLoc)
puma_declare_library_dependencies(AuthServerSdk	LINK_SCOPE PUBLIC
	ImtCore::imtbasesdl ImtCore::imtauthsdl ImtCore::imtappsdl ImtCore::imtcolorsdl ImtCore::imtchatsdl ImtCore::imtdesksdl
	ImtCore::imtbase ImtCore::imtdesign ImtCore::imtdev ImtCore::imtrepo ImtCore::imtlog ImtCore::imtloggui ImtCore::imtcrypt
	ImtCore::imtdoc ImtCore::imtcol ImtCore::imtzip ImtCore::imtwidgets ImtCore::imtgui ImtCore::imtstyle ImtCore::imtdb
	ImtCore::imtdbgui ImtCore::imtapp ImtCore::imtcom ImtCore::imtrest ImtCore::imtgql ImtCore::imtguigql ImtCore::imtclientgql
	ImtCore::imtservergql ImtCore::imtserverapp ImtCore::imtqml ImtCore::imt3d ImtCore::imt3dgui ImtCore::imt3dview
	ImtCore::imtlic ImtCore::imtlicgui ImtCore::imtlicgql ImtCore::imtauth ImtCore::imtauthgui ImtCore::imtauthdb ImtCore::imtauthgql
	ImtCore::imtchat ImtCore::imtchatdb ImtCore::imtchatgql ImtCore::imtdesk ImtCore::imtdeskdb ImtCore::imtdeskgql
	ImtCore::imtcontrolsqml ImtCore::imtstylecontrolsqml ImtCore::imtguiqml ImtCore::imtguigqlqml ImtCore::imtauthguiqml
	ImtCore::imtlicguiqml ImtCore::imtcolguiqml ImtCore::imtdocguiqml ImtCore::ImtCoreLoc)
puma_declare_library_dependencies(AuthServerSdk	LINK_SCOPE PUBLIC
	Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Xml Qt${QT_VERSION_MAJOR}::WebSockets Qt${QT_VERSION_MAJOR}::QuickWidgets)
