option(USE_KWIN_NO_SCALE "use lingmo LD_PRELOAD kwin_no_scale" ON)

install(
    FILES
    kglobalshortcutsrc
    lingmo-kwinrc
    lingmo-kwinrulesrc
    klaunchrc
    kdeglobals
    DESTINATION
    "/etc/xdg"
)

install(
    FILES
    org.kde.kwin.compositing.json
    org.kde.kwin.logging.json
    org.kde.kwin.multitaskview.display.json
    org.lingmo.displayjack.watermark.display.json
    org.kde.kwin.eventtracking.json
    org.kde.kwin.virtualterminal.switch.json
    DESTINATION
    "/usr/share/dsg/configs/org.kde.kwin"
)

install(
    FILES
    org.kde.kwin.decoration.titlebar.json
    DESTINATION
    "/usr/share/dsg/configs/org.kde.kwin.decoration"
)

install(
    FILES
    org.lingmo.kwin.json
    DESTINATION
    "/usr/share/lingmo-log-viewer/lingmo-log.conf.d/"
)

if (USE_KWIN_NO_SCALE)
    configure_file(kwin_no_scale.in kwin_no_scale)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kwin_no_scale DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif()
