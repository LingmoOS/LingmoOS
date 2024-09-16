# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(BIN_NAME reset-password-dialog)

if(NOT DISABLE_SYS_UPDATE)

set(SRCS
        main.cpp
        utils.h
        pwqualitymanager.h
        pwqualitymanager.cpp
        resetpassworddialog.h
        resetpassworddialog.cpp
        securityquestionswidget.h
        securityquestionswidget.cpp
        unionidwidget.h
        unionidwidget.cpp
        passwordwidget.h
        passwordwidget.cpp
        resetpasswordworker.h
        resetpasswordworker.cpp
)
set(QRC resetpassworddialog.qrc)
# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Concurrent REQUIRED)

find_package(DtkWidget REQUIRED)
find_package(DtkGui REQUIRED)

pkg_check_modules(DeepinPwCheck REQUIRED IMPORTED_TARGET libdeepin_pw_check)


add_executable(${BIN_NAME} ${SRCS} ${QRC})
target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(${BIN_NAME} PRIVATE
    session-ui-dbus-shared
    ${DtkWidget_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Network_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Concurrent_LIBRARIES}
    PkgConfig::DeepinPwCheck
    crypt
    ${LIBS}
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION lib/dde-control-center/)

endif()
