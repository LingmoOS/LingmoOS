set(xcb_plugin_SRCS
    kglobalaccel_x11.cpp
    ../../logging.cpp
)

add_library(KF5GlobalAccelPrivateXcb MODULE ${xcb_plugin_SRCS})
target_link_libraries(KF5GlobalAccelPrivateXcb
    KF5GlobalAccelPrivate
    XCB::XCB
    XCB::KEYSYMS
)

install(
    TARGETS
        KF5GlobalAccelPrivateXcb
    DESTINATION
        ${PLUGIN_INSTALL_DIR}/org.kde.kglobalaccel5.platforms/
)
