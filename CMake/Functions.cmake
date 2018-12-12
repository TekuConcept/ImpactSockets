# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Functions                                                                   #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

FUNCTION (x_add_executable)
    # MESSAGE("Executable Traget: ${ARGV0}")
    ADD_EXECUTABLE(${ARGV})
ENDFUNCTION ()

FUNCTION (x_add_library)
    # MESSAGE("Library Target: ${ARGV0}")
    ADD_LIBRARY(${ARGV})
ENDFUNCTION ()
