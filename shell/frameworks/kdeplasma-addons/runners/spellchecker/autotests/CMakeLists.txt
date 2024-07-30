# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Fushan Wen <qydwhotmail@gmail.com>

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_test(spellcheckrunnertest.cpp TEST_NAME spellcheckrunnertest
    LINK_LIBRARIES
    Qt::Test
    krunner_spellcheck_static
)
