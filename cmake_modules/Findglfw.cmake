# FindGLFW
# ------------
#
# Locate GLFW library
#
# This module requires
#
# ::
#
#   glfw_DIR, the precompiled binaries folder
#
# This module defines
#
# ::
#
#   glfw_FOUND, if false, do not try to link to glfw
#   glfw::glfw, the target to link against.
#

find_path( glfw_DIR include/GLFW/glfw3.h 
	HINTS
	PATH_SUFFIXES
		GLFW
	PATHS
		/usr/local
		/usr
)

find_path( glfw_INCLUDE_DIR GLFW/glfw3.h 
	HINTS
	PATH_SUFFIXES
		include
	PATHS
		${glfw_DIR}
		/usr/local/include
		/usr/include
)

if( WIN32 )
	if ( MINGW )
		if ( CMAKE_CL_64 OR CMAKE_GENERATOR MATCHES Win64 )
			set( _LIBFOLDER "lib-mingw-w64" )
		else ()
			set( _LIBFOLDER "lib-mingw" )
		endif ()
	elseif ( MSVC_VERSION GREATER_EQUAL 1930 )
		set( _LIBFOLDER "lib-vc2022" )
	elseif ( MSVC_VERSION GREATER_EQUAL 1920 )
		set( _LIBFOLDER "lib-vc2019" )
	elseif ( MSVC_VERSION GREATER_EQUAL 1910 )
		set( _LIBFOLDER "lib-vc2017" )
	elseif ( MSVC14 )
		set( _LIBFOLDER "lib-vc2015" )
	elseif ( MSVC12 )
		set( _LIBFOLDER "lib-vc2013" )
	elseif ( MSVC11 )
		set( _LIBFOLDER "lib-vc2012" )
	else ()
		message( SEND_ERROR "Unsupported compiler" )
	endif ()

	find_path( glfw_LIBRARY_DIR glfw3.lib
		HINTS
		PATH_SUFFIXES
			${_LIBFOLDER}
		PATHS
			${glfw_DIR}
	)

	find_library( glfw_LIBRARY
		NAMES
			glfw3.lib
		HINTS
		PATHS
			${glfw_LIBRARY_DIR}
	)
else()
	find_path( glfw_LIBRARY_DIR libglfw3.so
		HINTS
		PATH_SUFFIXES
			lib
		PATHS
			${glfw_DIR}
	)

	find_library( glfw_LIBRARY
		NAMES
			libglfw3.so
		HINTS
		PATHS
			${glfw_LIBRARY_DIR}
	)
endif()

find_package_handle_standard_args( glfw DEFAULT_MSG glfw_LIBRARY glfw_INCLUDE_DIR )

if ( glfw_FOUND )
	if ( NOT TARGET glfw::glfw )
		add_library( glfw::glfw UNKNOWN IMPORTED )
		set_target_properties(glfw::glfw PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${glfw_INCLUDE_DIR}" )
		set_target_properties( glfw::glfw PROPERTIES
			IMPORTED_LINK_INTERFACE_LANGUAGES "C"
			IMPORTED_LOCATION_DEBUG "${glfw_LIBRARY}"
			IMPORTED_LOCATION_RELEASE "${glfw_LIBRARY}"
			IMPORTED_LOCATION_RELWITHDEBINFO "${glfw_LIBRARY}"
			IMPORTED_LOCATION_MINSIZEREL "${glfw_LIBRARY}" )
		unset( glfw_INCLUDE_DIR CACHE )
		unset( glfw_LIBRARY_DIR CACHE )
		unset( glfw_LIBRARY CACHE )
	endif()
endif ()
