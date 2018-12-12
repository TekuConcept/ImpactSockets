include(CheckCXXSourceRuns)
#include(CheckVariableExists)
#include(CheckCXXSymbolExists)

SET(TMP_REQ_FLAGS "${CMAKE_REQUIRED_FLAGS}")
SET(CMAKE_REQUIRED_FLAGS
    "${CMAKE_REQUIRED_FLAGS} -std=c++11 -Wall -Werror -Wextra")
get_directory_property(CMAKE_HAS_PARENT_SCOPE PARENT_DIRECTORY)
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
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_UINT8_T)
SET(HAVE_UINT8_T ${HAVE_UINT8_T} ${SCOPE})

STRING(REPLACE "TYPE" "uint16_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_UINT16_T)
SET(HAVE_UINT16_T ${HAVE_UINT16_T} ${SCOPE})

STRING(REPLACE "TYPE" "uint32_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_UINT32_T)
SET(HAVE_UINT32_T ${HAVE_UINT32_T} ${SCOPE})

STRING(REPLACE "TYPE" "uint64_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_UINT64_T)
SET(HAVE_UINT64_T ${HAVE_UINT64_T} ${SCOPE})

STRING(REPLACE "TYPE" "int8_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_INT8_T)
SET(HAVE_INT8_T ${HAVE_INT8_T} ${SCOPE})

STRING(REPLACE "TYPE" "int16_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_INT16_T)
SET(HAVE_INT16_T ${HAVE_INT16_T} ${SCOPE})

STRING(REPLACE "TYPE" "int32_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_INT32_T)
SET(HAVE_INT32_T ${HAVE_INT32_T} ${SCOPE})

STRING(REPLACE "TYPE" "int64_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_INT64_T)
SET(HAVE_INT64_T ${HAVE_INT64_T} ${SCOPE})

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# ENDIANNESS CHECK                                        #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

SET(ENDIAN_CODE_TEMPLATE "            \
#include <cstdint>                    \n\
int main(void) {                      \n\
    union {                           \n\
        uint32_t dword\;              \n\
        uint8_t byte[4]\;             \n\
    } value = { 0x01020304 }\;        \n\
return !(                             \n\
    (value.byte[0] == '\\\\TESTA') && \n\
    (value.byte[2] == '\\\\TESTB'))\; \n\
}                                     \
")

STRING(REPLACE "TESTA" "x01" ENDIAN_CODE "${ENDIAN_CODE_TEMPLATE}")
STRING(REPLACE "TESTB" "x03" ENDIAN_CODE "${ENDIAN_CODE}")
CHECK_CXX_SOURCE_RUNS(${ENDIAN_CODE} BIG_ENDIAN)
SET(BIG_ENDIAN ${BIG_ENDIAN} ${SCOPE})

STRING(REPLACE "TESTA" "x04" ENDIAN_CODE "${ENDIAN_CODE_TEMPLATE}")
STRING(REPLACE "TESTB" "x02" ENDIAN_CODE "${ENDIAN_CODE}")
CHECK_CXX_SOURCE_RUNS(${ENDIAN_CODE} LITTLE_ENDIAN)
SET(LITTLE_ENDIAN ${LITTLE_ENDIAN} ${SCOPE})

IF (NOT BIG_ENDIAN AND NOT LITTLE_ENDIAN)
    # Bit-Aligned and Word-Aligned Endianness Not Supported
    MESSAGE(FATAL_ERROR "Architecture endianness not supported")
ENDIF()

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# FEATURE CHECK                                           #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

CHECK_CXX_SOURCE_RUNS(" \
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


SET(CMAKE_REQUIRED_FLAGS "${TMP_REQ_FLAGS}")
