configure_file(config-kwindowsystem.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kwindowsystem.h )

add_library(KF6WindowSystem)
add_library(KF6::WindowSystem ALIAS KF6WindowSystem)

qt_extract_metatypes(KF6WindowSystem)

set_target_properties(KF6WindowSystem PROPERTIES
    VERSION     ${KWINDOWSYSTEM_VERSION}
    SOVERSION   ${KWINDOWSYSTEM_SOVERSION}
    EXPORT_NAME WindowSystem
)

ecm_create_qm_loader(KF6WindowSystem kwindowsystem6_qt)

target_sources(KF6WindowSystem PRIVATE
    kwindoweffects.cpp
    kwindoweffects_dummy.cpp
    kwindowshadow.cpp
    kwindowsystem.cpp
    pluginwrapper.cpp
    kwindowsystemplugininterface.cpp
    )
ecm_qt_declare_logging_category(KF6WindowSystem
    HEADER kwindowsystem_debug.h
    IDENTIFIER LOG_KWINDOWSYSTEM
    CATEGORY_NAME kf.windowsystem
    OLD_CATEGORY_NAMES org.kde.kwindowsystem
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KWindowSystem"
    EXPORT KWINDOWSYSTEM
)

if (KWINDOWSYSTEM_X11)
   target_include_directories(KF6WindowSystem
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/platforms/xcb
   )

   target_link_libraries(KF6WindowSystem
        PUBLIC
            # public because they are used in kkeyserver_x11.h
            X11::X11
        PRIVATE
            XCB::XCB
            XCB::RES
            X11::Xfixes
            XCB::KEYSYMS
            Qt6::GuiPrivate # qtx11extras_p.h
   )

   ecm_qt_declare_logging_category(KF6WindowSystem
        HEADER kwindowsystem_xcb_debug.h
        IDENTIFIER LOG_KKEYSERVER_X11
        CATEGORY_NAME kf.windowsystem.keyserver.x11
        DEFAULT_SEVERITY Warning
    )

   target_sources(KF6WindowSystem PRIVATE
        platforms/xcb/kselectionowner.cpp
        platforms/xcb/kselectionwatcher.cpp
        platforms/xcb/kxmessages.cpp
        platforms/xcb/kxutils.cpp
        platforms/xcb/netwm.cpp
        kkeyserver.cpp
        kx11extras.cpp
        kstartupinfo.cpp
        kusertimestamp.cpp
        kxerrorhandler.cpp
        kwindowinfo.cpp
   )

   # we install kkeyserver_x11.h which needs the X11 headers available
   # if we don't add the include path here code that includes kkeyserver.h will fail
   # to compile unless X11 is installed in /usr/include
   target_include_directories(KF6WindowSystem PUBLIC ${XCB_XCB_INCLUDE_DIR})
endif ()

if (KWINDOWSYSTEM_WAYLAND)
    target_sources(KF6WindowSystem PRIVATE kwaylandextras.cpp)
endif()

ecm_generate_export_header(KF6WindowSystem
    BASE_NAME KWindowSystem
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 6.0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6WindowSystem INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KWindowSystem>")

target_link_libraries(KF6WindowSystem
    PUBLIC Qt6::Gui
)

ecm_generate_headers(KWindowSystem_HEADERS
  HEADER_NAMES
  KWindowEffects
  KWindowShadow,KWindowShadowTile
  KWindowSystem

  REQUIRED_HEADERS KWindowSystem_HEADERS
)

if (KWINDOWSYSTEM_X11)
    ecm_generate_headers(KWindowSystem_HEADERS
    HEADER_NAMES
    KX11Extras
    KStartupInfo
    KUserTimestamp
    KKeyServer
    KWindowInfo

    REQUIRED_HEADERS KWindowSystem_HEADERS
    )
endif()

if (KWINDOWSYSTEM_WAYLAND)
    ecm_generate_headers(KWindowSystem_HEADERS
        HEADER_NAMES
        KWaylandExtras

        REQUIRED_HEADERS KWindowSystem_HEADERS
    )
endif()

install(TARGETS KF6WindowSystem EXPORT KF6WindowSystemTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  # FIXME: It seems odd to install this.
  ${CMAKE_CURRENT_BINARY_DIR}/config-kwindowsystem.h
  ${CMAKE_CURRENT_BINARY_DIR}/kwindowsystem_export.h
  ${KWindowSystem_HEADERS}
  netwm_def.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KWindowSystem COMPONENT Devel
)

install(
    FILES
        kwindoweffects_p.h
        kwindowshadow_p.h
        kwindowsystem_p.h
        kwindowsystemplugininterface_p.h
    DESTINATION
        ${KDE_INSTALL_INCLUDEDIR_KF}/KWindowSystem/private
    COMPONENT
        Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6WindowSystem_QCH
        NAME KWindowSystem
        BASE_NAME KF6WindowSystem
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCE_DIRS
            # using dir for now, to cover any platform dependent code
            # TODO: should only use public headers, to cover only public API
            ${CMAKE_CURRENT_SOURCE_DIR}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt6Gui_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KWINDOWSYSTEM_EXPORT
            KWINDOWSYSTEM_DEPRECATED
            KWINDOWSYSTEM_DEPRECATED_EXPORT
            "KWINDOWSYSTEM_DEPRECATED_VERSION(x, y, t)"
            "KWINDOWSYSTEM_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KWINDOWSYSTEM_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "KWINDOWSYSTEM_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

add_subdirectory(platforms)

if(KWINDOWSYSTEM_QML)
    add_subdirectory(qml)
endif()

ecm_qt_install_logging_categories(
    EXPORT KWINDOWSYSTEM
    FILE kwindowsystem.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

