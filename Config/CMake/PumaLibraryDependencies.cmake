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
# Puma uses explicit PUBLIC/PRIVATE/INTERFACE link scopes.
#
# Included once, centrally, from Build/CMake/CMakeLists.txt after all library
# targets have been created.
# ---------------------------------------------------------------------------

# The SDL code generated into ImtCore::imtbasesdl serializes through
# imtgql::CGqlParamObject, but ImtCore does not declare that edge, so GNU ld places
# libimtgql.a before libimtbasesdl.a and leaves those symbols undefined. Augmenting the
# imported target here is safe: Puma sees ImtCore as plain imported static libraries,
# without the Qt autogen targets that make the same edge a hard cycle inside ImtCore.
declare_target_dependencies(ImtCore::imtbasesdl LINK_SCOPE INTERFACE
	ImtCore::imtgql
)

if(QT_VERSION_MAJOR EQUAL 6)
	declare_target_dependencies(pumaqml LINK_SCOPE PUBLIC
		Qt${QT_VERSION_MAJOR}::Core5Compat
	)
endif()

declare_target_dependencies(pumatest LINK_SCOPE PRIVATE
	AuthClientSdk
	AuthServerSdk
	ImtCore::imtgqltest
	Acf::itest
	Qt${QT_VERSION_MAJOR}::Test
)

# declare_target_dependencies() skips missing targets silently, and ImtCore builds and
# exports imtgqltest only when it is itself configured with BUILD_TESTING=ON - without
# this check the omission surfaces much later as unresolved CGqlSdlRequestTest symbols.
if(TARGET pumatest AND NOT TARGET ImtCore::imtgqltest)
	message(FATAL_ERROR
		"pumatest requires ImtCore::imtgqltest, which is missing from the ImtCore package at "
		"'${ImtCore_DIR}'. Reconfigure ImtCore with -DBUILD_TESTING=ON, or configure Puma with "
		"-DBUILD_TESTING=OFF to skip the tests.")
endif()
