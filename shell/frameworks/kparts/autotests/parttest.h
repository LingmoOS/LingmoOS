/*
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PARTTEST_H
#define PARTTEST_H

#include <QObject>

class PartTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testAutoDeletePart();
    void testAutoDeleteWidget();
    void testNoAutoDeletePart();
    void testNoAutoDeleteWidget();

    void testOpenUrlArguments();
    void testAutomaticMimeType();
    void testEmptyUrlAfterCloseUrl();

    void testToolbarVisibility();
    void testShouldNotCrashAfterDelete();
    void testActivationEvent();
};

#endif /* PARTTEST_H */
