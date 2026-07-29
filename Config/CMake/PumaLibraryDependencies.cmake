# ---------------------------------------------------------------------------
# PumaLibraryDependencies.cmake
#
# Declares target-based inter-library dependencies for Puma libraries.
# These propagate transitively through the ACF modern-cmake export machinery
# so downstream consumers (via find_package(Puma)) inherit the correct
# include dirs and link deps automatically.
#
# This file is included from the top-level Build/CMake/CMakeLists.txt AFTER
# all sub-projects have been defined.
# ---------------------------------------------------------------------------

if(NOT ACF_MODERN_CMAKE)
	return()
endif()

# pumaqml depends on ImtCore web/qml infrastructure
if(TARGET pumaqml)
	target_link_libraries(pumaqml ${ACF_LIBRARY_LINK_SCOPE}
		Qt${QT_VERSION_MAJOR}::Core
		Qt${QT_VERSION_MAJOR}::Qml
	)
	if(QT_VERSION_MAJOR EQUAL 6)
		target_link_libraries(pumaqml ${ACF_LIBRARY_LINK_SCOPE}
			Qt${QT_VERSION_MAJOR}::Core5Compat
		)
	endif()
endif()
