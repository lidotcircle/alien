cmake_minimum_required(VERSION 3.4)

project(alien)
set(CMAKE_PROJECT_VERSION 0.7.1)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CheckIncludeFile)
CHECK_INCLUDE_FILE(alloca.h HAVE_ALLOCA_H)
CHECK_INCLUDE_FILE(stdint.h HAVE_STDINT_H)
configure_file(${CMAKE_CURRENT_LIST_DIR}/src/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

if(WIN32)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

include("cmake/lua.cmake")
include("cmake/libffi.cmake")
add_subdirectory("thirdparty/funchook/")
function(link_funchook target type)
    string(TOLOWER ${type} typelc)
    target_link_libraries(${target} PRIVATE funchook-${typelc})
    if (${typelc} STREQUAL shared)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:funchook-shared> $<TARGET_FILE_DIR:${target}>
            )
    endif()
endfunction()

add_library(alien SHARED)
set_target_properties(alien PROPERTIES
    OUTPUT_NAME alien_c
    PREFIX "")
target_sources(alien PRIVATE
    src/alien.cpp
    src/alien_library.cpp
    src/alien_lua_util.cpp
    src/alien_hook.cpp
    src/alien_buffer.cpp
    src/alien_callback.cpp
    src/alien_function.cpp
    src/alien_struct.cpp
    src/alien_type_basic.cpp
    src/alien_type_struct.cpp
    src/alien_type_union.cpp
    src/utils.cpp
    )
target_include_directories(alien PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(alien PRIVATE lua)
link_libffi(alien)
link_funchook(alien SHARED)

set(alien_lua_f ${CMAKE_CURRENT_LIST_DIR}/src/alien.lua)
add_custom_target(alien_lua ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${alien_lua_f} $<TARGET_FILE_DIR:alien> DEPENDS ${lua_test_file})

set(alien_lua_d ${CMAKE_CURRENT_LIST_DIR}/src/alien)
file(GLOB_RECURSE alien_lua_d_f ${alien_lua_d}/*)
add_custom_target(alien_lua_sub ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${alien_lua_d} $<TARGET_FILE_DIR:alien>/alien
    DEPENDS ${alien_lua_d_f})

set(antianti ${CMAKE_CURRENT_LIST_DIR}/tools/antianti)
file(GLOB_RECURSE antianti_SOURCES ${antianti}/*)
add_custom_target(antianti_lib ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${antianti} $<TARGET_FILE_DIR:alien>/antianti
    DEPENDS ${antianti_SOURCES})

add_library(alientest SHARED tests/alientest.c)
if (NOT WIN32)
target_link_libraries(alientest m)
endif()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/tools)

enable_testing()
file(GLOB alientest_lua "${CMAKE_CURRENT_LIST_DIR}/tests/test_*.lua")
foreach(lua_test_file IN LISTS alientest_lua)
    get_filename_component(filenamewe ${lua_test_file} NAME_WE)
    string(CONCAT targetname "lua_" ${filenamewe})

    add_custom_target(${targetname} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${alientest_lua} $<TARGET_FILE_DIR:alientest>
        DEPENDS ${lua_test_file}
    )

    add_test(NAME ${targetname} 
             COMMAND lua ${filenamewe}.lua 
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    message("-- added test ${targetname}")
endforeach()
