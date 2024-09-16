find_package(DtkTools REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-W -Wall -fPIC -fstack-protector-all -z relro -z noexecstack -z now -pie")

# qdbusxml2cpp-fix
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/dbusinterface/)
set(QT_DBUS_INTERFACE_INCLUDE ${PROJECT_BINARY_DIR}/dbusinterface/)

macro(qt5_add_dbus_interface_fix srcs xml class file)
  execute_process(COMMAND ${DTK_XML2CPP} -c ${class} -p ${PROJECT_BINARY_DIR}/dbusinterface/${file} ${PROJECT_SOURCE_DIR}/dbus/interface/${xml})
  list(APPEND ${srcs}
    ${PROJECT_BINARY_DIR}/dbusinterface/${file}.h
    ${PROJECT_BINARY_DIR}/dbusinterface/${file}.cpp
  )
endmacro(qt5_add_dbus_interface_fix)

# qtdbusextended
set(DBUS_INCLUDE ${PROJECT_SOURCE_DIR}/dbus/)
list(APPEND DBUSEXTENDED_SOURCE
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/DBusExtended
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextended.h
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/DBusExtendedAbstractInterface
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextendedabstractinterface.h
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextendedpendingcallwatcher_p.h
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextendedabstractinterface.cpp
  ${PROJECT_SOURCE_DIR}/src/qtdbusextended/dbusextendedpendingcallwatcher.cpp
)

add_subdirectory("dde-session-ctl")
add_subdirectory("dde-session")
