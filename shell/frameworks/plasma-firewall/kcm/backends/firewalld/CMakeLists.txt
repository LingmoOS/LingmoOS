# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Lucas Biaggi <lbjanuario@gmail.com>
set(firewalld_backend_SRCS
    dbustypes.cpp
    dbustypes.h
    firewalldclient.cpp
    firewalldclient.h
    firewalldjob.cpp
    firewalldjob.h
    firewalldlogmodel.cpp
    firewalldlogmodel.h
    queryrulesfirewalldjob.cpp
    queryrulesfirewalldjob.h
)

kcoreaddons_add_plugin(
    firewalldbackend
    SOURCES ${firewalld_backend_SRCS}
    INSTALL_NAMESPACE "kf6/plasma_firewall")

target_link_libraries(firewalldbackend
    kcm_firewall_core
    Qt::DBus
)
