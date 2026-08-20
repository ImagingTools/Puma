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
# Puma uses keyword target_link_libraries() signatures (PUBLIC/PRIVATE/
# INTERFACE) consistently via ACF_LIBRARY_LINK_SCOPE.
#
# Included once, centrally, from Build/CMake/CMakeLists.txt after all library
# targets have been created.
# ---------------------------------------------------------------------------

# Declare the dependencies of a Puma library, ignoring any entry whose target
# does not exist in the current configuration (for example Windows-only SDK
# libraries, or ImtCore::/Acf::/AcfSln::/IAcf:: targets that are not available).
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


if(QT_VERSION_MAJOR EQUAL 6)
	puma_declare_library_dependencies(pumaqml LINK_SCOPE ${ACF_LIBRARY_LINK_SCOPE}
		Qt${QT_VERSION_MAJOR}::Core5Compat
	)
endif()

puma_declare_library_dependencies(pumatest LINK_SCOPE ${ACF_APPLICATION_LINK_SCOPE}
	AuthClientSdk
)
