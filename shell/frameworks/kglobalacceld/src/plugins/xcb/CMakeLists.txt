set(xcb_plugin_SRCS
    kglobalaccel_x11.cpp
    kglobalaccel_x11.h
    ../../logging.cpp
)

add_library(KGlobalAccelDXcb MODULE ${xcb_plugin_SRCS})
target_link_libraries(KGlobalAccelDXcb
    KGlobalAccelD
    KF6::WindowSystem
    XCB::XCB
    XCB::KEYSYMS
    XCB::XKB
    XCB::RECORD
)

install(
    TARGETS
        KGlobalAccelDXcb
    DESTINATION
        ${KDE_INSTALL_PLUGINDIR}/org.kde.kglobalacceld.platforms/
)
