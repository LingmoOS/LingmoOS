add_subdirectory(kirigami-filepicker)

add_definitions(-DTRANSLATION_DOMAIN="xdg-desktop-portal-kde")

add_executable(xdg-desktop-portal-kde)

set(xdg_desktop_portal_kde_SRCS
    access.cpp
    accessdialog.cpp
    accessdialog.h
    access.h
    account.cpp
    account.h
    appchooser.cpp
    appchooserdialog.cpp
    appchooserdialog.h
    appchooser.h
    background.cpp
    background.h
    dbushelpers.cpp dbushelpers.h
    desktopportal.cpp
    desktopportal.h
    email.cpp
    email.h
    filechooser.cpp
    filechooser.h
    globalshortcuts.cpp globalshortcuts.h
    inhibit.cpp
    inhibit.h
    inputcapture.cpp
    inputcapturebarrier.cpp
    inputcapturedialog.cpp
    notification.cpp
    notification.h
    notificationinhibition.cpp
    notificationinhibition.h
    outputsmodel.cpp outputsmodel.h
    print.cpp
    print.h
    quickdialog.cpp quickdialog.h
    remotedesktop.cpp
    remotedesktopdialog.cpp
    remotedesktopdialog.h
    remotedesktop.h
    restoredata.h
    restoredata.cpp
    request.cpp
    request.h
    screencast.cpp
    screencast.h
    screencasting.cpp
    screencasting.h
    screenchooserdialog.cpp
    screenchooserdialog.h
    screenshot.cpp
    screenshotdialog.cpp
    screenshotdialog.h
    screenshot.h
    session.cpp
    session.h
    settings.cpp
    settings.h
    userinfodialog.cpp
    userinfodialog.h
    utils.cpp
    utils.h
    waylandintegration.cpp
    waylandintegration.h
    waylandintegration_p.h
    xdg-desktop-portal-kde.cpp
    resources.qrc
    portalicon.cpp portalicon.h
    dynamiclauncher.cpp dynamiclauncher.h
    dynamiclauncherdialog.cpp dynamiclauncherdialog.h
    xdgshortcut.cpp xdgshortcut.h
    region-select/SelectionEditor.h
    region-select/SelectionEditor.cpp
)

ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeBackground"
    CATEGORY_NAME "xdp-kde-background"
    HEADER background_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Background"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeNotificationInhibition"
    CATEGORY_NAME "xdp-kde-notificationinhibition"
    HEADER notificationinhibition_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Notification Inhibition"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeAccess"
    CATEGORY_NAME "xdp-kde-access"
    HEADER access_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Access"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeAccount"
    CATEGORY_NAME "xdp-kde-account"
    HEADER account_debug.h)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeSettings"
    CATEGORY_NAME "xdp-kde-settings"
    HEADER settings_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Settings"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeAppChooser"
    CATEGORY_NAME "xdp-kde-app-chooser"
    HEADER appchooser_debug.h
    DESCRIPTION "XDG Desktop Portal KDE App Chooser"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeDesktopPortal"
    CATEGORY_NAME "xdp-kde-desktop-portal"
    HEADER desktopportal_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Desktop Portal"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeDynamicLauncher"
    CATEGORY_NAME "xdp-kde-dynamic-launcher"
    HEADER dynamiclauncher_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Dynamic Launcher"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeDynamicLauncherDialog"
    CATEGORY_NAME "xdp-kde-dynamic-launcher-dialog"
    HEADER dynamiclauncherdialog_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Dynamic Launcher Dialog"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeEmail"
    CATEGORY_NAME "xdp-kde-email"
    HEADER email_debug.h
    DESCRIPTION "XDG Desktop Portal KDE e-mail"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeFileChooser"
    CATEGORY_NAME "xdp-kde-file-chooser"
    HEADER filechooser_debug.h
    DESCRIPTION "XDG Desktop Portal KDE File Chooser"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeInhibit"
    CATEGORY_NAME "xdp-kde-inhibit"
    HEADER inhibit_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Inhibit"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeNotification"
    CATEGORY_NAME "xdp-kde-notification"
    HEADER notification_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Notification"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdePrint"
    CATEGORY_NAME "xdp-kde-print"
    HEADER print_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Print"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeRemoteDesktop"
    CATEGORY_NAME "xdp-kde-remotedesktop"
    HEADER remotedesktop_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Remote Desktop"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeRemoteDesktopDialog"
    CATEGORY_NAME "xdp-kde-remote-desktop-dialog"
    HEADER remotedesktopdialog_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Remote Desktop Dialog"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgRequestKdeRequest"
    CATEGORY_NAME "xdp-kde-request"
    HEADER request_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Request"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeScreenCast"
    CATEGORY_NAME "xdp-kde-screencast"
    HEADER screencast_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Screencast"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeScreenshot"
    CATEGORY_NAME "xdp-kde-screenshot"
    HEADER screenshot_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Screenshot"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeScreenshotDialog"
    CATEGORY_NAME "xdp-kde-screenshot-dialog"
    HEADER screenshotdialog_debug.h)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgSessionKdeSession"
    CATEGORY_NAME "xdp-kde-session"
    HEADER session_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Session"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKde"
    CATEGORY_NAME "xdp-kde"
    HEADER debug.h
    DESCRIPTION "XDG Desktop Portal KDE Generic"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeWaylandIntegration"
    CATEGORY_NAME "xdp-kde-wayland-integration"
    HEADER waylandintegration_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Wayland"
    EXPORT XDPKDE
)
ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeAccessDialog"
    CATEGORY_NAME "xdp-kde-access-dialog"
    HEADER accessdialog_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Access Dialog"
    EXPORT XDPKDE
)

ecm_qt_declare_logging_category(xdg_desktop_portal_kde_SRCS
    IDENTIFIER "XdgDesktopPortalKdeInputCapture"
    CATEGORY_NAME "xdp-kde-inputcapture"
    HEADER inputcapture_debug.h
    DESCRIPTION "XDG Desktop Portal KDE Input Capture"
    EXPORT XDPKDE
)

set(kglobalaccel_xml ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf6_org.kde.KGlobalAccel.xml)
set_source_files_properties(${kglobalaccel_xml} PROPERTIES
    INCLUDE "kglobalaccel.h"
    NO_NAMESPACE TRUE
    CLASSNAME "KGlobalAccelInterface"
)
qt_add_dbus_interface(xdg_desktop_portal_kde_SRCS ${kglobalaccel_xml} kglobalaccel_interface)

set(kglobalaccel_component_xml ${KGLOBALACCEL_DBUS_INTERFACES_DIR}/kf6_org.kde.kglobalaccel.Component.xml)
set_source_files_properties(${kglobalaccel_component_xml} PROPERTIES
    INCLUDE "kglobalshortcutinfo.h"
    NO_NAMESPACE TRUE
    CLASSNAME "KGlobalAccelComponentInterface"
)
qt_add_dbus_interface(xdg_desktop_portal_kde_SRCS ${kglobalaccel_component_xml} kglobalaccel_component_interface)

qt_add_dbus_interface(xdg_desktop_portal_kde_SRCS org.kde.KWin.TabletModeManager.xml tabletmodemanager_interface)
qt_add_dbus_interface(xdg_desktop_portal_kde_SRCS org.kde.KWin.VirtualKeyboard.xml virtualkeyboard_interface)

qt6_generate_wayland_protocol_client_sources(xdg-desktop-portal-kde FILES ${PLASMA_WAYLAND_PROTOCOLS_DIR}/zkde-screencast-unstable-v1.xml)
qt6_generate_wayland_protocol_client_sources(xdg-desktop-portal-kde FILES ${PLASMA_WAYLAND_PROTOCOLS_DIR}/fake-input.xml)
qt6_generate_wayland_protocol_client_sources(xdg-desktop-portal-kde FILES ${Wayland_DATADIR}/wayland.xml)


set_source_files_properties(../data/org.freedesktop.Accounts.User.xml PROPERTIES NO_NAMESPACE TRUE)
qt_add_dbus_interface(xdg_desktop_portal_kde_SRCS ../data/org.freedesktop.Accounts.User.xml user_interface)

set_source_files_properties(../data/org.freedesktop.portal.Documents.xml PROPERTIES NO_NAMESPACE TRUE)
qt_add_dbus_interface(xdg_desktop_portal_kde_SRCS ../data/org.freedesktop.portal.Documents.xml documents_interface)

set_source_files_properties(../data/org.kde.KIOFuse.VFS.xml PROPERTIES NO_NAMESPACE TRUE)
qt_add_dbus_interface(xdg_desktop_portal_kde_SRCS ../data/org.kde.KIOFuse.VFS.xml fuse_interface)

qt_add_dbus_interface(xdg_desktop_portal_kde_SRCS ../data/org.freedesktop.Application.xml fdo_application_interface)

target_sources(xdg-desktop-portal-kde PRIVATE ${xdg_desktop_portal_kde_SRCS})

target_link_libraries(xdg-desktop-portal-kde
    Qt::Core
    Qt::GuiPrivate
    Qt::DBus
    Qt::Concurrent
    Qt::PrintSupport
    Qt::QuickWidgets
    Qt::Widgets
    Qt::WaylandClient
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::GlobalAccel
    KF6::GuiAddons
    KF6::I18n
    KF6::KIOFileWidgets
    KF6::Notifications
    KF6::Service
    Plasma::KWaylandClient
    KF6::WidgetsAddons
    KF6::WindowSystem
    KF6::IconThemes
    KF6::StatusNotifierItem
    KirigamiFilepicker
    Wayland::Client
    PkgConfig::XKB
)
target_link_libraries(xdg-desktop-portal-kde Qt::PrintSupportPrivate Qt::GuiPrivate)

install(TARGETS xdg-desktop-portal-kde DESTINATION ${KDE_INSTALL_LIBEXECDIR})

install(FILES
    xdg-desktop-portal-kde.notifyrc
    DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

ecm_qt_install_logging_categories(EXPORT XDPKDE FILE xdp-kde.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})
