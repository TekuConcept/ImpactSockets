
include(CheckCXXSourceRuns)
include(CheckCXXSourceCompiles)
#include(CheckVariableExists)
#include(CheckCXXSymbolExists)

IF (UNIX)
    SET(TMP_REQ_FLAGS "${CMAKE_REQUIRED_FLAGS}")
    SET(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -std=c++11 -Wall -Werror -Wextra")
ENDIF (UNIX)

GET_DIRECTORY_PROPERTY(CMAKE_HAS_PARENT_SCOPE PARENT_DIRECTORY)
IF (CMAKE_HAS_PARENT_SCOPE)
    SET(SCOPE PARENT_SCOPE)
ELSE ()
    SET(SCOPE "")
ENDIF ()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# SYSTEM CHECKS                                           #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

IF (UNIX AND NOT APPLE)
    SET(__OS_LINUX__ 1 ${SCOPE})
ELSEIF (APPLE)
    SET(__OS_APPLE__ 1 ${SCOPE})
ELSEIF (WIN32 OR MSVC OR MSYS OR MINGW)
    SET(__OS_WINDOWS__ 1 ${SCOPE})
ENDIF ()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# TYPE CHECKS                                             #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

SET(TYPE_CODE_TEMPLATE "\
#include <cstdint>      \n\
TYPE n\;                \n\
int main() {return 0\;} \
")

STRING(REPLACE "TYPE" "uint8_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_COMPILES(${TYPE_CODE} HAVE_UINT8_T)
SET(HAVE_UINT8_T ${HAVE_UINT8_T} ${SCOPE})

STRING(REPLACE "TYPE" "uint16_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_COMPILES(${TYPE_CODE} HAVE_UINT16_T)
SET(HAVE_UINT16_T ${HAVE_UINT16_T} ${SCOPE})

STRING(REPLACE "TYPE" "uint32_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_COMPILES(${TYPE_CODE} HAVE_UINT32_T)
SET(HAVE_UINT32_T ${HAVE_UINT32_T} ${SCOPE})

STRING(REPLACE "TYPE" "uint64_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_COMPILES(${TYPE_CODE} HAVE_UINT64_T)
SET(HAVE_UINT64_T ${HAVE_UINT64_T} ${SCOPE})

STRING(REPLACE "TYPE" "int8_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_COMPILES(${TYPE_CODE} HAVE_INT8_T)
SET(HAVE_INT8_T ${HAVE_INT8_T} ${SCOPE})

STRING(REPLACE "TYPE" "int16_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_COMPILES(${TYPE_CODE} HAVE_INT16_T)
SET(HAVE_INT16_T ${HAVE_INT16_T} ${SCOPE})

STRING(REPLACE "TYPE" "int32_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_COMPILES(${TYPE_CODE} HAVE_INT32_T)
SET(HAVE_INT32_T ${HAVE_INT32_T} ${SCOPE})

STRING(REPLACE "TYPE" "int64_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_COMPILES(${TYPE_CODE} HAVE_INT64_T)
SET(HAVE_INT64_T ${HAVE_INT64_T} ${SCOPE})

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# ENDIANNESS CHECK                                        #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

IF (NOT CMAKE_CROSSCOMPILING)

    SET(ENDIAN_CODE_TEMPLATE "            \
    int main(void) {                      \n\
        union {                           \n\
            unsigned long int dword\;     \n\
            unsigned char byte[4]\;       \n\
        } value = { 0x01020304 }\;        \n\
        return !(                         \n\
        (value.byte[0] == TESTA) &&       \n\
        (value.byte[2] == TESTB))\;       \n\
    }                                     \
    ")

    STRING(REPLACE "TESTA" "0x01" ENDIAN_CODE "${ENDIAN_CODE_TEMPLATE}")
    STRING(REPLACE "TESTB" "0x03" ENDIAN_CODE "${ENDIAN_CODE}")
    CHECK_CXX_SOURCE_RUNS(${ENDIAN_CODE} BIG_ENDIAN)
    SET(BIG_ENDIAN ${BIG_ENDIAN} ${SCOPE})

    STRING(REPLACE "TESTA" "0x04" ENDIAN_CODE "${ENDIAN_CODE_TEMPLATE}")
    STRING(REPLACE "TESTB" "0x02" ENDIAN_CODE "${ENDIAN_CODE}")
    CHECK_CXX_SOURCE_RUNS(${ENDIAN_CODE} LITTLE_ENDIAN)
    SET(LITTLE_ENDIAN ${LITTLE_ENDIAN} ${SCOPE})

    IF (NOT BIG_ENDIAN AND NOT LITTLE_ENDIAN)
        # Bit-Aligned and Word-Aligned Endianness Not Supported
        MESSAGE(FATAL_ERROR "Architecture endianness not supported")
    ENDIF()

ELSE()

    # cannot always run cross-compiled programs on host
    # so convert this automated check into a manual one
    # NOTE: C++20 now supports endianness checking at compile-time
    # TODO: add compile-time checking for supported compilers
    SET(BIG_ENDIAN OFF CACHE BOOL "architecture endianness")
    IF (BIG_ENDIAN)
        SET(LITTLE_ENDIAN OFF)
    ELSE()
        SET(LITTLE_ENDIAN ON)
    ENDIF()

ENDIF()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# FEATURE CHECK                                           #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

CHECK_CXX_SOURCE_COMPILES(" \
int main(void) {        \n\
    int x = 0;          \n\
    switch (x) {        \n\
        case 0: x++;    \n\
        [[fallthrough]];\n\
        default: break; \n\
    }                   \n\
    return x - 1;       \n\
}                       \
" HAVE_FALLTHROUGH_ATTRIBUTE)
SET(HAVE_FALLTHROUGH_ATTRIBUTE ${HAVE_FALLTHROUGH_ATTRIBUTE} ${SCOPE})


IF (UNIX)
    SET(CMAKE_REQUIRED_FLAGS "${TMP_REQ_FLAGS}")
ENDIF (UNIX)
