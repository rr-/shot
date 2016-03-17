cmake_minimum_required(VERSION 2.8.8)
project(shot)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")
set(CMAKE_SOURCE_DIR "${CMAKE_BINARY_DIR}/../")

# -------
# Version
# -------
execute_process(COMMAND git describe --tags --abbrev=0 OUTPUT_VARIABLE VERSION_SHORT OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND git describe --always --dirty --long --tags OUTPUT_VARIABLE VERSION_LONG OUTPUT_STRIP_TRAILING_WHITESPACE)
if("${VERSION_SHORT}" STREQUAL "")
    set(VERSION_SHORT "0.0")
    set(VERSION_LONG "?")
endif()

# ------------
# Dependencies
# ------------
find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})
link_directories(${PNG_LIBRARY_DIRS})

find_package(X11)
include_directories(${X11_INCLUDE_DIR})
link_directories(${X11_LIBRARY_DIRS})

find_package(Xrandr)
include_directories(${xrandr_INCLUDE_DIR})
link_directories(${xrandr_LIBRARY_DIRS})

find_package(GDI32)
include_directories(${GDI32_INCLUDE_DIR})
link_directories(${GDI32_LIBRARY_DIRS})

# -------------
# Build options
# -------------
if(CMAKE_COMPILER_IS_GNUC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pedantic-errors")
endif()
add_definitions(-D_POSIX_C_SOURCE=200809L) # needed for nanosleep()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c99") # mxe is still at cmake 3.0.2, can't use CMAKE_C_STANDARD

# ------------
# Source files
# ------------
file(READ "${CMAKE_SOURCE_DIR}/docs/help.txt" HELP HEX)
string(REGEX REPLACE "(..)" "\\\\x\\1" HELP "${HELP}")
configure_file("${CMAKE_SOURCE_DIR}/src/help.h.in" "${CMAKE_SOURCE_DIR}/src/help.h" @ONLY)
configure_file("${CMAKE_SOURCE_DIR}/src/version.h.in" "${CMAKE_SOURCE_DIR}/src/version.h" @ONLY)
file(GLOB_RECURSE sources
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.h)

foreach(path ${sources})
    if ("${path}" MATCHES "_win\\." AND NOT GDI32_FOUND)
        list(REMOVE_ITEM sources "${path}")
    endif()
    if ("${path}" MATCHES "_x11\\." AND NOT X11_FOUND)
        list(REMOVE_ITEM sources "${path}")
    endif()
endforeach()

add_executable(shot ${sources})
target_include_directories(shot BEFORE PUBLIC "${CMAKE_SOURCE_DIR}/src")

# ------------
# Installation
# ------------
install(FILES ${CMAKE_SOURCE_DIR}/docs/shot.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/man/man1)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/shot DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
add_custom_target(uninstall COMMAND
    rm -f "${CMAKE_INSTALL_PREFIX}/man/man1/shot.1" &&
    rm -f "${CMAKE_INSTALL_PREFIX}/bin/shot" )

# -------------------
# Linking definitions
# -------------------
target_link_libraries(shot ${PNG_LIBRARIES})
if(X11_FOUND)
    if(NOT XRANDR_FOUND)
        message(FATAL_ERROR "XRandR not found")
    endif()
    target_link_libraries(shot ${X11_LIBRARIES})
    target_link_libraries(shot ${XRANDR_LIBRARIES})
elseif(GDI32_FOUND)
    target_link_libraries(shot ${GDI32_LIBRARIES})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows") # disable console window
else()
    message(FATAL_ERROR "Neither X11 or Windows detected")
endif()
