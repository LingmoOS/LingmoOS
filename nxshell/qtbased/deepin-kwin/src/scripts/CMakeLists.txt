function(add_kwin_script name)
    kpackage_install_package(${name} ${name} scripts deepin-kwin)

    # Copy the script to the build directory so one can run tests without prior
    # make install. FIXME: use add_custom_command.
    file(COPY ${name} DESTINATION ${CMAKE_BINARY_DIR}/bin/deepin-deepin-kwin/scripts/)
endfunction()

add_kwin_script(videowall)
add_kwin_script(synchronizeskipswitcher)
add_kwin_script(desktopchangeosd)
add_kwin_script(minimizeall)
add_kwin_script(ddelaunchpad)