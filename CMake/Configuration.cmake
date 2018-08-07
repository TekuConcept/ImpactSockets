# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# COMPILER CONFIGURATION                                  #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}'")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
    STRING "Choose the type of build, options are: None;Debug;Release" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "None" "Debug" "Release")
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR
    CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wextra -pthread")
endif()

message(STATUS "C++ Flags:  ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ Debug:   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "C++ Release: ${CMAKE_CXX_FLAGS_RELEASE}")



# Set install directory to project root if building on Windows
if(MSVC)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Install" CACHE
      PATH "Prefix prepended to install directories" FORCE)
endif()

set(LIBRARIES_DIR       ${CMAKE_CURRENT_SOURCE_DIR}/Libraries)
set(SOCKETS_DIR         ${CMAKE_CURRENT_SOURCE_DIR}/Sockets)
set(SOCKETS_INCLUDE_DIR ${SOCKETS_DIR}/Include)
set(SOCKETS_SOURCE_DIR  ${SOCKETS_DIR}/Source)
set(EXAMPLES_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/Examples)
set(TESTS_DIR           ${CMAKE_CURRENT_SOURCE_DIR}/Tests)

if(${BUILD_SHARED})
    set(BUILD_MODE SHARED)
    set(BUILD_FLAGS -DDLL_EXPORTS)
else()
    set(BUILD_MODE STATIC)
    set(BUILD_FLAGS -DSTATIC)
endif()
