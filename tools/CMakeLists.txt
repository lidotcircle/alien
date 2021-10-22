cmake_minimum_required(VERSION 3.4)

project(alien_tools)

include("../cmake/lua.cmake")

if(WIN32)
add_executable(antclose ${CMAKE_CURRENT_LIST_DIR}/windows/antiDebug-ntclose.c)

add_library(anandebug SHARED ${CMAKE_CURRENT_LIST_DIR}/windows/antianti.cpp)
target_link_libraries(anandebug PRIVATE funchook-shared)
endif()

add_library(lualoader SHARED ${CMAKE_CURRENT_LIST_DIR}/lualoader.c)
target_link_libraries(lualoader PRIVATE lua)
set_target_properties(lualoader PROPERTIES
    OUTPUT_NAME lualdr
    PREFIX "")
