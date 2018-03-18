cmake_minimum_required (VERSION 3.0.2)
project(SocketsLibrary)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Find GTest                                                                  #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

option(BUILD_TESTS "Build Complience Tests" OFF)
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
	  message(SEND_ERROR "Cannot Find GTest")
	endif()
endif()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# COMPILER CONFIGURATION                                                      #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR
	CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wextra -O0 -g")
endif()

if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")

  set(CompilerFlags
    CMAKE_CXX_FLAGS
    CMAKE_CXX_FLAGS_DEBUG
    CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG
    CMAKE_C_FLAGS_RELEASE
  )
  foreach(CompilerFlag ${CompilerFlags})
    string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
  endforeach()

  # Set install directory to project root if building on Windows
  set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Install" CACHE PATH
		"Prefix prepended to install directories" FORCE)
endif()

set(LIBRARIES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Libraries)
set(EVENTS_INCLUDE_DIR ${LIBRARIES_PATH}/Events/Include)
set(SOCKETS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Sockets)
set(SOCKETS_INCLUDE_DIR ${SOCKETS_PATH}/Include)
set(EXAMPLES_PATH ${CMAKE_CURRENT_SOURCE_DIR}/Examples)

option(BUILD_EXAMPLES "Build all demos using the library" ON)
option(BUILD_SHARED "Build shared libraries instead of static ones." OFF)
if(${BUILD_SHARED})
	set(BUILD_MODE SHARED)
	set(BUILD_FLAGS -DDLL_EXPORTS)
else()
	set(BUILD_MODE STATIC)
	set(BUILD_FLAGS -DSTATIC)
endif()

set(LIBRARY_NAME ImpactSockets)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# PROJECT MODULES                                                             #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

include_directories(
  ${EVENTS_INCLUDE_DIR}
  ${SOCKETS_INCLUDE_DIR}
)
file(GLOB_RECURSE SOURCE_FILES "${SOCKETS_PATH}/Source/*.cpp")
add_library(${LIBRARY_NAME} ${BUILD_MODE} ${SOURCE_FILES})
target_compile_definitions(${LIBRARY_NAME} PUBLIC ${BUILD_FLAGS})

if(BUILD_EXAMPLES)
	set(EXAMPLE_PROGRAMS
		"DemoTCPClient"
		"DemoTCPServer"
		"DemoWebsocketClient"
		"DemoWebsocketServer"
		"DisconnectingSocket"
	)
	foreach(PROGRAM ${EXAMPLE_PROGRAMS})
		add_executable(${PROGRAM} "${EXAMPLES_PATH}/${PROGRAM}.cpp")
		target_link_libraries(${PROGRAM} ${LIBRARY_NAME})
	endforeach()
	if(NOT MSVC)
		TARGET_LINK_LIBRARIES(DemoWebsocketServer pthread)
		TARGET_LINK_LIBRARIES(DisconnectingSocket pthread)
	endif()
endif()

set(TARGETS
	${LIBRARY_NAME}
	${DEMO_TCP_CLIENT}
	${DEMO_TCP_SERVER}
	${DEMO_WS_CLIENT}
)

install(
  TARGETS ${TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
)
INSTALL (
  DIRECTORY ${SOCKETS_INCLUDE_DIR}/
  DESTINATION include/sockets
  FILES_MATCHING PATTERN "*.h*"
)
INSTALL (FILES
	"${SOCKETS_INCLUDE_DIR}/RFC/2616"
	"${SOCKETS_INCLUDE_DIR}/RFC/3174"
	"${SOCKETS_INCLUDE_DIR}/RFC/4648"
	"${SOCKETS_INCLUDE_DIR}/RFC/6455"
	DESTINATION include/sockets/RFC
)

INSTALL (
  DIRECTORY ${EVENTS_INCLUDE_DIR}/
  DESTINATION include/sockets
  FILES_MATCHING PATTERN "*.h"
)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# BUILD TESTS                                                                 #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

if(BUILD_TESTS AND GTEST_FOUND)
  include_directories(
    ${GTEST_INCLUDE_DIRS}
    "${SOCKETS_PATH}/Tests/Include"
  )

  file(GLOB_RECURSE TEST_FILES "${SOCKETS_PATH}/Tests/Cases/*.cpp")
  set(TESTS Tests)
  add_executable(${TESTS} ${TEST_FILES} ${SOCKETS_PATH}/Tests/main.cpp)
  target_link_libraries(${TESTS} ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES}
  	${LIBRARY_NAME})
  if(NOT MSVC)
  	target_link_libraries(${TESTS} pthread)
  endif()
  
  file(GLOB_RECURSE SYSTEM_TESTS "${SOCKETS_PATH}/Tests/System/*.cpp")
  foreach(PROGRAM ${SYSTEM_TESTS})
  	get_filename_component(PROGRAM_FILE ${PROGRAM} NAME)
  	string(REGEX REPLACE "\\.[^.]*$" "" PROGRAM_EXT ${PROGRAM_FILE})
		add_executable(${PROGRAM_EXT} ${PROGRAM})
		target_link_libraries(${PROGRAM_EXT} ${LIBRARY_NAME})
		if(NOT MSVC)
			target_link_libraries(${PROGRAM_EXT} pthread)
		endif()
	endforeach()

  install_targets(/bin ${TESTS})
endif()
