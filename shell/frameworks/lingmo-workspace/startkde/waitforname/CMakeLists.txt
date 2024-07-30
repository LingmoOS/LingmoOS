
set(lingmo_waitforname_SRCS
    waiter.cpp
    waiter.h
    main.cpp
   )

ecm_qt_declare_logging_category(lingmo_waitforname_SRCS HEADER debug_p.h IDENTIFIER LOG_LINGMO CATEGORY_NAME org.kde.lingmo.waitforname
    DESCRIPTION "lingmo waitforname"
    EXPORT LINGMOWORKSPACE
)

add_executable(lingmo_waitforname ${lingmo_waitforname_SRCS})
ecm_mark_nongui_executable(lingmo_waitforname)

target_link_libraries(lingmo_waitforname
    Qt::DBus
   )

configure_file(org.kde.lingmo.Notifications.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lingmo.Notifications.service)

configure_file(org.kde.KSplash.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KSplash.service)

install(TARGETS lingmo_waitforname ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.lingmo.Notifications.service
        DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KSplash.service
        DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
