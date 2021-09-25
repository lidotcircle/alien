
if(WIN32)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
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

