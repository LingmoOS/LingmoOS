set(BIN_NAME dde-lock-test)

aux_source_directory(. LOCK_TEST)

set(LOCK_TEST_SRCS
    ${LOCK_TEST}
    ${GLOBAL_UTILS}
    ${GLOBAL_UTILS_DBUS}
    ${GLOBAL_UTILS_KEYBOARDMONITOR}
    ${WIDGETS}
    ${SESSION_WIDGETS}
    ${AUTHENTICATE}
    ${QT_DBUS_EXTENDED}
    ${DBUS_DATA_TYPES}
    ${PROJECT_SOURCE_DIR}/src/dde-lock/lockframe.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/lockworker.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus/dbuslockagent.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus/dbuslockfrontservice.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus/dbusshutdownagent.cpp
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus/dbusshutdownfrontservice.cpp
)

add_executable(${BIN_NAME}
    ${LOCK_TEST_SRCS}
    ${QRCS}
    ${LOCK_QRCS}
)

target_include_directories(${BIN_NAME} PUBLIC
    ${DTKWIDGET_INCLUDE_DIR}
    ${DTKCORE_INCLUDE_DIR}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${QGSettings_INCLUDE_DIRS}
    ${PAM_INCLUDE_DIR}
    ${XCB_EWMH_INCLUDE_DIRS}
    ${Qt5X11Extras_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ${PROJECT_SOURCE_DIR}/src/dde-lock
    ${PROJECT_SOURCE_DIR}/src/dde-lock/dbus
    ${Greeter_INCLUDE_DIRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt5Test_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${Qt_LIBS}
    ${PAM_LIBRARIES}
    ${XCB_EWMH_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Greeter_LIBRARIES}
    -lpthread
    -lm
)
