set(BIN_NAME lightdm-lingmo-greeter-test)

aux_source_directory(. GREETER_TEST)

set(LINGMO_PW_CHECK liblingmo_pw_check.so)

set(GREETER_TEST_SRCS
    ${GREETER_TEST}
    ${transaction_DBUS_SCRS}
    ${GLOBAL_UTILS}
    ${GLOBAL_UTILS_DBUS}
    ${GLOBAL_UTILS_KEYBOARDMONITOR}
    ${WIDGETS}
    ${SESSION_WIDGETS}
    ${AUTHENTICATE}
    ${QT_DBUS_EXTENDED}
    ${DBUS_DATA_TYPES}
    ${PROJECT_SOURCE_DIR}/src/lightdm-lingmo-greeter/loginframe.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-lingmo-greeter/greeterworker.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-lingmo-greeter/logincontent.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-lingmo-greeter/logintipswindow.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-lingmo-greeter/pwqualitymanager.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-lingmo-greeter/passwordlevelwidget.cpp
    ${PROJECT_SOURCE_DIR}/src/lightdm-lingmo-greeter/changepasswordwidget.cpp
)

add_executable(${BIN_NAME}
    ${GREETER_TEST_SRCS}
    ${QRCS}
    ${GREETER_QRCS}
)

target_include_directories(${BIN_NAME} PUBLIC
    ${PAM_INCLUDE_DIR}
    ${DTKWIDGET_INCLUDE_DIR}
    ${DTKCORE_INCLUDE_DIR}
    ${XCB_EWMH_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
    ${QGSettings_INCLUDE_DIRS}
    ${Qt5X11Extras_INCLUDE_DIRS}
    ${Greeter_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src/lightdm-lingmo-greeter
)

target_compile_definitions(lightdm-lingmo-greeter
    PRIVATE ENABLE_SESSION
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt_LIBS}
    ${PAM_LIBRARIES}
    ${XCB_EWMH_LIBRARIES}
    ${DtkWidget_LIBRARIES}
    ${DtkCore_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    ${Qt5X11Extras_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${Greeter_LIBRARIES}
    ${Qt5Test_LIBRARIES}
    ${GTEST_LIBRARIES}
    ${LINGMO_PW_CHECK}
    -lpthread
    -lm
)
