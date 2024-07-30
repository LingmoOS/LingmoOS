# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Tomaz Canabrava <tcanabrava@kde.org>

add_executable(firewall_tests
    main.cpp
)

target_link_libraries(firewall_tests
    Qt::Core
    kcm_firewall_core
)
