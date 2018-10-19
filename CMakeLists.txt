CMAKE_MINIMUM_REQUIRED (VERSION 3.0.2)
PROJECT (SocketsLibrary)

SET(CMAKE_MODULES_DIR "${CMAKE_SOURCE_DIR}/CMake")

OPTION(BUILD_SYSTEM_TESTS "Build runtime tests" ON)
OPTION(BUILD_UNIT_TESTS "Built unit tests" ON)
#OPTION(BUILD_SHARED "Build shared libraries instead of static ones." OFF)
SET(BUILD_SHARED OFF) # not implemented right now
OPTION(BUILD_EXAMPLES "Build the examples that demonstrate use-cases" ON)

INCLUDE(${CMAKE_MODULES_DIR}/Checks.cmake)
INCLUDE(${CMAKE_MODULES_DIR}/Dependencies.cmake)
INCLUDE(${CMAKE_MODULES_DIR}/Functions.cmake)
INCLUDE(${CMAKE_MODULES_DIR}/Configuration.cmake)

CONFIGURE_FILE(${CMAKE_MODULES_DIR}/configure.h.in configure.h @ONLY)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# LIBRARY MODULES                                         #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

INCLUDE_DIRECTORIES(
    ${SOCKETS_INCLUDE_DIR}
    ${SOCKETS_INCLUDE_DIR}/placeholder
    ${CMAKE_CURRENT_BINARY_DIR} # location of configure.h
)
FILE(GLOB_RECURSE SOURCE_FILES "${SOCKETS_SOURCE_DIR}/*.cpp")
SET(IMPACT_SOCKETS ImpactSockets)
x_add_library(${IMPACT_SOCKETS} ${BUILD_MODE} ${SOURCE_FILES})
TARGET_COMPILE_DEFINITIONS(${IMPACT_SOCKETS} PUBLIC ${BUILD_FLAGS})

INSTALL (
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
INSTALL (
    DIRECTORY ${SOCKETS_INCLUDE_DIR}/placeholder/
    DESTINATION include/sockets
    FILES_MATCHING PATTERN "*"
)


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# EXAMPLE MODULES                                         #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

IF (BUILD_EXAMPLES)
    SET(EXAMPLES
        "DemoTCPClient"
        "DemoTCPServer"
        "Discovery"
        "ListInterfaces"
    )
    FOREACH (EXAMPLE ${EXAMPLES})
        x_add_executable(${EXAMPLE} "${EXAMPLES_DIR}/${EXAMPLE}.cpp")
        TARGET_LINK_LIBRARIES(${EXAMPLE} ${IMPACT_SOCKETS})
    ENDFOREACH ()
ENDIF ()



# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# TEST MODULES                                            #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

IF (BUILD_UNIT_TESTS)
    IF (GTEST_FOUND)
        IF (NOT MSVC)
            SET(OTHER_LIBRARIES pthread)
        ELSE ()
            SET(OTHER_LIBRARIES "")
        ENDIF ()

        SET(UNIT_SETS
            "unit_set_1"
        )
        FOREACH(UNIT_SET ${UNIT_SETS})
            FILE(GLOB_RECURSE UNIT_FILES "${TESTS_DIR}/GUTs/${UNIT_SET}/*.cpp")
            x_add_executable(${UNIT_SET} ${UNIT_FILES} ${TESTS_DIR}/GUTs/main.cpp)
            TARGET_INCLUDE_DIRECTORIES(${UNIT_SET} PUBLIC ${GTEST_INCLUDE_DIRS})
            TARGET_LINK_LIBRARIES(${UNIT_SET}
                ${GTEST_LIBRARIES}
                ${GMOCK_LIBRARIES}
                ${IMPACT_SOCKETS}
                ${OTHER_LIBRARIES}
            )
        ENDFOREACH ()
    ENDIF (GTEST_FOUND)
ENDIF ()

IF (BUILD_SYSTEM_TESTS)
    SET(SYSTEM_TESTS
        "test_basic_socket"
        "test_networking"
        "test_impact_error"
        "test_worker_thread"
        "test_async_pipeline"
    )
    FOREACH (SYSTEM_TEST ${SYSTEM_TESTS})
        x_add_executable(${SYSTEM_TEST} "${TESTS_DIR}/System/${SYSTEM_TEST}.cpp")
        TARGET_LINK_LIBRARIES(${SYSTEM_TEST} ${IMPACT_SOCKETS})
    ENDFOREACH ()
    IF (NOT MSVC)
        TARGET_COMPILE_OPTIONS(test_async_pipeline PRIVATE "-ggdb3")
    ENDIF ()
ENDIF ()
