# Find gdi32 header and library for wingcc driver
#

# This module defines the following uncached variables:
#  GDI32_FOUND, if false, do not try to use gdi32.
#  GDI32_LIBRARIES, the libraries to link against to use gdi32

# Borland compiler doesn't know the gdi32 library
IF(NOT BORLAND)
  set(GDI32_LIBRARY gdi32)
ENDIF(NOT BORLAND)

try_compile(TESTGDI32
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/cmake/FindGDI32.c
  CMAKE_FLAGS -DLINK_LIBRARIES=${GDI32_LIBRARY}
  OUTPUT_VARIABLE OUTPUT)
if(TESTGDI32)
  set(GDI32_FOUND ON)
  set(GDI32_LIBRARIES ${GDI32_LIBRARY})
  file(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log
  "Determining if gdi32 is available passed with "
  "the following output:\n${OUTPUT}\n\n")
else(TESTGDI32)
  set(GDI32_FOUND OFF)
  set(GDI32_LIBRARIES "")
  file(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
  "Determining if gdi32 is available failed with "
  "the following output:\n${OUTPUT}\n\n")
endif(TESTGDI32)

if(GDI32_FOUND)
  if(NOT GDI32_FIND_QUIETLY)
    message(STATUS "FindGDI32: Found gdi32 header file and library")
  endif(NOT GDI32_FIND_QUIETLY)
else(GDI32_FOUND)
  if(GDI32_FIND_REQUIRED)
    message(FATAL_ERROR "FindGDI32: Could not find gdi32 header file and/or library")
  endif(GDI32_FIND_REQUIRED)
endif(GDI32_FOUND)
