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

puma_declare_library_dependencies(AuthClientSdk LINK_SCOPE PUBLIC
	ImtCore::imtauthguiqml
	ImtCore::imtlicguiqml
	ImtCore::imtdocguiqml
	ImtCore::imtcolguiqml
	ImtCore::imtguigqlqml
	ImtCore::imtguiqml
	ImtCore::imtcontrolsqml
	ImtCore::imtstylecontrolsqml
	ImtCore::imtauthgql
	ImtCore::imtauthdb
	ImtCore::imtchatdb
	ImtCore::imtdeskdb
	ImtCore::imt2dsdl
	ImtCore::ImtCoreLoc
	AcfSln::AcfSlnLoc
	Acf::AcfLoc

)

puma_declare_library_dependencies(AuthServerSdk	LINK_SCOPE PUBLIC
	ImtCore::imtdeskgql
	ImtCore::imtauthgql
	ImtCore::imtchatgql
	ImtCore::imtauthdb
	ImtCore::imtchatdb
	ImtCore::imtdeskdb
	Qt${QT_VERSION_MAJOR}::Widgets
)


# --- Plug-ins ---------------------------------------------------------------
puma_declare_library_dependencies(PumaSettingsPlugin LINK_SCOPE PRIVATE
	ImtCore::imtserverapp
	Qt${QT_VERSION_MAJOR}::Xml
)

# --- Server applications ----------------------------------------------------
puma_declare_library_dependencies(PumaServerPg LINK_SCOPE PRIVATE
	ImtCore::imtserverapp
	ImtCore::imtauthgql
	ImtCore::imtauthdb
	ImtCore::imtlog
	ImtCore::imtchat
)

puma_declare_library_dependencies(PumaServerSl LINK_SCOPE PRIVATE
	ImtCore::imtserverapp
	ImtCore::imtauthgql
	ImtCore::imtauthdb
	ImtCore::imtlog
	ImtCore::imtchat
)

puma_declare_library_dependencies(PumaServerPgTest LINK_SCOPE PRIVATE
	ImtCore::imtserverapp
	ImtCore::imtauthgql
	ImtCore::imtauthdb
	ImtCore::imtlog
	ImtCore::imtchat
)

puma_declare_library_dependencies(PumaServerSlTest LINK_SCOPE PRIVATE
	ImtCore::imtserverapp
	ImtCore::imtauthgql
	ImtCore::imtauthdb
	ImtCore::imtlog
	ImtCore::imtchat
	Acf::AcfLoc
)


puma_declare_library_dependencies(PumaServerConfigurator LINK_SCOPE PRIVATE
	pumaqml
	ImtCore::imtlicguiqml
	ImtCore::imtauthguiqml
	ImtCore::imtdocguiqml
	ImtCore::imtcolguiqml
	ImtCore::imtcontrolsqml
	ImtCore::imtguigqlqml
	ImtCore::imtstylecontrolsqml
	ImtCore::imtguiqml
	ImtCore::imtdeskdb
	ImtCore::imtauthdb
	ImtCore::imtchatdb
	ImtCore::imtserverapp
	ImtCore::imt2dsdl
	ImtCore::ImtCoreLoc
	AcfSln::AcfSlnLoc
	Acf::AcfLoc
)


puma_declare_library_dependencies(PumaClient LINK_SCOPE PRIVATE
	AuthClientSdk 
	AuthServerSdk
	ImtCore::imt3dview 
	ImtCore::imtrepo
)


# --- Test harness -----------------------------------------------------------
if(TARGET pumatest AND NOT TARGET Qt${QT_VERSION_MAJOR}::Test)
	find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test)
endif()

puma_declare_library_dependencies(pumatest LINK_SCOPE PUBLIC
	AuthClientSdk
	AuthServerSdk
	ImtCore::imtgqltest
	Acf::itest
	Qt${QT_VERSION_MAJOR}::Test
)

if(UNIX AND TARGET PumaClient)
	target_link_libraries(PumaClient ${ACF_APPLICATION_LINK_SCOPE} -lz)
endif()
