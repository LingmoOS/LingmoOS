/*
    SPDX-FileCopyrightText: 2016 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef DIALOGSTATETEST_H
#define DIALOGSTATETEST_H

#include <QTest>

#include "lingmoquick/dialog.h"

class DialogStateTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void windowState();

private:
    bool verifyState(LingmoQuick::Dialog *dialog) const;

    LingmoQuick::Dialog *m_dialog;
};

#endif
