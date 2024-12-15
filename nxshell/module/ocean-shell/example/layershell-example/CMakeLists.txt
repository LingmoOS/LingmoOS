# SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

add_executable(layershell-example main.cpp qml.qrc)
target_link_libraries(layershell-example PRIVATE Qt::Gui Qt::Qml)
