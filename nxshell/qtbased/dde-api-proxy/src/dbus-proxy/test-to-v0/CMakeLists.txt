set(PRO_NAME "dde-api-dbus-proxy-test-to-v0")

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/dbus-proxy/dbusinterface/v0/session/)
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/dbus-proxy/dbusinterface/v0/system/)
list(APPEND QT_DBUS_PROXY_INTERFACE_INCLUDE
    ${PROJECT_BINARY_DIR}/dbus-proxy/dbusinterface/v0/session/
    ${PROJECT_BINARY_DIR}/dbus-proxy/dbusinterface/v0/system/
)

file(GLOB_RECURSE PROXY_DBUS_TYPES ${DBUS_FILE_PATH}/types/*)

# dbus interface
qt5_add_dbus_proxy_interface_fix(PROXY_INTERFACE_SOURCES v0/session/com.deepin.daemon.SoundEffect.xml SoundEffect v0/session/com_deepin_daemon_SoundEffect)
qt5_add_dbus_proxy_interface_fix(PROXY_INTERFACE_SOURCES v0/session/com.deepin.dbusdemo.xml DbusDemo v0/session/com_deepin_dbusdemo)
# test
qt5_add_dbus_proxy_interface_fix(PROXY_INTERFACE_SOURCES v0/system/com.deepin.daemon.Accounts.xml System_Accounts v0/system/com_deepin_daemon_Accounts)
qt5_add_dbus_proxy_interface_fix(PROXY_INTERFACE_SOURCES v0/system/com.deepin.daemon.Accounts.User.xml System_AccountsUser v0/system/com_deepin_daemon_Accounts_User)

add_executable(${PRO_NAME}
    main.cpp
    ${PROXY_INTERFACE_SOURCES}
    ${PROXY_DBUS_TYPES}
)

target_link_libraries(${PRO_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    ${DtkCore_LIBRARIES}
 )

target_include_directories(${PRO_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROXY_DBUSEXTENDED_INCLUDE}
    ${DBUS_PROXY_SOURCE_PATH}/dbus/
    ${DBUS_PROXY_SOURCE_PATH}/common/
    ${QT_DBUS_PROXY_INTERFACE_INCLUDE}
    ${DBUS_FILE_PATH}
)
