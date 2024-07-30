add_library(KF6Notifications)
add_library(KF6::Notifications ALIAS KF6Notifications)

qt_extract_metatypes(KF6Notifications)

set_target_properties(KF6Notifications PROPERTIES
    VERSION     ${KNOTIFICATIONS_VERSION}
    SOVERSION   ${KNOTIFICATIONS_SOVERSION}
    EXPORT_NAME Notifications
)

ecm_create_qm_loader(KF6Notifications knotifications6_qt)

target_sources(KF6Notifications PRIVATE
  knotification.cpp
  knotificationreplyaction.cpp
  knotificationmanager.cpp
  knotificationpermission.cpp

  knotifyconfig.cpp
  knotificationplugin.cpp
)

if (HAVE_DBUS)
  target_sources(KF6Notifications PRIVATE
    imageconverter.cpp #needed to marshal images for sending over dbus by NotifyByPopup
    notifybypopup.cpp
    notifybyportal.cpp
  )
endif()

if (ANDROID)
    add_subdirectory(android)
    target_sources(KF6Notifications PRIVATE
        notifybyandroid.cpp
        knotifications.qrc
    )
endif()

if (WITH_SNORETOAST)
    target_sources(KF6Notifications PRIVATE notifybysnore.cpp)
endif ()

if (APPLE)
    target_sources(KF6Notifications PRIVATE notifybymacosnotificationcenter.mm)
endif()

ecm_qt_declare_logging_category(KF6Notifications
    HEADER debug_p.h
    IDENTIFIER LOG_KNOTIFICATIONS
    CATEGORY_NAME kf.notifications
    OLD_CATEGORY_NAMES org.kde.knotifications
    DESCRIPTION "KNotifications"
    EXPORT KNOTIFICATIONS
)

if (TARGET Canberra::Canberra)
    target_sources(KF6Notifications PRIVATE
        notifybyaudio.cpp
    )
endif()

if (HAVE_DBUS)
  set(notifications_xml org.freedesktop.Notifications.xml)
  qt_add_dbus_interface(knotifications_dbus_SRCS ${notifications_xml} notifications_interface)
  target_sources(KF6Notifications PRIVATE ${knotifications_dbus_SRCS})
endif()

configure_file(config-knotifications.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-knotifications.h )

ecm_generate_export_header(KF6Notifications
    EXPORT_FILE_NAME knotifications_export.h
    BASE_NAME KNotifications
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6Notifications INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KNotifications>")

target_link_libraries(KF6Notifications PUBLIC Qt6::Gui)
if (HAVE_DBUS)
   target_link_libraries(KF6Notifications PUBLIC Qt6::DBus)
endif()
target_link_libraries(KF6Notifications PRIVATE
   KF6::ConfigCore
)
if (ANDROID)
    target_link_libraries(KF6Notifications PRIVATE Qt::CorePrivate)
endif()

if (TARGET SnoreToast::SnoreToastActions)
  target_link_libraries(KF6Notifications PRIVATE Qt6::Network SnoreToast::SnoreToastActions)
endif ()

if (TARGET Canberra::Canberra)
    target_link_libraries(KF6Notifications PRIVATE Canberra::Canberra)
endif()

if(APPLE)
  target_link_libraries(KF6Notifications PRIVATE "-framework Foundation" "-framework AppKit")
endif()

ecm_generate_headers(KNotifications_HEADERS
  HEADER_NAMES
  KNotification
  KNotificationPermission
  KNotificationReplyAction
  KNotifyConfig

  REQUIRED_HEADERS KNotifications_HEADERS
)

install(TARGETS KF6Notifications EXPORT KF6NotificationsTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/knotifications_export.h
  ${KNotifications_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KNotifications COMPONENT Devel
)

ecm_qt_install_logging_categories(
    EXPORT KNOTIFICATIONS
    FILE knotifications.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6Notifications_QCH
        NAME KNotifications
        BASE_NAME KF6Notifications
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KNotifications_HEADERS}
            "${CMAKE_SOURCE_DIR}/Mainpage.dox"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Gui_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KNOTIFICATIONS_EXPORT
            KNOTIFICATIONS_DEPRECATED
            KNOTIFICATIONS_DEPRECATED_EXPORT
            "KNOTIFICATIONS_DEPRECATED_VERSION(x, y, t)"
            "KNOTIFICATIONS_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

if (ANDROID)
    install(FILES KF6Notifications-android-dependencies.xml
        DESTINATION ${KDE_INSTALL_LIBDIR}
        RENAME KF6Notifications_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml
    )
endif()

if (TARGET Qt6::Qml)
    add_subdirectory(qml)
endif()
