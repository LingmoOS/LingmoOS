# SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
# SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie_clarius@yahoo.de>
include(ECMAddTests)

ecm_add_test(datetimerunnertest.cpp TEST_NAME datetimerunnertest LINK_LIBRARIES Qt::Test KF6::Runner)
krunner_configure_test(datetimerunnertest org.kde.datetime)
