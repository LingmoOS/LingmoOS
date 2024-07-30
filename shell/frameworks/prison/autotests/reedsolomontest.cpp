/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "../src/lib/bitvector_p.h"
#include "../src/lib/reedsolomon_p.h"

#include <QDebug>
#include <QObject>
#include <QTest>

Q_DECLARE_METATYPE(Prison::BitVector)

using namespace Prison;

class ReedSolomonTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void rsTest_data()
    {
        QTest::addColumn<int>("poly");
        QTest::addColumn<int>("symCount");
        QTest::addColumn<BitVector>("input");
        QTest::addColumn<BitVector>("output");

        BitVector in;
        BitVector out;
        out.appendMSB(0, 20);
        QTest::newRow("empty") << (int)ReedSolomon::GF16 << 5 << in << out;

        in.clear();
        out.clear();
        in.appendMSB(0x5c, 8);
        out.appendMSB(7, 6);
        out.appendMSB(5, 7);
        out.appendMSB(0x4d, 7);
        QTest::newRow("GF16") << (int)ReedSolomon::GF16 << 5 << in << out;
    }

    void rsTest()
    {
        QFETCH(int, poly);
        QFETCH(int, symCount);
        QFETCH(BitVector, input);
        QFETCH(BitVector, output);

        ReedSolomon rs(poly, symCount);
        const auto res = rs.encode(input);
        QCOMPARE(res.size(), output.size());
        if (res != output) {
            qDebug() << "Actual  :" << res;
            qDebug() << "Expected:" << output;
        }
        QCOMPARE(res, output);
    }
};

QTEST_APPLESS_MAIN(ReedSolomonTest)

#include "reedsolomontest.moc"
