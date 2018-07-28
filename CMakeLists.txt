cmake_minimum_required (VERSION 3.0.2)
project(SocketsLibrary)

option(BUILD_TESTS "Build Complience Tests" ON)
#option(BUILD_SHARED "Build shared libraries instead of static ones." OFF)
set(BUILD_SHARED OFF) # not implemented right now
option(BUILD_EXAMPLES "Build the examples that demonstrate use-cases" ON)
#list(APPEND <items>)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Functions                                                                   #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

function(x_add_executable)
  #message("Executable Traget: ${ARGV0}")
  add_executable(${ARGV})
endfunction()

function(x_add_library)
  #message("Library Target: ${ARGV0}")
  add_library(${ARGV})
endfunction()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Find GTest                                                                  #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

if(BUILD_TESTS)
	if(MSVC)
	  set(CMAKE_PREFIX_PATH
	    ${CMAKE_PREFIX_PATH}
	    "C:/Program Files (x86)/googletest-distribution"
	    "C:/Program Files/googletest-distribution"
	  )
	endif()
	find_package(GTest)
	if(GTEST_FOUND)
	  string(REGEX REPLACE "gtest" "gmock" GMOCK_LIBRARIES ${GTEST_LIBRARIES})
	  get_filename_component(GTEST_DIR "${GTEST_INCLUDE_DIRS}" DIRECTORY)
	  message(STATUS "Found GTest: ${GTEST_DIR}")
	else()
	  message(SEND_ERROR "Cannot Find GTest - Some tests cannot be built")
	endif()
endif()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
#                 COMPILER CONFIGURATION                  #
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

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
#                      LIBRARY MODULES                    #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

include_directories(${SOCKETS_INCLUDE_DIR})
file(GLOB_RECURSE SOURCE_FILES "${SOCKETS_SOURCE_DIR}/*.cpp")
set(IMPACT_SOCKETS ImpactSockets)
x_add_library(${IMPACT_SOCKETS} ${BUILD_MODE} ${SOURCE_FILES})
target_compile_definitions(${IMPACT_SOCKETS} PUBLIC ${BUILD_FLAGS})

install(
  TARGETS ${IMPACT_SOCKETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
INSTALL (
  DIRECTORY ${SOCKETS_INCLUDE_DIR}/
  DESTINATION include/sockets
  FILES_MATCHING PATTERN "*.h*"
)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
#                      EXAMPLE MODULES                    #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

if(BUILD_EXAMPLES)
  set(EXAMPLES
    "DemoTCPClient"
    "DemoTCPServer"
    "Discovery"
  )
  foreach(EXAMPLE ${EXAMPLES})
		x_add_executable(${EXAMPLE} "${EXAMPLES_DIR}/${EXAMPLE}.cpp")
		target_link_libraries(${EXAMPLE} ${IMPACT_SOCKETS})
	endforeach()
endif(BUILD_EXAMPLES)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
#                       TEST MODULES                      #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

if(BUILD_TESTS)

  if(GTEST_FOUND)
    if(NOT MSVC)
      set(OTHER_LIBRARIES pthread)
    else()
      set(OTHER_LIBRARIES "")
    endif()
  
    set(UNIT_SETS
      "unit_set_1"
    )
    foreach(UNIT_SET ${UNIT_SETS})
      file(GLOB_RECURSE UNIT_FILES "${TESTS_DIR}/GUTs/${UNIT_SET}/*.cpp")
      x_add_executable(${UNIT_SET} ${UNIT_FILES} ${TESTS_DIR}/GUTs/main.cpp)
      target_include_directories(${UNIT_SET} PUBLIC ${GTEST_INCLUDE_DIRS})
      target_link_libraries(${UNIT_SET}
        ${GTEST_LIBRARIES}
        ${GMOCK_LIBRARIES}
        ${IMPACT_SOCKETS}
        ${OTHER_LIBRARIES}
      )
    endforeach()
  endif(GTEST_FOUND)

  set(SYSTEM_TESTS
		"test_basic_socket"
		"test_networking"
		"test_impact_error"
		"test_worker_thread"
		"test_async_pipeline"
	)
	foreach(SYSTEM_TEST ${SYSTEM_TESTS})
		x_add_executable(${SYSTEM_TEST} "${TESTS_DIR}/System/${SYSTEM_TEST}.cpp")
		target_link_libraries(${SYSTEM_TEST} ${IMPACT_SOCKETS})
	endforeach()
	
  target_compile_options(test_async_pipeline PRIVATE "-ggdb3")

endif(BUILD_TESTS)