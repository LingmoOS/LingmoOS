
cmake_minimum_required(VERSION 3.0)
project(deepin-diskmanager-service)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


set(APP_RES_DIR "assets")
set(APP_SERVICE "${APP_RES_DIR}/data/com.deepin.diskmanager.service")
set(SYSTEMD_SERVICE "${APP_RES_DIR}/data/diskmanager-daemon.service")
set(APP_CONFIG "${APP_RES_DIR}/data/com.deepin.diskmanager.conf")
set(APP_EDEV_DIR "udev")
set(APP_UDEV_FILES "${APP_EDEV_DIR}/99-diskmanager.rules")
set(APP_USBADD_FILES "${APP_EDEV_DIR}/USBadd.sh")
set(APP_USBREMOVE_FILES "${APP_EDEV_DIR}/USBremove.sh")
set(APP_DEEPIN_DISKMANAGER_SERVICE_BIN "${APP_EDEV_DIR}/deepin-diskmanager-authenticateProxy")
set(APP_POLICY_DIR "policy")
set(APP_POLICY_FILES "${APP_POLICY_DIR}/com.deepin.diskmanager.policy")

# Find the library
find_package(PkgConfig REQUIRED)
find_package(DtkGui REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    Widgets
    DBus
REQUIRED)
find_package(PolkitQt5-1)

set(LINK_LIBS
    Qt5::Core
    Qt5::DBus
    ${DtkCore_LIBRARIES}
    parted
    parted-fs-resize
    PolkitQt5-1::Agent
)

file(GLOB ALL_SOURCES
    "*.cpp"
    "./diskoperation/*.cpp"
    "./diskoperation/luksoperator/*.cpp"
    "./diskoperation/filesystems/*.cpp"
    "./diskoperation/lvmoperator/*.cpp")
file(GLOB ALL_HEADERS
    "*.h"
    "./diskoperation/*.h"
    "./diskoperation/luksoperator/*.h"
    "./diskoperation/filesystems/*.h"
    "./diskoperation/lvmoperator/*.h")

include_directories(../basestruct)
include_directories(../log)
include_directories(diskoperation)
link_directories(${PROJECT_BINARY_DIR})

add_executable(${PROJECT_NAME} ${ALL_SOURCES} ${ALL_HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)
#link_libraries(../basestruct/basestruct)
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS} basestruct)
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS} ddmlog)


set(CMAKE_INSTALL_PREFIX /usr)

# Install files
install(TARGETS ${PROJECT_NAME} DESTINATION lib/deepin-daemon/)
install(FILES ${APP_SERVICE} DESTINATION share/dbus-1/system-services/)
install(FILES ${SYSTEMD_SERVICE} DESTINATION /lib/systemd/system/)
install(FILES ${APP_CONFIG} DESTINATION share/dbus-1/system.d/)
install(PROGRAMS ${APP_USBADD_FILES} DESTINATION libexec/openconnect/)
install(PROGRAMS ${APP_USBREMOVE_FILES} DESTINATION libexec/openconnect/)
install(FILES ${APP_UDEV_FILES} DESTINATION /lib/udev/rules.d/)
install(FILES ${APP_POLICY_FILES} DESTINATION share/polkit-1/actions)
install(PROGRAMS ${APP_DEEPIN_DISKMANAGER_SERVICE_BIN} DESTINATION bin/)
