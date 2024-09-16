# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

find_package(Qt${QT_VERSION_MAJOR} ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Core Gui Quick)

add_subdirectory(applet-example)
add_subdirectory(applet-example-data)
add_subdirectory(containment-example)
add_subdirectory(panel-example)
add_subdirectory(layershell-example)
add_subdirectory(osd-example)
add_subdirectory(drag-example)
