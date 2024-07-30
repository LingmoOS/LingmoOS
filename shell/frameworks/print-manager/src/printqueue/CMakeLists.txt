set(queuemanager_SRCS
    main.cpp
    PrintQueue.cpp
    PrintQueueUi.cpp
    PrintQueue.h
    PrintQueueUi.h
)

ki18n_wrap_ui(queuemanager_SRCS
    PrintQueueUi.ui
)

add_executable(kde-print-queue
    ${queuemanager_SRCS}
)

target_link_libraries(kde-print-queue
    Qt::Core
    Qt::Widgets
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::DBusAddons
    KF6::ConfigCore
    kcupslib
)

install(TARGETS kde-print-queue DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.PrintQueue.desktop DESTINATION ${KDE_INSTALL_APPDIR})
