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
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus Network Concurrent)
find_package(Dtk${DTK_VERSION_MAJOR} REQUIRED COMPONENTS Widget Gui)

pkg_check_modules(LingmoPwCheck REQUIRED IMPORTED_TARGET liblingmo_pw_check)


add_executable(${BIN_NAME} ${SRCS} ${QRC})
target_include_directories(${BIN_NAME} PUBLIC
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Gui
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(${BIN_NAME} PRIVATE
    session-ui-dbus-shared
    Dtk${DTK_VERSION_MAJOR}::Widget
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Concurrent
    PkgConfig::LingmoPwCheck
    crypt
    ${LIBS}
)

# bin
install(TARGETS ${BIN_NAME} DESTINATION lib/ocean-control-center/)

endif()
