/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QImageReader>
#include <QStandardPaths>
#include <QTest>

#include <KIconEngine>
#include <KIconLoader>

static int setupEnvBeforeQApplication = []() {
    qunsetenv("QT_SCREEN_SCALE_FACTORS");
    qputenv("QT_SCALE_FACTOR", "2.0");
    return 1;
}();

class KIconEngine_Scaled_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);
    }

    void testCenterIcon()
    {
        QIcon icon(new KIconEngine(QStringLiteral(":/test-22x22.png"), KIconLoader::global()));
        QVERIFY(!icon.isNull());

        QWindow w;
        QCOMPARE(w.devicePixelRatio(), 2.0);
        auto image = icon.pixmap(QSize(22, 22), w.devicePixelRatio()).toImage();
        QCOMPARE(image.devicePixelRatio(), 2.0);
        QCOMPARE(image.size(), QSize(44, 44));

        QImageReader reader(QStringLiteral(":/test-22x22.png"));
        reader.setScaledSize(QSize(44, 44));
        QImage unscaled = reader.read();
        QVERIFY(!unscaled.isNull());
        QCOMPARE(unscaled.size(), QSize(44, 44));
        unscaled.setDevicePixelRatio(2.0);
        QCOMPARE(image, unscaled);

        // center vertically
        QVERIFY(icon.pixmap(QSize(22, 26), w.devicePixelRatio()).toImage().copy(0, 4, 44, 44) == image);
        // center horizontally
        QVERIFY(icon.pixmap(QSize(26, 22), w.devicePixelRatio()).toImage().copy(4, 0, 44, 44) == image);
    }
};

QTEST_MAIN(KIconEngine_Scaled_UnitTest)

#include "kiconengine_scaled_unittest.moc"
