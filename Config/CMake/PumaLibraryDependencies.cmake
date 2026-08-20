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

if(QT_VERSION_MAJOR EQUAL 6)
	acf_declare_target_dependencies(pumaqml LINK_SCOPE ${ACF_LIBRARY_LINK_SCOPE}
		Qt${QT_VERSION_MAJOR}::Core5Compat
	)
endif()

acf_declare_target_dependencies(pumatest LINK_SCOPE ${ACF_APPLICATION_LINK_SCOPE}
	AuthClientSdk
)
