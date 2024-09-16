set(xcb_plugin_SRCS
    kwindoweffects.cpp
    kwindowinfo.cpp
    kwindowsystem.cpp
    kxerrorhandler.cpp
    kxutils.cpp
    plugin.cpp
)
ecm_qt_declare_logging_category(xcb_plugin_SRCS HEADER kwindowsystem_xcb_debug.h IDENTIFIER LOG_KKEYSERVER_X11 CATEGORY_NAME org.kde.kwindowsystem.keyserver.x11 DEFAULT_SEVERITY Warning)

add_library(KF5WindowSystemX11Plugin MODULE ${xcb_plugin_SRCS})
target_link_libraries(KF5WindowSystemX11Plugin
    KF5WindowSystem
    Qt5::X11Extras
    XCB::XCB
    ${X11_LIBRARIES}
    ${X11_Xfixes_LIB}
)

ecm_generate_headers(KWindowSystemX11_HEADERS
    HEADER_NAMES
        KSelectionOwner
        KSelectionWatcher
        KXMessages
        NETWM # does not match the classnames in that file...

    REQUIRED_HEADERS
        KWindowSystemX11_HEADERS
)

install(
    FILES
        ${KWindowSystemX11_HEADERS}
        kkeyserver_x11.h
        kmanagerselection.h
        fixx11h.h
    DESTINATION
        ${KDE_INSTALL_INCLUDEDIR_KF5}/KWindowSystem
    COMPONENT
        Devel
)

set_target_properties(
    KF5WindowSystemX11Plugin
    PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/kf5/org.kde.kwindowsystem.platforms"
)

install(
    TARGETS
        KF5WindowSystemX11Plugin
    DESTINATION
        ${PLUGIN_INSTALL_DIR}/kf5/org.kde.kwindowsystem.platforms/
)
