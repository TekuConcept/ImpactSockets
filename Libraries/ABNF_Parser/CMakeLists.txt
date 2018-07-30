project(ABNF)
cmake_minimum_required(VERSION 3.0.2)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Find GTest                                                                  #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

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

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Program                                                                     #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR
	CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror -Wextra")
endif()

include_directories("${CMAKE_SOURCE_DIR}/Include")
file(GLOB_RECURSE ABNF_SOURCE_FILES "${CMAKE_SOURCE_DIR}/Source/*.cpp")
add_executable(abnf ${ABNF_SOURCE_FILES} main.cpp)

file(GLOB_RECURSE UNIT_SOURCE_FILES "${CMAKE_SOURCE_DIR}/UnitTests/Units/*.cpp")
add_executable(unit_test ${UNIT_SOURCE_FILES} ${CMAKE_SOURCE_DIR}/UnitTests/guts.cpp)
target_include_directories(unit_test PUBLIC ${GTEST_INCLUDE_DIRS})
target_link_libraries(unit_test ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES})
