/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include <Prison/Barcode>

#include <QObject>
#include <QTest>

using namespace Prison;

class QrTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRenderText_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("refName");

        QTest::newRow("text") << QStringLiteral("KF5::Prison") << "qr-text.png";
    }

    void testRenderText()
    {
        QFETCH(QString, input);
        QFETCH(QString, refName);

        auto code = Prison::Barcode::create(Prison::QRCode);
        QVERIFY(code);
        code->setData(input);
        const auto img = code->toImage(code->preferredSize(1));
        img.save(refName);

        QImage ref(QStringLiteral(":/qr/") + refName);
        ref = ref.convertToFormat(img.format());
        QCOMPARE(img, ref);
    }

    void testRenderBinary_data()
    {
        QTest::addColumn<QByteArray>("input");
        QTest::addColumn<QString>("refName");

        QTest::newRow("text") << QByteArray("KF5::Prison") << "qr-text.png";
        QTest::newRow("binary") << QByteArray("KDE\x0\x1\x2\x3\x4\x5\x6\x7\x8\x9kde", 16) << "qr-binary.png";
    }

    void testRenderBinary()
    {
        QFETCH(QByteArray, input);
        QFETCH(QString, refName);

        auto code = Prison::Barcode::create(Prison::QRCode);
        QVERIFY(code);
        code->setData(input);
        const auto img = code->toImage(code->preferredSize(1));
        img.save(refName);

        QImage ref(QStringLiteral(":/qr/") + refName);
        ref = ref.convertToFormat(img.format());
        QCOMPARE(img, ref);
    }
};

QTEST_APPLESS_MAIN(QrTest)

#include "qrtest.moc"
