# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2020 David Edmundson <davidedmundson@kde.org>
# SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

include(ECMAddTests)

set(SOURCES
    main.cpp
)

ecm_add_test(
    ${SOURCES}
    TEST_NAME ksystemstatstest
    LINK_LIBRARIES Qt::Test ksystemstats_core
)
