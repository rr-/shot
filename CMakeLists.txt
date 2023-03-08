cmake_minimum_required(VERSION 3.6 FATAL_ERROR)

# -------------------
# Version and project
# -------------------
find_package(Git)
if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.git" AND Git_FOUND)
    execute_process(COMMAND
        "${GIT_EXECUTABLE}" describe --always --dirty --long --tags
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE SHOT_VERSION_LONG OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
if(NOT SHOT_VERSION_LONG)
    set(SHOT_VERSION_LONG "0.0")
endif()
string(REGEX REPLACE "-.*" "" SHOT_VERSION_SHORT "${SHOT_VERSION_LONG}")
project(shot VERSION ${SHOT_VERSION_SHORT} LANGUAGES C)

# ------------
# Dependencies
# ------------
find_package(PNG REQUIRED)

if(WIN32)
    set(target "win")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11 xrandr)
    set(target "x11")
endif()

# ---------------------------
# Source files and executable
# ---------------------------
file(READ docs/help.txt HELP HEX)
string(REGEX REPLACE "(..)" "\\\\x\\1" HELP "${HELP}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/help.h"
"#ifndef HELP_H\n"
"#define HELP_H\n"
"static const char *help_str = \"${HELP}\";\n"
"#endif\n")

add_executable(shot WIN32 # disable console window on windows
    src/bitmap.c src/bitmap.h
    src/bitmap_${target}.c
    src/grab.h
    src/grab_${target}.c
    src/monitor.c src/monitor.h
    src/monitor_mgr.c src/monitor_mgr.h
    src/monitor_mgr_${target}.c
    src/region.h
    src/shot.c
    src/region_picker/active_monitor.c src/region_picker/active_monitor.h
    src/region_picker/active_window_${target}.c
    src/region_picker/active_window.h
    src/region_picker/errors.h
    src/region_picker/interactive_${target}.c
    src/region_picker/interactive.h
    src/region_picker/interactive_common.c
    src/region_picker/interactive_common.h
    src/region_picker/monitor.c src/region_picker/monitor.h
    src/region_picker/string.c src/region_picker/string.h
    src/region_picker/window_${target}.c
    src/region_picker/window.h
    "${CMAKE_CURRENT_BINARY_DIR}/help.h")

# -------------
# Build options
# -------------
target_include_directories(shot PRIVATE src "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(shot PROPERTIES
    C_STANDARD 99 C_STANDARD_REQUIRED ON C_EXTENSIONS OFF)
target_compile_definitions(shot PRIVATE SHOT_VERSION="${SHOT_VERSION_LONG}"
    $<$<NOT:$<BOOL:${WIN32}>>:_POSIX_C_SOURCE=200809L>) # needed for nanosleep()
include(CheckCCompilerFlag)
foreach(FLAG IN ITEMS Wall Wextra pedantic-errors)
    string(TOUPPER "${FLAG}" VARIABLE)
    string(REPLACE "-" "_" VARIABLE "${VARIABLE}")
    check_c_compiler_flag("-${FLAG}" "${VARIABLE}")
    if("${${VARIABLE}}")
        target_compile_options(shot PRIVATE "-${FLAG}")
    endif()
endforeach()

# -------------------
# Linking definitions
# -------------------
target_link_libraries(shot PRIVATE PNG::PNG
    $<$<NOT:$<BOOL:${WIN32}>>:PkgConfig::X11>)
if(WIN32)
    set(CMAKE_C_CREATE_WIN32_EXE "-mconsole") # overwrite "-mwindows"
endif()

# ------------
# Installation
# ------------
include(GNUInstallDirs)
install(TARGETS shot DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}")
install(FILES docs/shot.1 DESTINATION "${CMAKE_INSTALL_FULL_MANDIR}/man1")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
"file(REMOVE \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_BINDIR}/shot\"
    \"\$ENV{DESTDIR}${CMAKE_INSTALL_FULL_MANDIR}/man1/shot.1\")")
add_custom_target(uninstall COMMAND "${CMAKE_COMMAND}" -P cmake_uninstall.cmake)
