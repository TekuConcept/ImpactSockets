CMAKE_MINIMUM_REQUIRED (VERSION 3.0.2)
PROJECT (ImpactSockets)

SET(CMAKE_MODULES_DIR "${CMAKE_SOURCE_DIR}/CMake")

OPTION(BUILD_SYSTEM_TESTS "Build runtime tests" ON)
OPTION(BUILD_UNIT_TESTS "Built unit tests" ON)
OPTION(BUILD_EXAMPLES "Build the examples that demonstrate use-cases" ON)

INCLUDE(${CMAKE_MODULES_DIR}/Checks.cmake)
INCLUDE(${CMAKE_MODULES_DIR}/Dependencies.cmake)
INCLUDE(${CMAKE_MODULES_DIR}/Functions.cmake)
INCLUDE(${CMAKE_MODULES_DIR}/Configuration.cmake)

CONFIGURE_FILE(${CMAKE_MODULES_DIR}/configure.h.in utils/configure.h @ONLY)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# LIBRARY MODULES                                         #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

INCLUDE_DIRECTORIES(
    ${SOCKETS_INCLUDE_DIR}
    ${SOCKETS_INCLUDE_DIR}/placeholder
    ${NPCAP_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR} # location of configure.h
)
FILE(GLOB_RECURSE SOCKET_SOURCE_FILES "${SOCKETS_SOURCE_DIR}/*.cpp")

x_add_library(${PROJECT_NAME}_obj OBJECT ${SOCKET_SOURCE_FILES})
SET_PROPERTY(TARGET ${PROJECT_NAME}_obj PROPERTY POSITION_INDEPENDENT_CODE 1)


IF (${BUILD_SHARED})
    ADD_LIBRARY(${PROJECT_NAME}_shared SHARED
        $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}_shared PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME})
    IF (UNIX AND NOT APPLE)
        # Setup so versioning for Linux systems
        SET_TARGET_PROPERTIES(${PROJECT_NAME}_shared
            PROPERTIES
            SOVERSION 0
            VERSION 0.1
        )
    ENDIF ()
    IF (HAVE_NPCAP)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME}_shared ${NPCAP_LIBRARY})
    ENDIF ()
    INSTALL (
        TARGETS ${PROJECT_NAME}_shared
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
ENDIF ()

IF (${BUILD_STATIC})
    ADD_LIBRARY(${PROJECT_NAME}_static STATIC
        $<TARGET_OBJECTS:${PROJECT_NAME}_obj>)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}_static PROPERTIES
        OUTPUT_NAME ${PROJECT_NAME})
    IF (HAVE_NPCAP)
        TARGET_LINK_LIBRARIES(${PROJECT_NAME}_static ${NPCAP_LIBRARY})
    ENDIF ()
    INSTALL (
        TARGETS ${PROJECT_NAME}_static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
ENDIF ()

IF (${BUILD_STATIC})
    SET(SELECTED_LINK_TARGET ${PROJECT_NAME}_static)
ELSEIF (${BUILD_SHARED})
    SET(SELECTED_LINK_TARGET ${PROJECT_NAME}_shared)
ELSE ()
    MESSAGE(FATAL_ERROR "Must build at least one of state or shared libraries")
ENDIF ()

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
INSTALL (
    FILES ${CMAKE_CURRENT_BINARY_DIR}/utils/configure.h
    DESTINATION include/sockets/utils
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
        "EthernetDemo"
    )
    FOREACH (EXAMPLE ${EXAMPLES})
        x_add_executable(${EXAMPLE} "${EXAMPLES_DIR}/${EXAMPLE}.cpp")
        TARGET_LINK_LIBRARIES(${EXAMPLE} ${SELECTED_LINK_TARGET})
    ENDFOREACH ()
ENDIF ()



# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# TEST MODULES                                            #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

FUNCTION(SUBDIRLIST out_var curdir)
    FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
    SET(result)
    FOREACH(child ${children})
        IF(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND result ${child})
        ENDIF()
    ENDFOREACH()
    SET(${out_var} "${result}" PARENT_SCOPE)
ENDFUNCTION()

IF (BUILD_UNIT_TESTS)
    IF (GTEST_FOUND)
        IF (NOT MSVC)
            SET(OTHER_LIBRARIES pthread)
        ELSE ()
            SET(OTHER_LIBRARIES "")
        ENDIF ()
        SUBDIRLIST(UNIT_SETS ${TESTS_DIR}/GUTs)
        FOREACH(UNIT_SET ${UNIT_SETS})
            FILE(GLOB_RECURSE UNIT_FILES "${TESTS_DIR}/GUTs/${UNIT_SET}/*.cpp")
            ADD_EXECUTABLE(${UNIT_SET} ${UNIT_FILES} ${TESTS_DIR}/GUTs/main.cpp)
            ADD_DEPENDENCIES(${UNIT_SET} ${PROJECT_NAME}_obj)
            TARGET_INCLUDE_DIRECTORIES(${UNIT_SET} PUBLIC ${GTEST_INCLUDE_DIRS})
            TARGET_LINK_LIBRARIES(${UNIT_SET}
                ${GTEST_LIBRARIES}
                ${GMOCK_LIBRARIES}
                ${SELECTED_LINK_TARGET}
                ${OTHER_LIBRARIES}
            )
        ENDFOREACH ()
    ENDIF (GTEST_FOUND)
ENDIF ()

IF (BUILD_SYSTEM_TESTS)
    IF (NOT MSVC)
        SET(OTHER_LIBRARIES pthread)
    ELSE ()
        SET(OTHER_LIBRARIES "")
    ENDIF ()
    FILE(GLOB_RECURSE SYSTEM_TESTS "${TESTS_DIR}/System/test_*.cpp")
    FOREACH (SYSTEM_TEST ${SYSTEM_TESTS})
    GET_FILENAME_COMPONENT(SYSTEM_TEST_NAME ${SYSTEM_TEST} NAME_WE)
        ADD_EXECUTABLE(${SYSTEM_TEST_NAME} ${SYSTEM_TEST})
        ADD_DEPENDENCIES(${SYSTEM_TEST_NAME} ${PROJECT_NAME}_obj)
        TARGET_LINK_LIBRARIES(${SYSTEM_TEST_NAME} ${SELECTED_LINK_TARGET})
    ENDFOREACH ()
    IF (NOT MSVC)
        TARGET_COMPILE_OPTIONS(test_async_pipeline PRIVATE "-ggdb3")
    ENDIF ()
ENDIF ()
