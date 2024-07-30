/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCONFIGTEST_H
#define KCONFIGTEST_H

#include <QObject>

class KConfigTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testComplex();
    void testInvalid();
    void initTestCase();
    void cleanupTestCase();

private:
    bool m_fontFromStringBug;
};

#endif /* KCONFIGTEST_H */
