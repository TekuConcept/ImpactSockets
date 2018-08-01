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
