# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Find GTest                                                                  #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

IF (MSVC)
    SET(CMAKE_PREFIX_PATH
        ${CMAKE_PREFIX_PATH}
        "C:/Program Files (x86)/googletest-distribution"
        "C:/Program Files/googletest-distribution"
    )
ENDIF ()
FIND_PACKAGE(GTest)
IF (GTEST_FOUND)
    STRING(REGEX REPLACE "gtest" "gmock" GMOCK_LIBRARIES ${GTEST_LIBRARIES})
    get_filename_component(GTEST_DIR "${GTEST_INCLUDE_DIRS}" DIRECTORY)
    MESSAGE(STATUS "Found GTest: ${GTEST_DIR}")
ELSE ()
    MESSAGE(SEND_ERROR "Cannot Find GTest - Some tests cannot be built")
ENDIF ()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Dependencies                                                                #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

SET(THIRD_PARTY_INCLUDE_DIRS "")
SET(THIRD_PARTY_LIBRARIES "")

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Find Npcap                                                                  #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

IF (WIN32) # This library is only necessary for Windows builds
    SET(NPCAP_ROOT_DIR "C:/npcap-sdk-0.1")
    FIND_PATH(NPCAP_INCLUDE_DIR NAMES pcap.h HINTS ${NPCAP_ROOT_DIR}/Include)
    IF ("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
        FIND_LIBRARY(NPCAP_LIBRARY NAMES wpcap HINTS ${NPCAP_ROOT_DIR/Lib/x64})
    ELSE ()
        FIND_LIBRARY(NPCAP_LIBRARY NAMES wpcap HINTS ${NPCAP_ROOT_DIR}/Lib)
    ENDIF ()
    IF (NPCAP_INCLUDE_DIR)
        get_filename_component(NPCAP_DIR "${NPCAP_INCLUDE_DIR}" DIRECTORY)
        MESSAGE(STATUS "Found Npcap: ${NPCAP_DIR}")
        SET(HAVE_NPCAP 1)
    ELSE ()
        MESSAGE(SEND_ERROR "Cannot Find Npcap - Used for raw sockets")
    ENDIF ()
ENDIF ()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Find libuv (libuv1-dev)                                                     #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

INCLUDE("${CMAKE_SOURCE_DIR}/CMake/FindLibUV.cmake")
IF (UV_LIBRARIES)
    LIST(APPEND THIRD_PARTY_INCLUDES ${UV_INCLUDE_DIRS})
    LIST(APPEND THIRD_PARTY_LIBRARIES ${UV_LIBRARIES})
    MESSAGE(STATUS "Found libuv: ${UV_INCLUDE_DIRS}")
ELSE ()
    MESSAGE(SEND_ERROR "libuv missing")
ENDIF ()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Find libgnutls (libgnutls28-dev)                                                     #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

INCLUDE("${CMAKE_SOURCE_DIR}/CMake/FindLibGnuTLS.cmake")
IF (GNUTLS_LIBRARIES)
    LIST(APPEND THIRD_PARTY_INCLUDES ${GNUTLS_INCLUDE_DIRS})
    LIST(APPEND THIRD_PARTY_LIBRARIES ${GNUTLS_LIBRARIES} nettle)
    MESSAGE(STATUS "Found libgnutls: ${GNUTLS_INCLUDE_DIRS}")
ELSE ()
    MESSAGE(SEND_ERROR "libgnutls missing")
ENDIF ()
