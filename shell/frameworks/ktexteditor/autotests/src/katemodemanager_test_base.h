/*
    SPDX-FileCopyrightText: 2021 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_KATEMODEMANAGER_TEST_BASE_H
#define KTEXTEDITOR_KATEMODEMANAGER_TEST_BASE_H

#include <QObject>

class KateModeManager;

class KateModeManagerTestBase : public QObject
{
    Q_OBJECT
protected:
    KateModeManagerTestBase();

    void wildcardsFindTestData();
    void mimeTypesFindTestData();

    const KateModeManager *const m_modeManager;
};

#endif // KTEXTEDITOR_KATEMODEMANAGER_TEST_BASE_H
