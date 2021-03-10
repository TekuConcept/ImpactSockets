# Find libgnutls
#
#   GNUTLS_INCLUDE_DIRS   - Include directories
#   GNUTLS_LIBRARIES      - Libraries and dependencies
#   GNUTLS_FOUND          - True if found
#

IF (GNUTLS_LIBRARIES AND GNUTLS_INCLUDE_DIRS)
    SET(GNUTLS_FOUND TRUE)
ELSE (GNUTLS_LIBRARIES AND GNUTLS_INCLUDE_DIRS)
    FIND_PATH(GNUTLS_INCLUDE_DIR
        NAMES gnutls.h
        PATHS
        /usr/include/
        /usr/local/include/
        /usr/include/gnutls/
        /usr/local/include/gnutls/
    )

    FIND_LIBRARY(GNUTLS_LIBRARY
        NAMES
        gnutls
        libgnutls
        libgnutlsxx
        PATHS
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/local/lib
        PATH_SUFFIXES
        ${CMAKE_FIND_LIBRARY_SUFFIXES}
        # Windows MSVC prebuilts:
        # <->lib
        # lib<->_imp
        # <->lib_static
        # Windows older "Win32 - MSVC" prebuilts
        # lib<->
    )

    IF (GNUTLS_INCLUDE_DIR AND GNUTLS_LIBRARY)
        SET(GNUTLS_FOUND TRUE)
    ENDIF ()

    SET(GNUTLS_INCLUDE_DIRS ${GNUTLS_INCLUDE_DIR})

    IF (GNUTLS_LIBRARY)
        SET(GNUTLS_LIBRARIES ${GNUTLS_LIBRARIES} ${GNUTLS_LIBRARY})
    ENDIF (GNUTLS_LIBRARY)

    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(ImpactUV DEFAULT_MSG
        GNUTLS_LIBRARIES GNUTLS_INCLUDE_DIRS
    )

    MARK_AS_ADVANCED(GNUTLS_INCLUDE_DIRS GNUTLS_LIBRARIES)
ENDIF (GNUTLS_LIBRARIES AND GNUTLS_INCLUDE_DIRS)
