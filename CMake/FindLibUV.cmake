# Find libuv
#
#   UV_INCLUDE_DIRS   - Include directories
#   UV_LIBRARIES      - Libraries and dependencies
#   UV_FOUND          - True if found
#

IF (UV_LIBRARIES AND UV_INCLUDE_DIRS)
    SET(UV_FOUND TRUE)
ELSE (UV_LIBRARIES AND UV_INCLUDE_DIRS)
    FIND_PATH(UV_INCLUDE_DIR
        NAMES uv.h
        PATHS
        /usr/include/
        /usr/local/include/
    )

    FIND_LIBRARY(UV_LIBRARY
        NAMES uv libuv
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

    IF (UV_INCLUDE_DIR AND UV_LIBRARY)
        SET(UV_FOUND TRUE)
    ENDIF ()

    SET(UV_INCLUDE_DIRS ${UV_INCLUDE_DIR})

    IF (UV_LIBRARY)
        SET(UV_LIBRARIES ${UV_LIBRARIES} ${UV_LIBRARY})
    ENDIF (UV_LIBRARY)

    INCLUDE(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(ImpactUV DEFAULT_MSG
        UV_LIBRARIES UV_INCLUDE_DIRS
    )

    MARK_AS_ADVANCED(UV_INCLUDE_DIRS UV_LIBRARIES)
ENDIF (UV_LIBRARIES AND UV_INCLUDE_DIRS)
