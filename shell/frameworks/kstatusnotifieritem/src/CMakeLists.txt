add_library(KF6StatusNotifierItem)
add_library(KF6::StatusNotifierItem ALIAS KF6StatusNotifierItem)

set_target_properties(KF6StatusNotifierItem PROPERTIES
    VERSION     ${KSTATUSNOTIFIERITEM_VERSION}
    SOVERSION   ${KSTATUSNOTIFIERITEM_SOVERSION}
    EXPORT_NAME StatusNotifierItem
)

ecm_create_qm_loader(KF6StatusNotifierItem kstatusnotifieritem6_qt)

target_sources(KF6StatusNotifierItem PRIVATE
    kstatusnotifieritem.cpp
)

if(APPLE)
    target_sources(KF6StatusNotifierItem PRIVATE
            macutils.mm)
endif()

if (HAVE_DBUS)
  target_sources(KF6StatusNotifierItem PRIVATE
    kstatusnotifieritemdbus_p.cpp
  )
endif()

ecm_qt_declare_logging_category(KF6StatusNotifierItem
    HEADER debug_p.h
    IDENTIFIER LOG_KSTATUSNOTIFIERITEM
    CATEGORY_NAME kf.statusnotifieritem
    DESCRIPTION "KStatusNotifierItem"
    EXPORT KSTATUSNOTIFIERITEM
)

if (HAVE_DBUS)
   set(HAVE_DBUSMENUQT 1)
   qt_add_dbus_adaptor(dbusmenu_qt_SRCS
     ${CMAKE_CURRENT_SOURCE_DIR}/libdbusmenu-qt/com.canonical.dbusmenu.xml
     ${CMAKE_CURRENT_SOURCE_DIR}/libdbusmenu-qt/dbusmenuexporterdbus_p.h DBusMenuExporterDBus
   )

   target_sources(KF6StatusNotifierItem PRIVATE
     ${dbusmenu_qt_SRCS}
     libdbusmenu-qt/dbusmenuexporter.cpp
     libdbusmenu-qt/dbusmenuexporterdbus_p.cpp
     libdbusmenu-qt/dbusmenu_p.cpp
     libdbusmenu-qt/dbusmenushortcut_p.cpp
     libdbusmenu-qt/dbusmenutypes_p.cpp
     libdbusmenu-qt/utils.cpp
     libdbusmenu-qt/dbusmenuexporterdbus_p.h
     libdbusmenu-qt/dbusmenuexporter.h
     libdbusmenu-qt/dbusmenuexporterprivate_p.h
     libdbusmenu-qt/dbusmenu_p.h
     libdbusmenu-qt/dbusmenushortcut_p.h
     libdbusmenu-qt/dbusmenutypes_p.h
     libdbusmenu-qt/debug_p.h
     libdbusmenu-qt/utils_p.h
  )

  set(kstatusnotifieritem_dbus_SRCS)
  qt_add_dbus_adaptor(kstatusnotifieritem_dbus_SRCS     org.kde.StatusNotifierItem.xml
                    kstatusnotifieritemdbus_p.h KStatusNotifierItemDBus)


  set(statusnotifierwatcher_xml org.kde.StatusNotifierWatcher.xml)
  qt_add_dbus_interface(kstatusnotifieritem_dbus_SRCS ${statusnotifierwatcher_xml} statusnotifierwatcher_interface)
  set(notifications_xml org.freedesktop.Notifications.xml)
  qt_add_dbus_interface(kstatusnotifieritem_dbus_SRCS ${notifications_xml} notifications_interface)
  target_sources(KF6StatusNotifierItem PRIVATE ${kstatusnotifieritem_dbus_SRCS})
endif()

configure_file(config-kstatusnotifieritem.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kstatusnotifieritem.h )

ecm_generate_export_header(KF6StatusNotifierItem
    EXPORT_FILE_NAME kstatusnotifieritem_export.h
    BASE_NAME KStatusNotifierItem
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6StatusNotifierItem INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KStatusNotifierItem>")

target_link_libraries(KF6StatusNotifierItem PUBLIC Qt6::Gui Qt6::Widgets)

if (HAVE_DBUS)
   target_link_libraries(KF6StatusNotifierItem PUBLIC Qt6::DBus)
endif()

if (TARGET KF6::WindowSystem)
    target_link_libraries(KF6StatusNotifierItem PRIVATE KF6::WindowSystem)
endif()

target_link_libraries(KF6StatusNotifierItem PRIVATE Qt6::WidgetsPrivate)

if(APPLE)
  target_link_libraries(KF6StatusNotifierItem PRIVATE "-framework Foundation" "-framework AppKit")
endif()

ecm_generate_headers(KStatusNotifierItem_HEADERS
  HEADER_NAMES
  KStatusNotifierItem

  REQUIRED_HEADERS KStatusNotifierItem_HEADERS
)

install(TARGETS KF6StatusNotifierItem EXPORT KF6StatusNotifierItemTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kstatusnotifieritem_export.h
  ${KStatusNotifierItem_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KStatusNotifierItem COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KSTATUSNOTIFIERITEM
    FILE kstatusnotifieritem.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6StatusNotifierItem_QCH
        NAME KStatusNotifierItem
        BASE_NAME KF6StatusNotifierItem
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KStatusNotifierItem_HEADERS}
        LINK_QCHS
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KSTATUSNOTIFIERITEM_EXPORT
            KSTATUSNOTIFIERITEM_DEPRECATED
            KSTATUSNOTIFIERITEM_DEPRECATED_EXPORT
            "KSTATUSNOTIFIERITEM_DEPRECATED_VERSION(x, y, t)"
            "KSTATUSNOTIFIERITEM_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if (HAVE_DBUS)
    install(FILES
        org.kde.StatusNotifierItem.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
        RENAME kf6_org.kde.StatusNotifierItem.xml)

    install(FILES
        org.kde.StatusNotifierWatcher.xml
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
        RENAME kf6_org.kde.StatusNotifierWatcher.xml)
endif()

