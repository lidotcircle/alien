
if(WIN32)
    if (NOT TARGET libffi)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(ARCH_PATH "x64")
        else()
            set(ARCH_PATH "x86")
        endif()

        add_library(libffi INTERFACE)
        target_include_directories(libffi INTERFACE "${CMAKE_CURRENT_LIST_DIR}/libffi/${ARCH_PATH}/include")
        target_link_libraries(libffi INTERFACE "${CMAKE_CURRENT_LIST_DIR}/libffi/${ARCH_PATH}/libffi.lib")
        set(libffiDLL "${CMAKE_CURRENT_LIST_DIR}/libffi/${ARCH_PATH}/libffi.dll")

        function(link_libffi target)
            target_link_libraries(${target} PRIVATE libffi)
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${libffiDLL} $<TARGET_FILE_DIR:${target}>
                )
        endfunction()
    endif()
else()
    find_library(LIBFFI_LIBRARY
        NAME ffi libffi
        HINTS /usr/lib/x86_64-linux-gnu)
    find_path(LIBFFI_INCLUDE_DIR ffi.h)

    if (NOT LIBFFI_LIBRARY OR NOT LIBFFI_INCLUDE_DIR)
        message(${LIBFFI_INCLUDE_DIR}, ${LIBFFI_LIBRARY})
        message(FATAL_ERROR "find libffi failed")
    else()
        message("-- Found libffi")
        message("--   ffi include directory: ${LIBFFI_INCLUDE_DIR}")
        message("--   ffi library: ${LIBFFI_LIBRARY}")
    endif()

    function(link_libffi target)
        target_link_libraries(${target} PRIVATE ${LIBFFI_LIBRARY})
        target_include_directories(${target} PRIVATE ${LIBFFI_INCLUDE_DIR})
    endfunction()
endif()

