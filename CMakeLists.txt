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
FILE(GLOB_RECURSE SOC_SOURCE_FILES "${SOCKETS_SOURCE_DIR}/*.cpp")

x_add_library(${PROJECT_NAME}_obj OBJECT ${SOC_SOURCE_FILES})
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
            SOVERSION ${IMPACT_VERSION_MAJOR}
            VERSION ${IMPACT_VERSION_MAJOR}.${IMPACT_VERSION_MINOR}.${IMPACT_VERSION_PATCH}
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
                ${SELECTED_LINK_TARGET}
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
        TARGET_LINK_LIBRARIES(${SYSTEM_TEST} ${SELECTED_LINK_TARGET})
    ENDFOREACH ()
    IF (NOT MSVC)
        TARGET_COMPILE_OPTIONS(test_async_pipeline PRIVATE "-ggdb3")
    ENDIF ()
ENDIF ()



# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# LINUX DEBIAN PACKAGE BUILDER                            #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

IF (${__OS_LINUX__})
    MESSAGE(STATUS "Linux debian package option available.")

    SET(CPACK_GENERATOR "DEB")
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "TekuConcept")
    SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Light-weight network sockets, protocols, and utilities")
    SET(CPACK_PACKAGE_VENDOR "TekuConcept")
    SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.md")
    SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")

    SET(CPACK_PACKAGE_VERSION_MAJOR "${IMPACT_VERSION_MAJOR}")
    SET(CPACK_PACKAGE_VERSION_MINOR "${IMPACT_VERSION_MINOR}")
    SET(CPACK_PACKAGE_VERSION_PATCH "${IMPACT_VERSION_PATCH}")

    INCLUDE(CPack)
ENDIF ()
