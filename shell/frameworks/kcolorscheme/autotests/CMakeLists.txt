# SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>
# SPDX-License-Identifier: BSD-2-Clause

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

# remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

ecm_add_test(kcolorschemetest.cpp LINK_LIBRARIES Qt6::Test KF6::ColorScheme)

