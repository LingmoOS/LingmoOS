/*
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QImage>
#include <QImageReader>
#include <QTest>

static bool imgEquals(const QImage &im1, const QImage &im2)
{
    const int height = im1.height();
    const int width = im1.width();
    for (int i = 0; i < height; ++i) {
        const auto *line1 = reinterpret_cast<const quint8 *>(im1.scanLine(i));
        const auto *line2 = reinterpret_cast<const quint8 *>(im2.scanLine(i));
        for (int j = 0; j < width; ++j) {
            if (line1[j] - line2[j] != 0) {
                return false;
            }
        }
    }
    return true;
}

class AniTests : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QCoreApplication::addLibraryPath(QStringLiteral(PLUGIN_DIR));
    }

    void testReadMetadata()
    {
        QImageReader reader(QFINDTESTDATA("ani/test.ani"));

        QVERIFY(reader.canRead());

        QCOMPARE(reader.imageCount(), 4);

        QCOMPARE(reader.size(), QSize(32, 32));

        QCOMPARE(reader.text(QStringLiteral("Title")), QStringLiteral("ANI Test"));
        QCOMPARE(reader.text(QStringLiteral("Author")), QStringLiteral("KDE Community"));
    }

    void textRead()
    {
        QImageReader reader(QFINDTESTDATA("ani/test.ani"));
        QVERIFY(reader.canRead());
        QCOMPARE(reader.currentImageNumber(), 0);

        QImage aniFrame;
        QVERIFY(reader.read(&aniFrame));

        QImage img1(QFINDTESTDATA("ani/test_1.png"));
        img1.convertTo(aniFrame.format());

        QVERIFY(imgEquals(aniFrame, img1));

        QCOMPARE(reader.nextImageDelay(), 166); // 10 "jiffies"

        QVERIFY(reader.canRead());
        // that read() above should have advanced us to the next frame
        QCOMPARE(reader.currentImageNumber(), 1);

        QVERIFY(reader.read(&aniFrame));
        QImage img2(QFINDTESTDATA("ani/test_2.png"));
        img2.convertTo(aniFrame.format());

        QVERIFY(imgEquals(aniFrame, img2));

        // The "middle" frame has a longer delay than the others
        QCOMPARE(reader.nextImageDelay(), 333); // 20 "jiffies"

        QVERIFY(reader.canRead());
        QCOMPARE(reader.currentImageNumber(), 2);

        QVERIFY(reader.read(&aniFrame));
        QImage img3(QFINDTESTDATA("ani/test_3.png"));
        img3.convertTo(aniFrame.format());

        QVERIFY(imgEquals(aniFrame, img3));

        QCOMPARE(reader.nextImageDelay(), 166);

        QVERIFY(reader.canRead());
        QCOMPARE(reader.currentImageNumber(), 3);

        QVERIFY(reader.read(&aniFrame));
        // custom sequence in the ANI file should get us back to img2
        QVERIFY(imgEquals(aniFrame, img2));

        QCOMPARE(reader.nextImageDelay(), 166);

        // We should have reached the end now
        QVERIFY(!reader.canRead());
        QVERIFY(!reader.read(&aniFrame));

        // Jump back to the start
        QVERIFY(reader.jumpToImage(0));

        QVERIFY(reader.canRead());
        QCOMPARE(reader.currentImageNumber(), 0);

        QCOMPARE(reader.nextImageDelay(), 166);

        QVERIFY(reader.read(&aniFrame));
        QVERIFY(imgEquals(aniFrame, img1));
    }
};

QTEST_MAIN(AniTests)

#include "anitest.moc"
