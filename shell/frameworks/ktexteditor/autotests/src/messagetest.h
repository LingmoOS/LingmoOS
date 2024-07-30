/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTEXTEDITOR_MESSAGE_TEST_H
#define KTEXTEDITOR_MESSAGE_TEST_H

#include <QObject>

class MessageTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void testPostMessage();
    void testAutoHide();
    void testAutoHideAfterUserInteraction();
    void testMessageQueue();
    void testPriority();
    void testCreateView();
    void testHideView();
    void testHideViewAfterUserInteraction();
};

#endif // KTEXTEDITOR_MESSAGE_TEST_H
