FIND_PATH(glm_DIR include/glm/glm.hpp
  HINTS
  PATH_SUFFIXES glm
  PATHS
  /usr/local
  /usr
)

FIND_PATH(glm_INCLUDE_DIR glm/glm.hpp
  HINTS
  PATH_SUFFIXES include
  PATHS
  ${glm_DIR}
)

find_package_handle_standard_args( glm DEFAULT_MSG glm_INCLUDE_DIR )

if ( glm_FOUND )
	if ( NOT TARGET glm::glm )
		add_library( glm::glm INTERFACE IMPORTED )
		set_target_properties(glm::glm PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${glm_INCLUDE_DIR}" )
		unset( glm_INCLUDE_DIR CACHE )
	endif()
endif ()
