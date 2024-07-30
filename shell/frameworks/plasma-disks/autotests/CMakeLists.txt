# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Test)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_test(smartdatatest.cpp TEST_NAME smartdatatest
    LINK_LIBRARIES Qt6::Test statickdedsmart)

ecm_add_test(smartmonitortest.cpp TEST_NAME smartmonitortest
    LINK_LIBRARIES Qt6::Test statickdedsmart)

ecm_add_test(devicetest.cpp TEST_NAME devicetest
    LINK_LIBRARIES Qt6::Test statickdedsmart)
