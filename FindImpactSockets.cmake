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

# Look for the header file.
find_path(SOCKETS_INCLUDE_DIR
  PATHS
  /usr/include/sockets
  /usr/local/include/sockets
  NAMES basic_socket
)
message("Library: ${SOCKETS_INCLUDE_DIR}")
mark_as_advanced(SOCKETS_INCLUDE_DIR)

# Look for the library (sorted from most current/relevant entry to least).
find_library(SOCKETS_LIBRARY NAMES
    ImpactSockets
  # Windows MSVC prebuilts:
  # <->lib
  # lib<->_imp
  # <->lib_static
  # Windows older "Win32 - MSVC" prebuilts
  # lib<->
)
mark_as_advanced(SOCKETS_LIBRARY)

if(SOCKETS_INCLUDE_DIR)
  if(EXISTS "${SOCKETS_INCLUDE_DIR}/SocVersion.h")
    file(STRINGS "${SOCKETS_INCLUDE_DIR}/SocVersion.h" soc_version_str REGEX "^#define[\t ]+SOCKET_VERSION[\t ]+\".*\"")
    
    string(REGEX REPLACE "^#define[\t ]+SOCKET_VERSION[\t ]+\"([^\"]*)\".*" "\\1" SOCKET_VERSION_STRING "${soc_version_str}")
    unset(soc_version_str)
  endif()
endif()

#include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sockets
                                  REQUIRED_VARS SOCKETS_LIBRARY SOCKETS_INCLUDE_DIR
                                  VERSION_VAR SOCKETS_VERSION_STRING)

if(SOCKETS_FOUND)
  set(SOCKETS_LIBRARIES ${SOCKETS_LIBRARY})
  set(SOCKETS_INCLUDE_DIRS ${SOCKETS_INCLUDE_DIR})
endif()
