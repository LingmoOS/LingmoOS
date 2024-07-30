/*
    SPDX-FileCopyrightText: 2013 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef KICONUTILSTEST_H
#define KICONUTILSTEST_H

#include <QObject>

class KIconUtilsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void addOverlayTest();
    void addOverlaysTest();
};

#endif // KICONUTILSTEST_H
