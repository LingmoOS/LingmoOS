// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include <QDebug>
#include <QTest>

#include <KSycoca>
#include <device.h>

class DeviceTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDBusName()
    {
        QString udi("udi-pass");
        Device dev("//superinvalid//udi/-/", "producti", "/dev/yo");
        QCOMPARE(dev.objectName(), "__superinvalid__udi___");
    }
};

QTEST_MAIN(DeviceTest)

#include "devicetest.moc"
