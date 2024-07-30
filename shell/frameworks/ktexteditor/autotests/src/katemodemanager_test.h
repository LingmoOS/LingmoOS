/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_KATEMODEMANAGER_TEST_H
#define KTEXTEDITOR_KATEMODEMANAGER_TEST_H

#include "katemodemanager_test_base.h"

class KateModeManagerTest : public KateModeManagerTestBase
{
    Q_OBJECT
private Q_SLOTS:
    void testWildcardsFind_data();
    void testWildcardsFind();
    void testMimeTypesFind_data();
    void testMimeTypesFind();
};

#endif // KTEXTEDITOR_KATEMODEMANAGER_TEST_H
