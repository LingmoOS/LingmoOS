add_library(KF6IdleTimeXcbPlugin0 MODULE)

target_include_directories(KF6IdleTimeXcbPlugin0
    PRIVATE
        ${XCB_XCB_INCLUDE_DIR}
        ${XCB_SYNC_INCLUDE_DIR}
)

target_sources(KF6IdleTimeXcbPlugin0 PRIVATE
    xsyncbasedpoller.cpp
    ../../logging.cpp
)

ecm_qt_declare_logging_category(KF6IdleTimeXcbPlugin0
    HEADER xsync_logging.h
    IDENTIFIER KIDLETIME_XSYNC_PLUGIN
    CATEGORY_NAME kf.idletime.xsync
    OLD_CATEGORY_NAMES org.kde.kf5.idletime.xsync
    DESCRIPTION "xsync plugin"
    EXPORT KIDLETIME
)

target_link_libraries(KF6IdleTimeXcbPlugin0
    KF6IdleTime
    Qt6::Gui
    XCB::XCB
    XCB::SYNC
    X11::XCB
    ${X11_LIBRARIES}
    ${X11_Xext_LIB}
)

install(
    TARGETS
        KF6IdleTimeXcbPlugin0
    DESTINATION
        ${KDE_INSTALL_PLUGINDIR}/kf6/org.kde.kidletime.platforms/
)
