# The shell's components drawn in the compositor, one static library each,
# on src/shell (internal windows) and their backends.

# The desktop background: the shared wallpaper decode, one node per output
# and the status bar's backdrop rules.
find_package(LingmoClients REQUIRED COMPONENTS appearance)
target_link_libraries(lingmo-shell PUBLIC Lingmo::AppearanceClient)
add_library(lingmo-shell-wallpaper STATIC
    src/shell/wallpaper/bar_backdrop.cpp
    src/shell/wallpaper/bar_backdrop.h
    src/shell/wallpaper/wallpaper.cpp
    src/shell/wallpaper/wallpaper.h
    src/shell/wallpaper/wallpaper_cache.cpp
    src/shell/wallpaper/wallpaper_cache.h
)
target_link_libraries(lingmo-shell-wallpaper PUBLIC lingmo-shell Lingmo::AppearanceClient)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-wallpaper)

# The on-screen display: org.lingmo.Osd and its level panel.
add_library(lingmo-shell-osd STATIC
    src/shell/osd/level_track.cpp
    src/shell/osd/level_track.h
    src/shell/osd/osd.cpp
    src/shell/osd/osd.h
    src/shell/osd/osd_levels.cpp
)
target_link_libraries(lingmo-shell-osd PUBLIC lingmo-shell lingmo-shell-statusbar
    Lingmo::AudioClient)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-osd)

# The dock: its view, window and input on the dock's backend.
add_library(lingmo-shell-dock-view STATIC
    src/shell/dock/dock_bounce.cpp
    src/shell/dock/dock_bounce.h
    src/shell/dock/dock_icon.cpp
    src/shell/dock/dock_icon.h
    src/shell/dock/dock_magnification.cpp
    src/shell/dock/dock_magnification.h
    src/shell/dock/dock_metrics.h
    src/shell/dock/dock_presentation.cpp
    src/shell/dock/dock_presentation.h
    src/shell/dock/dock_tooltip.cpp
    src/shell/dock/dock_tooltip.h
    src/shell/dock/dock_window.cpp
    src/shell/dock/dock_window.h
)
target_link_libraries(lingmo-shell-dock-view PUBLIC lingmo-shell lingmo-shell-dock)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-dock-view)

# The clipboard history's picker on the clipboard backend.
add_library(lingmo-shell-clipboard-view STATIC
    src/shell/clipboard/clipboard_picker.cpp
    src/shell/clipboard/clipboard_picker.h
    src/shell/clipboard/clipboard_picker_view.cpp
    src/shell/clipboard/clipboard_picker_view.h
)
target_link_libraries(lingmo-shell-clipboard-view PUBLIC lingmo-shell lingmo-shell-clipboard)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-clipboard-view)

# The sheets offering force quit for applications that stopped responding.
add_library(lingmo-shell-notresponding STATIC
    src/shell/notresponding/not_responding_sheets.cpp
    src/shell/notresponding/not_responding_sheets.h
)
target_link_libraries(lingmo-shell-notresponding PUBLIC lingmo-shell)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-notresponding)
# The screen capture interface: org.lingmo.Screenshot, its overlays,
# toolbar and Options, pictures and recordings.
add_library(lingmo-shell-screenshot STATIC
    src/shell/screenshot/capture_clipboard.cpp
    src/shell/screenshot/capture_clipboard.h
    src/shell/screenshot/capture_notifier.cpp
    src/shell/screenshot/capture_notifier.h
    src/shell/screenshot/capture_overlay.cpp
    src/shell/screenshot/capture_overlay.h
    src/shell/screenshot/capture_settings.cpp
    src/shell/screenshot/capture_settings.h
    src/shell/screenshot/capture_toolbar.cpp
    src/shell/screenshot/capture_toolbar.h
    src/shell/screenshot/screen_capture.cpp
    src/shell/screenshot/screen_capture.h
    src/shell/screenshot/screen_grabber.cpp
    src/shell/screenshot/screen_grabber.h
    src/shell/screenshot/screen_recording.cpp
    src/shell/screenshot/screen_recording.h
)
target_link_libraries(lingmo-shell-screenshot PUBLIC lingmo-shell lingmo-shell-common
    lingmo-wayland lingmo-wayland-private-shell Lingmo::AudioClient)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-screenshot)
# The session's secret agents (NetworkManager, BlueZ) and their prompts.
find_package(LingmoClients REQUIRED COMPONENTS networkmanager bluez)
add_library(lingmo-shell-secrets STATIC
    src/shell/secrets/bluetooth_pairing_dialog.cpp
    src/shell/secrets/bluetooth_pairing_dialog.h
    src/shell/secrets/network_dialogs.cpp
    src/shell/secrets/network_dialogs.h
    src/shell/secrets/network_secret_agent.cpp
    src/shell/secrets/network_secret_agent.h
    src/shell/secrets/secret_dialog.cpp
    src/shell/secrets/secret_dialog.h
    src/shell/secrets/secret_prompts.cpp
    src/shell/secrets/secret_prompts.h
    src/shell/secrets/secret_store.cpp
    src/shell/secrets/secret_store.h
    src/shell/secrets/vpn_secret_fields.cpp
    src/shell/secrets/vpn_secret_fields.h
)
target_link_libraries(lingmo-shell-secrets PUBLIC lingmo-shell
    Lingmo::NetworkManagerClient Lingmo::BluezClient)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-secrets)
# The notification banners and the notification centre on their backend.
add_library(lingmo-shell-notification-view STATIC
    src/shell/notification/notification_card.cpp
    src/shell/notification/notification_card.h
    src/shell/notification/notification_centre.cpp
    src/shell/notification/notification_centre.h
    src/shell/notification/notification_controls.cpp
    src/shell/notification/notification_controls.h
    src/shell/notification/notification_motion.cpp
    src/shell/notification/notification_motion.h
    src/shell/notification/notification_popups.cpp
    src/shell/notification/notification_popups.h
    src/shell/notification/notification_ui.cpp
    src/shell/notification/notification_ui.h
)
target_link_libraries(lingmo-shell-notification-view PUBLIC lingmo-shell lingmo-shell-notification)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-notification-view)
# The launcher: its window, grid, search and paging on the launcher's backend.
add_library(lingmo-shell-launcher-view STATIC
    src/shell/launcher/launcher_cell.cpp
    src/shell/launcher/launcher_cell.h
    src/shell/launcher/launcher_metrics.cpp
    src/shell/launcher/launcher_metrics.h
    src/shell/launcher/launcher_search_field.cpp
    src/shell/launcher/launcher_search_field.h
    src/shell/launcher/launcher_tween.h
    src/shell/launcher/launcher_view.cpp
    src/shell/launcher/launcher_view.h
    src/shell/launcher/launcher_wallpaper.cpp
    src/shell/launcher/launcher_wallpaper.h
    src/shell/launcher/launcher_window.cpp
    src/shell/launcher/launcher_window.h
)
target_link_libraries(lingmo-shell-launcher-view PUBLIC lingmo-shell lingmo-shell-launcher
    lingmo-shell-wallpaper)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-launcher-view)
# The control centre, the status panels and the power dialog.
find_package(LingmoClients REQUIRED COMPONENTS networkmanager bluez audio)
add_library(lingmo-shell-controlcenter STATIC
    src/shell/controlcenter/control_center.cpp
    src/shell/controlcenter/control_center.h
    src/shell/controlcenter/control_center_services.h
    src/shell/controlcenter/control_center_view.cpp
    src/shell/controlcenter/control_center_view.h
    src/shell/controlcenter/level_slider.cpp
    src/shell/controlcenter/level_slider.h
    src/shell/controlcenter/panel_style.cpp
    src/shell/controlcenter/panel_style.h
    src/shell/controlcenter/power_dialog.cpp
    src/shell/controlcenter/power_dialog.h
    src/shell/controlcenter/session_services.cpp
    src/shell/controlcenter/session_services.h
    src/shell/controlcenter/status_panel.cpp
    src/shell/controlcenter/status_panel.h
    src/shell/controlcenter/tiles.cpp
    src/shell/controlcenter/tiles.h
    src/shell/statusbar/status_bar_panels.h
)
target_link_libraries(lingmo-shell-controlcenter PUBLIC lingmo-shell lingmo-shell-osd
    lingmo-shell-statusbar lingmo-shell-notification Lingmo::NetworkManagerClient
    Lingmo::BluezClient Lingmo::AudioClient)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-controlcenter)
# The status bar's items, which the lock screen shows too.
add_library(lingmo-shell-bar-items STATIC
    src/shell/statusbar/bar_item.cpp
    src/shell/statusbar/bar_item.h
    src/shell/statusbar/status_bar_panels.h
    src/shell/statusbar/status_icon.cpp
    src/shell/statusbar/status_icon.h
)
target_link_libraries(lingmo-shell-bar-items PUBLIC lingmo-shell lingmo-shell-statusbar)
# The status bar: its window, tray, status icons and clock on the bar's backend.
find_package(LingmoClients REQUIRED COMPONENTS appearance)
add_library(lingmo-shell-statusbar-view STATIC
    src/shell/statusbar/dbus_menu_popup.cpp
    src/shell/statusbar/dbus_menu_popup.h
    src/shell/statusbar/device_usage.cpp
    src/shell/statusbar/device_usage.h
    src/shell/statusbar/status_bar_backdrop.cpp
    src/shell/statusbar/status_bar_backdrop.h
    src/shell/statusbar/status_bar_menu_bar.h
    src/shell/statusbar/status_bar_panels.h
    src/shell/statusbar/status_bar_window.cpp
    src/shell/statusbar/status_bar_window.h
    src/shell/statusbar/status_items_view.cpp
    src/shell/statusbar/status_items_view.h
    src/shell/statusbar/system_tray_view.cpp
    src/shell/statusbar/system_tray_view.h
)
target_link_libraries(lingmo-shell-statusbar-view PUBLIC lingmo-shell lingmo-shell-bar-items
    lingmo-shell-statusbar lingmo-shell-wallpaper Lingmo::AudioClient)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-statusbar-view)
# The status bar's menu bar.
include(src/shell/menubar/menubar.cmake)

# Non-activating on-screen keyboard, using the existing seat/IME dispatch path.
add_library(lingmo-shell-keyboard STATIC
    src/shell/keyboard/keyboard_model.cpp
    src/shell/keyboard/keyboard_view.cpp
    src/shell/keyboard/screen_keyboard.cpp)
target_link_libraries(lingmo-shell-keyboard PUBLIC lingmo-shell)
target_link_libraries(lingmo-aurora PRIVATE lingmo-shell-keyboard)

add_executable(lingmo-screen-keyboard src/shell/keyboard/main.cpp)
target_link_libraries(lingmo-screen-keyboard PRIVATE Lingmo::SDKBase)
install(TARGETS lingmo-screen-keyboard RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
