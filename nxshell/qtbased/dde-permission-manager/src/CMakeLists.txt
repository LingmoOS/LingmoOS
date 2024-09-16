set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_FLAGS "-W -Wall -fPIC -fstack-protector-all -z relro -z noexecstack -z now -pie")

find_package(Qt5 REQUIRED COMPONENTS Core DBus Widgets)
find_package(DtkCore REQUIRED)
find_package(DtkWidget REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(Polkit-qt5 REQUIRED IMPORTED_TARGET polkit-qt5-agent-1 polkit-qt5-1)

qt5_add_dbus_adaptor(ADAPTER_SOURCES
                     ../dbus/org.deepin.dde.Permission1.xml
                     impl/permission_service.h
                     PermissionService)
qt5_add_dbus_adaptor(ADAPTER_SOURCES
                     ../dbus/org.deepin.dde.Permission1.Client.xml
                     impl/permission_client.h
                     PermissionClient
                     )
qt5_add_dbus_interfaces(DBUS_INTERFACES
                     ../dbus/org.deepin.dde.Permission1.Client.xml
                     )
set(SRCS
    main.cpp
    settings.h
    settings.cpp
    permissionpolicy.h
    permissionpolicy.cpp
    permissionoption.h
    permissionoption.cpp
    impl/permission_service.h
    impl/permission_service.cpp
    impl/permission_client.h
    impl/permission_client.cpp
    widgets/clientdialog.h
    widgets/clientdialog.cpp
    utils/dconfig_settings.h
    utils/dconfig_settings.cpp
    utils/common.h
    utils/common.cpp
    utils/phase_pm_dconfig.h
    utils/phase_pm_dconfig.cpp
    ../dbus/types/permissions_info.h
    ../dbus/types/permissions_info.cpp
)

add_executable(dde-permission-manager
    ${SRCS}
    ${ADAPTER_SOURCES}
    ${DBUS_INTERFACES}
)

target_link_libraries(dde-permission-manager PUBLIC
    Qt5::Core
    Qt5::DBus
    Qt5::Widgets
    ${DtkWidget_LIBRARIES}
    PkgConfig::Polkit-qt5
)

install(DIRECTORY DESTINATION share/permission/policy)

install(TARGETS dde-permission-manager DESTINATION bin)
