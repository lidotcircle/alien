cmake_minimum_required(VERSION 3.4)

project(alien)
set(CMAKE_PROJECT_VERSION 0.7.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CheckIncludeFile)
CHECK_INCLUDE_FILE(alloca.h HAVE_ALLOCA_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include("cmake/lua.cmake")
include("cmake/libffi.cmake")

set(alien_lua ${CMAKE_CURRENT_SOURCE_DIR}/src/alien.lua)
add_library(alien SHARED)
set_target_properties(alien PROPERTIES
    OUTPUT_NAME alien_c
    PREFIX "")
target_sources(alien PRIVATE src/alien.c ${alien_lua})
target_include_directories(alien PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(alien PRIVATE lua)

link_libffi(alien)
add_custom_command(TARGET alien POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${alien_lua} $<TARGET_FILE_DIR:alien>
    )


set(alientest_lua ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_alien.lua)
add_library(alientest SHARED tests/alientest.c)
if (NOT WIN32)
target_link_libraries(alientest m)
endif()
add_custom_command(TARGET alientest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${alientest_lua} $<TARGET_FILE_DIR:alientest>
    )

