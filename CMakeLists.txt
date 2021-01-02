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
    ${UV_INCLUDE_DIRS}
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
    FILE(GLOB_RECURSE DEMO_PROGRAMS "${CMAKE_SOURCE_DIR}/Examples/*.cpp")
    FOREACH (DEMO_PROGRAM ${DEMO_PROGRAMS})
        GET_FILENAME_COMPONENT(DEMO_NAME ${DEMO_PROGRAM} NAME_WE)
        ADD_EXECUTABLE(${DEMO_NAME} ${DEMO_PROGRAM})
        ADD_DEPENDENCIES(${DEMO_NAME} ${PROJECT_NAME}_obj)
        TARGET_LINK_LIBRARIES(${DEMO_NAME}
            $<TARGET_OBJECTS:${PROJECT_NAME}_obj>
            ${UV_LIBRARIES}
        )
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

SET(TESTS_DIR ${CMAKE_SOURCE_DIR}/Tests)

SET(BUILD_UNIT_TESTS ON CACHE BOOL "Build unit-tests")
SET(BUILD_SYSTEM_TESTS ON CACHE BOOL "Build system-tests")

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
            x_add_executable(${UNIT_SET} ${UNIT_FILES} "${TESTS_DIR}/GUTs/main.cpp")
            ADD_DEPENDENCIES(${UNIT_SET} ${PROJECT_NAME}_obj)
            TARGET_INCLUDE_DIRECTORIES(${UNIT_SET} PUBLIC ${GTEST_INCLUDE_DIRS})
            TARGET_LINK_LIBRARIES(${UNIT_SET}
                ${GTEST_LIBRARIES}
                ${GMOCK_LIBRARIES}
                ${SELECTED_LINK_TARGET}
                ${UV_LIBRARIES}
                ${OTHER_LIBRARIES}
            )
        ENDFOREACH ()
    ENDIF (GTEST_FOUND)
ENDIF ()

IF (BUILD_SYSTEM_TESTS)
    FILE(GLOB_RECURSE SYSTEM_TESTS "${TESTS_DIR}/System/test_*.cpp")
    FOREACH (SYSTEM_TEST ${SYSTEM_TESTS})
        GET_FILENAME_COMPONENT(SYSTEM_TEST_NAME ${SYSTEM_TEST} NAME_WE)
        ADD_EXECUTABLE(${SYSTEM_TEST_NAME} ${SYSTEM_TEST})
        TARGET_LINK_LIBRARIES(${SYSTEM_TEST_NAME}
            ${SELECTED_LINK_TARGET}
            ${UV_LIBRARIES}
            ${OTHER_LIBRARIES}
        )
    ENDFOREACH ()
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
