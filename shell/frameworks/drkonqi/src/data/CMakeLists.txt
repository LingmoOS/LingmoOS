install(FILES mappings DESTINATION ${KDE_INSTALL_DATADIR}/drkonqi)
install(DIRECTORY debuggers DESTINATION ${KDE_INSTALL_DATADIR}/drkonqi FILES_MATCHING PATTERN "*rc")
install(DIRECTORY gdb_preamble DESTINATION ${KDE_INSTALL_DATADIR}/drkonqi/gdb/python/)

if(WITH_PYTHON_VENDORING)
    find_program(pip_EXE pip3)
    if(NOT pip_EXE)
        find_program(pip_EXE pip)
    endif()

    set(vendor_dir ${CMAKE_CURRENT_BINARY_DIR}/python)
    add_custom_target(python_vendor ALL
        BYPRODUCTS ${vendor_dir}
        COMMAND ${pip_EXE} install --target ${vendor_dir} pygdbmi psutil sentry-sdk)
    install(DIRECTORY ${vendor_dir} DESTINATION ${KDE_INSTALL_DATADIR}/drkonqi/gdb/)
endif()
