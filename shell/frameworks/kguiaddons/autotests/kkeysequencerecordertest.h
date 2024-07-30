/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
*/

#ifndef KKEYSEQUENCERECORDERTEST_H
#define KKEYSEQUENCERECORDERTEST_H

#include <QObject>

class QWindow;

class KKeySequenceRecorderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testValidWindow();
    void testRecording();
    void testModifiers();
    void testModifierless();
    void testModifierOnly();
    void testModifierOnlyMultiple();
    void testModifierOnlyMultipleInterrupt();
    void testModifierOnlyDisabled();
    void testMultiKeyAllowed();
    void testKeyNonLetterNoModifier();

private:
    QWindow *m_window;
};

#endif
