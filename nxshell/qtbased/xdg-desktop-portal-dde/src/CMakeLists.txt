find_package(
    Qt6 CONFIG REQUIRED COMPONENTS
    Core
    Concurrent
    DBus
    Widgets
    WaylandClient)
find_package(Qt6WaylandScannerTools REQUIRED)

set_source_files_properties(
                      ${CMAKE_SOURCE_DIR}/misc/org.freedesktop.Notifications.xml
                      PROPERTIES
                      CLASSNAME
                        NotificationService
                      NO_NAMESPACE
                        OFF
)

qt6_add_dbus_interface(NotificationInterface
                      ${CMAKE_SOURCE_DIR}/misc/org.freedesktop.Notifications.xml
                      notification_interface
)

set(SRC
    ${NotificationInterface}
    xdg-desktop-portal-dde.cpp
    ddesktopportal.h
    ddesktopportal.cpp
    screenshot.h
    screenshot.cpp
    background.h
    background.cpp
    filechooser.h
    filechooser.cpp
    wallpaper.h
    wallpaper.cpp
    notification.h
    notification.cpp
    settings.h
    settings.cpp
    inhibit.h
    inhibit.cpp
    appchooser.h
    appchooser.cpp
    appchooserdialog.h
    appchooserdialog.cpp
    appchooserdelegate.h
    appchooserdelegate.cpp
    appchoosermodel.h
    appchoosermodel.cpp
    iteminfo.h
    iteminfo.cpp
    account.h
    account.cpp
    access.cpp
    access.h
    globalshortcut.h
    globalshortcut.cpp
    lockdown.h
    lockdown.cpp
    secret.h
    secret.cpp
    request.h
    request.cpp
    dbushelpers.h
    utils.h
    utils.cpp
    personalization_manager_client.h
    personalization_manager_client.cpp
)

add_executable(${PROJECT_NAME}
    ${SRC})

qt6_generate_wayland_protocol_client_sources(${PROJECT_NAME}
    FILES
    ${CMAKE_SOURCE_DIR}/protocol/treeland-personalization-manager-v1.xml
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::Gui
    Qt::DBus
    Qt::Concurrent
    Qt::WaylandClient
    xdg-desktop-portal-dde-wayland
    )

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBEXECDIR})

add_subdirectory(wayland)
