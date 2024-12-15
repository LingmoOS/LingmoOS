# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

#设置包含目录
include_directories(${PROJECT_SOURCE_DIR}/src/include/ui
                    ${PROJECT_SOURCE_DIR}/src/include/ui/mainFrame
                    ${PROJECT_SOURCE_DIR}/src/include/ui/settings
                    ${PROJECT_SOURCE_DIR}/src/include/ui/websocket
                    ${PROJECT_SOURCE_DIR}/src/include/log
                    ${PROJECT_SOURCE_DIR}/src/include/downloader
                    ${CMAKE_BINARY_DIR}
)
link_directories(${CMAKE_BINARY_DIR}/lib)

#需要moc(加Q_OBJECT)的头文件
set (DWM_EXTENSIONSERVICE_HEADERS
    ${PROJECT_SOURCE_DIR}/src/include/downlaoder/accessiblewidget.h
)

#设置输入执行文件名称
add_executable(downloader main.cpp
    accessiblewidget.cpp
)

target_link_libraries(downloader dlmcontrolui dlmlog Qt5::Core)

#INSTALL(TARGETS downloader DESTINATION share/downloader)
#install(TARGETS downloader DESTINATION bin)

