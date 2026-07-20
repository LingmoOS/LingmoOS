set(SRCS main.cpp
         application.cpp
         datehelper.cpp
         screenhelper.cpp
         notificationsmodel.cpp
         notificationserver.cpp
         notification.cpp
         notificationpopup.cpp
         notificationwindow.cpp
         historymodel.cpp
         settings.cpp
         utils.cpp
         dbus/notificationsadaptor.cpp
         resources.qrc
)

# for Ubuntu
# qt_add_dbus_adaptor(SRCS org.freedesktop.Notifications.xml notificationserver.h NotificationServer)

qt_add_dbus_adaptor(DBUS_SOURCES com.lingmo.Notification.xml application.h Application)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

find_package(KF6WindowSystem)

add_executable(lingmo-notificationd ${SRCS} ${DBUS_SOURCES})

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt6_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

target_link_libraries(lingmo-notificationd
    Qt6::Core
    Qt6::DBus
    Qt6::Quick
    Qt6::Widgets
    KF6::WindowSystem
)

install(TARGETS lingmo-notificationd
    DESTINATION /usr/bin
    COMPONENT Runtime
)

add_custom_target(notificationd_translations DEPENDS ${QM_FILES} SOURCES ${ts_files_replaced})
add_dependencies(lingmo-notificationd notificationd_translations)

install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-notificationd/translations)
