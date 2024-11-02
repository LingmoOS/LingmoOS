cmake_minimum_required(VERSION 3.14)

project(lingmo-process-manager LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DQT_NO_KEYWORDS)

find_package(PkgConfig REQUIRED)

find_package(KF5KIO)

find_package(KF5WindowSystem)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Concurrent DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Concurrent DBus)

pkg_check_modules(GSETTINGQT REQUIRED gsettings-qt)
pkg_check_modules(LINGMOSDKWAYLANDHELPER_PKG lingmosdk-waylandhelper)
pkg_check_modules(GIO REQUIRED gio-unix-2.0)
pkg_check_modules(DATACOLLECT lingmosdk-datacollect)

file(GLOB_RECURSE BASE_FILE CMAKE_CONFIGURE_DEPENDS
    utils/misc.h
    utils/misc.cpp
    base/systemddbusinterface.h
    base/systemddbusinterface.cpp)

add_subdirectory(autotests)
add_subdirectory(daemon)

include_directories(${GIO_INCLUDE_DIRS})
include_directories(base utils core)

set(BASE_SOURCE
    base/schedpolicy.h
    base/schedpolicy.cpp
    base/appinfo.h
    base/appinfo.cpp
    base/singleton.h
    base/timewheel.h
    base/timewheel.cpp
    base/daemondbusinterface.h
    base/daemondbusinterface.cpp
    base/systemddbusinterface.h
    base/systemddbusinterface.cpp
    base/profileattribute.h
    base/profileattribute.cpp
    base/profileaction.h
    base/profileaction.cpp
    base/resourcecontroller.h
    base/resourcecontroller.cpp
    base/groupmanagementunit.h
    base/groupmanagementunit.cpp
    base/notifydbusinterface.h
    base/notifydbusinterface.cpp
)

set(UTILS_SOURCE
    utils/processinfohelper.h
    utils/processinfohelper.cpp
    utils/misc.h
    utils/misc.cpp
    utils/appinfohelper.h
    utils/appinfohelper.cpp
)

set(CORE_SOURCE
    core/desktopfilemanager.h
    core/desktopfilemanager.cpp
    core/configmanager.h
    core/configmanager.cpp
    core/appinfomanager.h
    core/appinfomanager.cpp
    core/exceptiongroupprocesswatcher.h
    core/exceptiongroupprocesswatcher.cpp
    core/processinfomanager.h
    core/processinfomanager.cpp
    core/eventwatcher.h
    core/eventwatcher.cpp
    core/resourcemanagerinterface.h
    core/resourcemanagerinterface.cpp
    core/groupmanager.h
    core/groupmanager.cpp
    core/schedpolicymanager.h
    core/schedpolicymanager.cpp
    core/devicestate.h
    core/devicestate.cpp
    core/multimediacontroller.h
    core/multimediacontroller.cpp
    core/appwhitelist.h
    core/appwhitelist.cpp
    core/applauncher.h
    core/applauncher.cpp
    core/appchooser.h
    core/appchooser.cpp
    core/applaunchmanager.h
    core/applaunchmanager.cpp
    core/systemenery/systemenergy.h
    core/systemenery/upowersystemenery.h
    core/systemenery/upowersystemenery.cpp
    core/energymeter/energymeter.h
    core/energymeter/cpuenergymeter.h
    core/energymeter/cpuenergymeter.cpp
    core/consumptionmonitor/processgroupconsumptionmonitor.h
    core/consumptionmonitor/processgroupconsumptionmonitor.cpp
    core/batteryremainingtimeestimator/batteryremainingtimeestimator.h
    core/batteryremainingtimeestimator/batteryremainingtimeestimator.cpp
    core/datacollector.h
    core/datacollector.cpp
    core/resourcewarninghandler.h
    core/resourcewarninghandler.cpp
    core/systemnotifiessender.h
    core/systemnotifiessender.cpp
    core/selection/selectionchecker.h
    core/selection/selectionchecker.cpp
)

qt5_add_dbus_adaptor(DBUS_SOURCE
    configs/com.lingmo.ProcessManager.xml
    processmanager.h
    ProcessManager
    processmanagerservice
    ProcessManagerService)

qt5_add_dbus_adaptor(DBUS_SOURCE
    configs/com.lingmo.ProcessManager.AppWhitelist.xml
    appwhitelist.h
    AppWhitelist
    appwhitelistservice
    AppWhitelistService)

qt5_add_dbus_adaptor(DBUS_SOURCE
    configs/com.lingmo.ProcessManager.AppLauncher.xml
    applaunchmanager.h
    AppLaunchManager
    applaunchmanagerservice
    AppLaunchManagerService)

set(TS_FILES
    ./translations/zh_CN.ts)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${REQUIRED_LIBS} LinguistTools REQUIRED)
qt5_create_translation(QM_FILES ${TS_FILES} ${CORE_SOURCE} ${UTILS_SOURCE} ${BASE_SOURCE} ${DBUS_SOURCE})

add_executable(
    lingmo-process-manager
    processmanager.h
    processmanager.cpp
    main.cpp
    ${DBUS_SOURCE}
    ${BASE_SOURCE}
    ${UTILS_SOURCE}
    ${CORE_SOURCE}
    ${TS_FILES}
    ${QM_FILES}
)

target_include_directories(lingmo-process-manager PRIVATE ${LINGMOSDKWAYLANDHELPER_PKG_INCLUDE_DIRS})
target_link_directories(lingmo-process-manager PRIVATE ${LINGMOSDKWAYLANDHELPER_PKG_LIBRARY_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${DATACOLLECT_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${DATACOLLECT_LIBRARY_DIRS})

target_link_libraries(
    lingmo-process-manager
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::DBus
    KF5::KIOCore
    KF5::WindowSystem
    ${GSETTINGQT_LIBRARIES}
    ${LINGMOSDKWAYLANDHELPER_PKG_LIBRARIES}
    ${DATACOLLECT_LIBRARIES}
    ${GIO_LIBRARIES}
    jsoncpp
    pthread
    procps
    X11
)

install(TARGETS
    lingmo-process-manager
    DESTINATION /usr/bin)
install(FILES
    configs/org.lingmo.ProcessManager.gschema.xml
    configs/org.lingmo.ProcessManager.DefaultGroupApps.gschema.xml
    configs/org.lingmo.ProcessManager.SoftFreezeMode.gschema.xml
    DESTINATION /usr/share/glib-2.0/schemas)
install(FILES
    configs/task-profiles-v1.json
    configs/task-profiles-v2.json
    configs/lingmo-process-manager.json
    DESTINATION /etc/lingmo-process-manager)
install(FILES
    configs/com.lingmo.ProcessManager.service
    DESTINATION /usr/share/dbus-1/services)
install(FILES ${QM_FILES}
    DESTINATION /usr/share/lingmo-process-manager/)
install(FILES
    configs/lingmo-process-manager.service
    DESTINATION /usr/lib/systemd/user)
