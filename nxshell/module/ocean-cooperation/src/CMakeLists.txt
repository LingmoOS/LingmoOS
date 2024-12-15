# share libraries
add_subdirectory(lib)

add_subdirectory(apps)

if(ENABLE_CUTEIPC)
    # compat v20 daemon app
    add_subdirectory(compat)
endif()
