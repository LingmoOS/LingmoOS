/*
    SPDX-FileCopyrightText: 2018 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "../src/lib/bitvector_p.h"
#include "../src/lib/code128barcode_p.h"

#include <prison.h>

#include <QObject>
#include <QTest>

Q_DECLARE_METATYPE(Prison::BitVector)

using namespace Prison;

class Code128BarcodeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEncode_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<BitVector>("output");

        BitVector v;
        QTest::newRow("empty") << QByteArray() << v;
        v.appendMSB(1680, 11);
        v.appendMSB(1554, 11);
        v.appendMSB(1062, 11);
        v.appendMSB(1424, 11);
        v.appendMSB(1220, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("all lower") << QByteArray("kde") << v;
        v.clear();
        v.appendMSB(1680, 11);
        v.appendMSB(1422, 11);
        v.appendMSB(1416, 11);
        v.appendMSB(1128, 11);
        v.appendMSB(1764, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("all uppper") << QByteArray("KDE") << v;

        v.clear();
        v.appendMSB(1680, 11);
        v.appendMSB(1614, 11);
        v.appendMSB(1764, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("1 digit") << QByteArray("4") << v;
        v.clear();
        v.appendMSB(1692, 11);
        v.appendMSB(1464, 11);
        v.appendMSB(1134, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("2 digits") << QByteArray("42") << v;
        v.clear();
        v.appendMSB(1680, 11);
        v.appendMSB(1614, 11);
        v.appendMSB(1650, 11);
        v.appendMSB(1650, 11);
        v.appendMSB(1124, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("3 digits") << QByteArray("422") << v;
        v.clear();
        v.appendMSB(1692, 11);
        v.appendMSB(1464, 11);
        v.appendMSB(1902, 11);
        v.appendMSB(1782, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("4 digits") << QByteArray("4223") << v;

        v.clear();
        v.appendMSB(1680, 11);
        v.appendMSB(1814, 11);
        v.appendMSB(1260, 11);
        v.appendMSB(1260, 11);
        v.appendMSB(1896, 11);
        v.appendMSB(1814, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("mixed") << QByteArray("X00Y") << v;

        v.clear();
        v.appendMSB(1668, 11);
        v.appendMSB(1292, 11);
        v.appendMSB(1292, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("null") << QByteArray("\0", 1) << v;
        v.clear();
        v.appendMSB(1668, 11);
        v.appendMSB(1422, 11);
        v.appendMSB(1416, 11);
        v.appendMSB(1128, 11);
        v.appendMSB(1292, 11);
        v.appendMSB(1412, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("Code A only") << QByteArray("KDE\0", 4) << v;

        v.clear();
        v.appendMSB(1668, 11);
        v.appendMSB(1292, 11);
        v.appendMSB(1518, 11);
        v.appendMSB(1554, 11);
        v.appendMSB(1062, 11);
        v.appendMSB(1424, 11);
        v.appendMSB(1616, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("Start A -> Latch B") << QByteArray("\0kde", 4) << v;
        v.clear();
        v.appendMSB(1680, 11);
        v.appendMSB(1554, 11);
        v.appendMSB(1062, 11);
        v.appendMSB(1886, 11);
        v.appendMSB(1292, 11);
        v.appendMSB(1292, 11);
        v.appendMSB(1602, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("Start B -> Latch A") << QByteArray("kd\0\0", 4) << v;

        v.clear();
        v.appendMSB(1668, 11);
        v.appendMSB(1292, 11);
        v.appendMSB(1954, 11);
        v.appendMSB(1118, 11);
        v.appendMSB(1590, 11);
        v.appendMSB(1292, 11);
        v.appendMSB(1328, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("Start A -> Shift B") << QByteArray("\0~@\0", 4) << v;
        v.clear();
        v.appendMSB(1680, 11);
        v.appendMSB(1974, 11);
        v.appendMSB(1954, 11);
        v.appendMSB(1292, 11);
        v.appendMSB(1310, 11);
        v.appendMSB(1844, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("Start B -> Shift A") << QByteArray("{\0}", 3) << v;

        v.clear();
        v.appendMSB(1692, 11);
        v.appendMSB(1436, 11);
        v.appendMSB(1112, 11);
        v.appendMSB(1518, 11);
        v.appendMSB(1304, 11);
        v.appendMSB(1112, 11);
        v.appendMSB(1158, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("Start C -> Latch B") << QByteArray("1234AB") << v;
        v.clear();
        v.appendMSB(1680, 11);
        v.appendMSB(1304, 11);
        v.appendMSB(1112, 11);
        v.appendMSB(1502, 11);
        v.appendMSB(1436, 11);
        v.appendMSB(1112, 11);
        v.appendMSB(1966, 11);
        v.appendMSB(6379, 13);
        QTest::newRow("Start B -> Latch C") << QByteArray("AB1234") << v;
    }

    void testEncode()
    {
        QFETCH(QByteArray, input);
        QFETCH(BitVector, output);

        Code128Barcode code;
        const auto v = code.encode(input);
        if (v != output) {
            qDebug() << "Actual  :" << v;
            qDebug() << "Expected:" << output;
        }
        QCOMPARE(v, output);
    }

    void testDimension()
    {
        auto barcode = Prison::Barcode::create(Prison::Code128);
        QVERIFY(barcode);
        QCOMPARE(barcode->format(), Prison::Code128);
        QCOMPARE(barcode->dimensions(), Prison::Barcode::OneDimension);
    }

    void testSize()
    {
        auto barcode = Prison::Barcode::create(Prison::Code128);
        QVERIFY(barcode);
        QCOMPARE(barcode->format(), Prison::Code128);
        barcode->setData(QStringLiteral("UNIT TEST"));
        QCOMPARE(barcode->minimumSize(), QSize(154, 1));
        QCOMPARE(barcode->preferredSize(1), QSize(308, 50));
        QCOMPARE(barcode->preferredSize(2), QSize(154, 50));
        QCOMPARE(barcode->toImage(barcode->preferredSize(1)).size(), QSize(308, 50));
        QCOMPARE(barcode->toImage({1, 1}).isNull(), true);
    }

    void testRender_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<QString>("refName");

        QTest::newRow("text") << QByteArray("KF5::Prison") << "code128-text.png";
        QTest::newRow("binary") << QByteArray("KDE\x0\x1\x2\x3\x4\x5\x6\x7\x8\x9kde", 16) << "code128-binary.png";
    }

    void testRender()
    {
        QFETCH(QByteArray, input);
        QFETCH(QString, refName);

        {
            auto code = Prison::Barcode::create(Prison::Code128);
            QVERIFY(code);
            code->setData(QString::fromLatin1(input.constData(), input.size()));
            const auto img = code->toImage(code->minimumSize());
            img.save(refName);

            QImage ref(QStringLiteral(":/code128/") + refName);
            ref = ref.convertToFormat(img.format());
            QCOMPARE(img, ref);
        }

        {
            auto code = Prison::Barcode::create(Prison::Code128);
            QVERIFY(code);
            code->setData(QString::fromLatin1(input.constData(), input.size()));
            const auto img = code->toImage(code->minimumSize());
            img.save(refName);

            QImage ref(QStringLiteral(":/code128/") + refName);
            ref = ref.convertToFormat(img.format());
            QCOMPARE(img, ref);
        }
    }
};

QTEST_APPLESS_MAIN(Code128BarcodeTest)

#include "code128barcodetest.moc"
