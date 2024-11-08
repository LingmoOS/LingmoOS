# SPDX-FileCopyrightText: 2024 LingmoSoft Co., Ltd.
# SPDX-FileCopyrightText: 2024 Hongfei Shang
#
# SPDX-License-Identifier: MulanPSL-2.0

install(FILES
    protocols/lingmo-shell.xml
    protocols/lingmo-window-management.xml

    DESTINATION ${CMAKE_INSTALL_DATADIR}/lingmo-wayland-protocols
)
