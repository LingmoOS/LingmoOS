# The dock without its view: its applications, pins, minimized windows,
# trash and settings, and org.lingmo.Dock.
add_library(lingmo-shell-dock STATIC
    dock.cpp
    dock.h
    dock_model.cpp
    dock_model.h
    dock_settings.cpp
    dock_settings.h
    minimized_windows.cpp
    minimized_windows.h
    trash_monitor.cpp
    trash_monitor.h
)
target_link_libraries(lingmo-shell-dock PUBLIC lingmo-shell-common)
