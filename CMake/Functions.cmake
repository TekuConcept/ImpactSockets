# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #
# Functions                                                                   #
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - #

function(x_add_executable)
  #message("Executable Traget: ${ARGV0}")
  add_executable(${ARGV})
endfunction()

function(x_add_library)
  #message("Library Target: ${ARGV0}")
  add_library(${ARGV})
endfunction()
