# What the shell's components share: the application windows and the task
# list, the installed applications, application starts and the file thread.
add_library(lingmo-shell-common STATIC
    app_launcher.cpp
    app_launcher.h
    application_badges.cpp
    application_badges.h
    applications.cpp
    applications.h
    count_badge.cpp
    count_badge.h
    file_thread.cpp
    file_thread.h
    list_model.h
    mime_icons.cpp
    mime_icons.h
    pending_calls.h
    process_info.cpp
    process_info.h
    sender_filter.h
    synced_list.h
    task_manager.cpp
    task_manager.h
    service_owner.h
    task_windows.h
    text_match.cpp
    text_match.h
    wm_task_windows.cpp
    wm_task_windows.h
)
target_link_libraries(lingmo-shell-common PUBLIC lingmo-core lingmo-wm Lingmo::SDKBase
    Lingmo::SDK PkgConfig::TitleText)
