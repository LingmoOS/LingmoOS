set(Blackwidget_Name ocean-blackwidget)
file(GLOB_RECURSE Blackwidget_SRCS
    "src/*.h"
    "src/*.cpp"
)

add_executable(${Blackwidget_Name}
    ${Blackwidget_SRCS}
)
set(Blackwidget_Includes
    ${GSETTINGS_INCLUDE_DIRS}
)
set(Blackwidget_Libraries
    ${GSETTINGS_LIBRARIES}
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
)

target_include_directories(${Blackwidget_Name} PUBLIC
    ${Blackwidget_Includes}
)

target_link_libraries(${Blackwidget_Name} PRIVATE
    ${Blackwidget_Libraries}
)

## bin
install(TARGETS ${Blackwidget_Name} DESTINATION lib/lingmo-daemon)
## service
install(FILES misc/dbus-services/org.lingmo.ocean.BlackScreen1.service DESTINATION share/dbus-1/services)
install(FILES misc/systemd/user/ocean-blackwidget.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})
