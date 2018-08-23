cmake_minimum_required (VERSION 3.0.2)
project(SocketsLibrary)

SET(CMAKE_MODULES_DIR "${CMAKE_SOURCE_DIR}/CMake")

option(BUILD_SYSTEM_TESTS "Build runtime tests" ON)
option(BUILD_UNIT_TESTS "Built unit tests" ON)
#option(BUILD_SHARED "Build shared libraries instead of static ones." OFF)
set(BUILD_SHARED OFF) # not implemented right now
option(BUILD_EXAMPLES "Build the examples that demonstrate use-cases" ON)

include(${CMAKE_MODULES_DIR}/Checks.cmake)
include(${CMAKE_MODULES_DIR}/Dependencies.cmake)
include(${CMAKE_MODULES_DIR}/Functions.cmake)
include(${CMAKE_MODULES_DIR}/Configuration.cmake)



# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# LIBRARY MODULES                                         #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

include_directories(
    ${SOCKETS_INCLUDE_DIR}
    ${SOCKETS_INCLUDE_DIR}/placeholder
)
file(GLOB_RECURSE SOURCE_FILES "${SOCKETS_SOURCE_DIR}/*.cpp")
set(IMPACT_SOCKETS ImpactSockets)
x_add_library(${IMPACT_SOCKETS} ${BUILD_MODE} ${SOURCE_FILES})
target_compile_definitions(${IMPACT_SOCKETS} PUBLIC ${BUILD_FLAGS})

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
# TEST MODULES                                            #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

if(BUILD_UNIT_TESTS)
    if(GTEST_FOUND)
        if(NOT MSVC)
            set(OTHER_LIBRARIES pthread)
        else()
            set(OTHER_LIBRARIES "")
        endif()
  
        set(UNIT_SETS
            "unit_set_1"
            "unit_set_2"
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
endif()

if(BUILD_SYSTEM_TESTS)
    set(SYSTEM_TESTS
        "test_basic_socket"
        "test_networking"
        "test_impact_error"
        "test_worker_thread"
        "test_async_pipeline"
        "test_regex"
    )
    foreach(SYSTEM_TEST ${SYSTEM_TESTS})
        x_add_executable(${SYSTEM_TEST} "${TESTS_DIR}/System/${SYSTEM_TEST}.cpp")
        target_link_libraries(${SYSTEM_TEST} ${IMPACT_SOCKETS})
    endforeach()
    target_compile_options(test_async_pipeline PRIVATE "-ggdb3")
endif()
