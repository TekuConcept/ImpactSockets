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
