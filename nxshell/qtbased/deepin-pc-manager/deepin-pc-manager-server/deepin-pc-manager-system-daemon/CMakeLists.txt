# SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set(BIN_NAME deepin-pc-manager-system-daemon)

# 设置包含当前目录，建议头文件的包含写全路径
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# 若目标库安装了.cmake文件，则可以直接使用find_package
find_package(PkgConfig REQUIRED)
# 若目标库未安装.cmake文件，但是安装了.pc文件，则可以使用pkgconfig
pkg_check_modules(GSettings REQUIRED IMPORTED_TARGET gsettings-qt)
pkg_check_modules(LibParted REQUIRED IMPORTED_TARGET libparted)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Sql DBus Concurrent)
find_package(Dtk REQUIRED COMPONENTS Core Widget)

# 建议对不同的模块进行分类，可使用通配符，可指定具名文件
file(GLOB_RECURSE SRCS
    "disk/*.h"
    "disk/*.cpp"
    "localcache/*.h"
    "localcache/*.cpp"
    "netcheck/*.h"
    "netcheck/*.cpp"
    "main.cpp"
    "writedbusdata.cpp"
    "writedbusdata.h"
    "daemon_adaptor.cpp"
    "daemon_adaptor.h"
    "../../deepin-pc-manager/src/window/modules/common/*.cpp"
    "../../deepin-pc-manager/src/window/modules/common/*.h"
)

# 生成可执行文件
add_executable(${BIN_NAME}
    ${SRCS}
)

# 这里针对当前二进制定义了一个宏，保存着版本号，方便程序代码中调用，若程序中需要自定义宏变量并且在代码中使用，可参考此方法。
# 注意CMAKE_PROJECT_XXX系列变量，某些属性是cmake帮助提供，但某些属性是project方法设置的，所以这里需要注意变量是否可用。
# 至于是使用add_compile_xxx还是target_compile_xxx，视情况而定，一个是指定所有对象，一个是指定具体的二进制
# target_compile_definitions(${BIN_NAME} PRIVATE
#     VERSION="${CMAKE_PROJECT_VERSION}"
# )

# Qt 从5.15版本开始，可以直接使用Qt::Core，而不需要加版本号，但为了兼容性，把版本号加上为好
target_include_directories(${BIN_NAME} PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Sql
  Qt${QT_VERSION_MAJOR}::DBus
  Qt${QT_VERSION_MAJOR}::Concurrent
  Dtk::Core
  Dtk::Widget
  PkgConfig::GSettings
  PkgConfig::LibParted
  ../../deepin-pc-manager/
  ../../deepin-pc-manager/src/
)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Sql
  Qt${QT_VERSION_MAJOR}::DBus
  Qt${QT_VERSION_MAJOR}::Concurrent
  Dtk::Core
  Dtk::Widget
  PkgConfig::GSettings
  PkgConfig::LibParted
)

# 指定安装目录，一般常用有3个： TARGETS（编译出来的目标二进制）、FILES（指定路径的文件，通常是配置文件或服务文件）、DIRECTORY（一般是配置文件较多时使用）。
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES com.deepin.pc.manager.system.daemon.conf DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system.d)
install(FILES com.deepin.pc.manager.system.daemon.service DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system-services)
install(FILES config/netcheck/com.deepin.pc.manager.netcheck.conf DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system.d)
install(FILES config/netcheck/com.deepin.pc.manager.netcheck.service DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/dbus-1/system-services)
install(FILES deepin-pc-manager-system-daemon.service DESTINATION /lib/systemd/system)
