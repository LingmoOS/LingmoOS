/*
 *  SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KMULTITABBARTEST_H
#define KMULTITABBARTEST_H

#include <QObject>

class KMultiTabBarTest : public QObject
{
    Q_OBJECT

public:
    KMultiTabBarTest(QObject *parent = nullptr);

private Q_SLOTS:
    void testTabsAppendAndRemove();
    void testTabStyleChanged();
    void testTabRaised();
    void shouldEmitClicked();
};

#endif
