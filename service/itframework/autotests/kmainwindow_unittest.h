/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KMAINWINDOW_UNITTEST_H
#define KMAINWINDOW_UNITTEST_H

#include <QObject>

class KMainWindow_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testDefaultName();
    void testFixedName();
    void testNameWithSpecialChars();
    void testNameWithHash();
    void testSaveWindowSize();
    void testSaveWindowSizeInStateConfig();
    void testAutoSaveSettings();
    void testNoAutoSave();
    void testWidgetWithStatusBar();

    void testDeleteOnClose();
};

#endif
