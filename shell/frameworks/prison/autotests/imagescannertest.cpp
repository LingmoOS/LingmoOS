/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#include <Prison/Format>
#include <Prison/ImageScanner>
#include <Prison/ScanResult>

#include <QImage>
#include <QObject>
#include <QTest>
#include <QtConcurrent>

using namespace Qt::Literals::StringLiterals;
using namespace Prison;

class ImageScannerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testImageScan()
    {
        auto result = ImageScanner::scan(QImage());
        QVERIFY(!result.hasContent());

        QImage img(u":/qr/qr-text.png"_s);
        QVERIFY(!img.isNull());
        result = ImageScanner::scan(img);
        QVERIFY(result.hasContent());
        QVERIFY(result.hasText());
        QCOMPARE(result.text(), "KF5::Prison"_L1);

        result = ImageScanner::scan(img, Format::QRCode);
        QCOMPARE(result.text(), "KF5::Prison"_L1);

        result = ImageScanner::scan(img, Format::Aztec);
        QVERIFY(!result.hasContent());

        img = QImage(u":/qr/qr-binary.png"_s);
        QVERIFY(!img.isNull());
        result = ImageScanner::scan(img);
        QVERIFY(result.hasContent());
        QVERIFY(!result.hasText());
        QVERIFY(result.hasBinaryData());
    }

    void testImageScanMT()
    {
        QImage img(u":/qr/qr-text.png"_s);
        QEventLoop loop;
        QtConcurrent::run([&img]() {
            return ImageScanner::scan(img);
        }).then([&loop](const ScanResult &result) {
            qDebug() << result.text();
            loop.exit();
        });
        loop.exec();
    }
};

QTEST_GUILESS_MAIN(ImageScannerTest)

#include "imagescannertest.moc"
