# SPDX-License-Identifier: Apache-2.0

set_ifndef(CC gcc)

message(STATUS "TOOLCHAIN_HOME: $ENV{TOOLCHAIN_HOME}")
set(TOOLCHAIN_HOME $ENV{TOOLCHAIN_HOME})

find_program(CMAKE_C_COMPILER ${CROSS_COMPILE}${CC}   PATH /home/maxu/programs/BullseyeCoverage/bin NO_DEFAULT_PATH)

if(CMAKE_C_COMPILER STREQUAL CMAKE_C_COMPILER-NOTFOUND)
  message(FATAL_ERROR "Zephyr was unable to find the toolchain. Is the environment misconfigured?
User-configuration:
ZEPHYR_TOOLCHAIN_VARIANT: ${ZEPHYR_TOOLCHAIN_VARIANT}
Internal variables:
CROSS_COMPILE: ${CROSS_COMPILE}
TOOLCHAIN_HOME: ${TOOLCHAIN_HOME}
")
endif()

execute_process(
  COMMAND ${CMAKE_C_COMPILER} --version
  RESULT_VARIABLE ret
  OUTPUT_QUIET
  ERROR_QUIET
  )
if(ret)
  message(FATAL_ERROR "Executing the below command failed. Are permissions set correctly?
'${CMAKE_C_COMPILER} --version'
"
    )
endif()
