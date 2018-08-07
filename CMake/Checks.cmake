include(CheckCXXSourceRuns)
#include(CheckVariableExists)
#include(CheckCXXSymbolExists)



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

STRING(REPLACE "TYPE" "uint16_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_UINT16_T)

STRING(REPLACE "TYPE" "uint32_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_UINT32_T)

STRING(REPLACE "TYPE" "uint64_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_UINT64_T)

STRING(REPLACE "TYPE" "int8_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_INT8_T)

STRING(REPLACE "TYPE" "int16_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_INT16_T)

STRING(REPLACE "TYPE" "int32_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_INT32_T)

STRING(REPLACE "TYPE" "int64_t" TYPE_CODE "${TYPE_CODE_TEMPLATE}")
CHECK_CXX_SOURCE_RUNS(${TYPE_CODE} HAVE_INT64_T)


# These types should be supported on just about every system that
# supports socket transactions and the C++11 standard, nevertheless,
# throw an error for those otherwise very rare circumstances when
# they are not.
if (NOT HAVE_UINT8_T  OR
    NOT HAVE_UINT16_T OR
    NOT HAVE_UINT32_T OR
    NOT HAVE_UINT64_T)
    message(FATAL_ERROR "One or more unsigned integer types not found")
endif()



# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# ENDIANNESS CHECK                                        #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

SET(ENDIAN_CODE_TEMPLATE "          \
#include <cstdint>                  \n\
int main(void) {                    \n\
  union {                           \n\
    uint32_t dword\;                \n\
    uint8_t byte[4]\;               \n\
  } value = { 0x01020304 }\;        \n\
return !(                           \n\
  (value.byte[0] == '\\\\TESTA') && \n\
  (value.byte[2] == '\\\\TESTB'))\; \n\
}                                   \
")


STRING(REPLACE "TESTA" "x01" ENDIAN_CODE "${ENDIAN_CODE_TEMPLATE}")
STRING(REPLACE "TESTB" "x03" ENDIAN_CODE "${ENDIAN_CODE}")
CHECK_CXX_SOURCE_RUNS(${ENDIAN_CODE} BIG_ENDIAN)

STRING(REPLACE "TESTA" "x04" ENDIAN_CODE "${ENDIAN_CODE_TEMPLATE}")
STRING(REPLACE "TESTB" "x02" ENDIAN_CODE "${ENDIAN_CODE}")
CHECK_CXX_SOURCE_RUNS(${ENDIAN_CODE} LITTLE_ENDIAN)


if (BIG_ENDIAN)
  add_definitions(-DARCH_BIG_ENDIAN)
elseif(LITTLE_ENDIAN)
  add_definitions(-DARCH_LITTLE_ENDIAN)
else()
  # Bit-Aligned and Word-Aligned Endianness Not Supported
  message(FATAL_ERROR "Architecture endianness not supported")
endif()
