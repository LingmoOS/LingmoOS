include_directories(
    ${CMAKE_BINARY_DIR}/kded
)

qt_add_dbus_interface(OsdInterface ../../osd/org.kde.kscreen.osdService.xml osdservice_interface)

add_executable(osdtest main.cpp
                       ../../common/osdaction.cpp
                       ${OsdInterface}
)

target_link_libraries(osdtest Qt::Core
                              Qt::DBus
                              KF5::I18n
)

add_test(NAME kscreen-kded-osdtest COMMAND osdtest)
ecm_mark_as_test(osdtest)
