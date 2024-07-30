/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "../src/lib/zxingutil_p.h"

#include <QDebug>
#include <QObject>
#include <QTest>

using namespace Prison;

class ZXingUtilTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testToStdWString()
    {
        auto w = ZXingUtil::toStdWString(QStringLiteral("KDE"));
        QCOMPARE(w.size(), 3);
        QCOMPARE(w[0], 'K');

        w = ZXingUtil::toStdWString(QByteArray("KDE"));
        QCOMPARE(w.size(), 3);
        QCOMPARE(w[0], 'K');

        w = ZXingUtil::toStdWString(QByteArray("\x80\x00\x7f", 3));
        QCOMPARE(w.size(), 3);
        QCOMPARE(w[0], 0x80);
        QCOMPARE(w[1], 0x00);
        QCOMPARE(w[2], 0x7f);
    }
};

QTEST_APPLESS_MAIN(ZXingUtilTest)

#include "zxingutiltest.moc"
