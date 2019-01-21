#.rst:
# FindImpactSockets
# --------
#
# Find ImpactSockets
#
# Find the ImpactSockets headers and libraries.
#
# ::
#
#   SOCKETS_INCLUDE_DIRS    - where to find sockets/sockets.h, etc.
#   SOCKETS_LIBRARIES      - List of libraries when using ImpactSockets.
#   SOCKETS_FOUND          - True if ImpactSockets found.
#   SOCKETS_VERSION_STRING - the version of ImpactSockets found.
#

IF (SOCKETS_LIBRARIES AND SOCKETS_INCLUDE_DIRS)
    SET(SOCKETS_FOUND TRUE)
ELSE (SOCKETS_LIBRARIES AND SOCKETS_INCLUDE_DIRS)
    FIND_PATH(SOCKETS_INCLUDE_DIR
        NAMES basic_socket
        PATHS
        /usr/include/sockets
        /usr/local/include/sockets
    )
    
    FIND_LIBRARY(SOCKETS_LIBRARY
        NAMES ImpactSockets
        PATHS
        /usr/lib
        /usr/local/lib
        # Windows MSVC prebuilts:
        # <->lib
        # lib<->_imp
        # <->lib_static
        # Windows older "Win32 - MSVC" prebuilts
        # lib<->
    )
    
    #IF (SOCKETS_INCLUDE_DIR)
    #    IF (EXISTS "${SOCKETS_INCLUDE_DIR}/SocVersion.h")
    #        FILE(STRINGS "${SOCKETS_INCLUDE_DIR}/SocVersion.h" soc_version_str REGEX "^#define[\t ]+SOCKETS_VERSION[\t ]+\".*\"")
    #        STRING(REGEX REPLACE "^#define[\t ]+SOCKETS_VERSION[\t ]+\"([^\"]*)\".*" "\\1" SOCKETS_VERSION_STRING "${soc_version_str}")
    #        UNSET(soc_version_str)
    #    ENDIF ()
    #ENDIF ()
    
    IF (SOCKETS_INCLUDE_DIR AND SOCKETS_LIBRARY)
        SET(SOCKETS_FOUND TRUE)
    ENDIF ()

    SET(SOCKETS_INCLUDE_DIRS ${SOCKETS_INCLUDE_DIR})

    IF (SOCKETS_LIBRARY)
        SET(SOCKETS_LIBRARIES ${SOCKETS_LIBRARIES} ${SOCKETS_LIBRARY})
    ENDIF (SOCKETS_LIBRARY)

    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(ImpactSockets DEFAULT_MSG
        SOCKETS_LIBRARIES SOCKETS_INCLUDE_DIRS
        #VERSION_VAR SOCKETS_VERSION_STRING
    )

    MARK_AS_ADVANCED(SOCKETS_INCLUDE_DIRS SOCKETS_LIBRARIES)
ENDIF (SOCKETS_LIBRARIES AND SOCKETS_INCLUDE_DIRS)