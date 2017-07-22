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
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wextra -lpthread")
endif()

# Set install directory to project root if building on Windows
if(MSVC)
	set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Install" CACHE PATH
		"Prefix prepended to install directories" FORCE)
endif()

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

include_directories(${SOCKETS_INCLUDE_DIR})
file(GLOB_RECURSE SOURCE_FILES "${SOCKETS_PATH}/Source/*.cpp")
add_library(${LIBRARY_NAME} ${BUILD_MODE} ${SOURCE_FILES})
target_compile_definitions(${LIBRARY_NAME} PUBLIC ${BUILD_FLAGS})

if(BUILD_EXAMPLES)
	set(DEMO_TCP_CLIENT DemoTCPClient)
	add_executable(${DEMO_TCP_CLIENT} "${EXAMPLES_PATH}/${DEMO_TCP_CLIENT}.cpp")
	target_link_libraries(${DEMO_TCP_CLIENT} ${LIBRARY_NAME})

	set(DEMO_TCP_SERVER DemoTCPServer)
	add_executable(${DEMO_TCP_SERVER} "${EXAMPLES_PATH}/${DEMO_TCP_SERVER}.cpp")
	target_link_libraries(${DEMO_TCP_SERVER} ${LIBRARY_NAME})

	set(DEMO_WS_CLIENT DemoWSClient)
	add_executable(${DEMO_WS_CLIENT} "${EXAMPLES_PATH}/${DEMO_WS_CLIENT}.cpp")
	target_link_libraries(${DEMO_WS_CLIENT} ${LIBRARY_NAME})
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
  DESTINATION include
  FILES_MATCHING PATTERN "*.h*"
)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# BUILD TESTS                                                                 #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

if(BUILD_TESTS AND GTEST_FOUND)
  include_directories(
    ${GTEST_INCLUDE_DIRS}
    "Tests/Mocks"
  )

  file(GLOB_RECURSE TEST_FILES "${SOCKETS_PATH}/Tests/Cases/*.cpp")
  set(TESTS Tests)
  add_executable(${TESTS} ${TEST_FILES} ${SOCKETS_PATH}/Tests/main.cpp)
  target_link_libraries(${TESTS} ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES}
  	${LIBRARY_NAME})

  install_targets(/bin ${TESTS})
endif()