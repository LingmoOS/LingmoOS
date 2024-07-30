# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Tomaz Canabrava <tcanabrava@kde.org>

add_executable(ss_tests
    main.cpp
)

target_link_libraries(ss_tests
    Qt::Core
    kcm_firewall_core
)
