/*
    SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QStandardPaths>
#include <QTest>

#include <KConfigGroup>
#include <KIconEngine>
#include <KIconLoader>
#include <KSharedConfig>

extern KICONTHEMES_EXPORT int kiconloader_ms_between_checks;

class KIconEngine_UnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);

        // Remove icon cache
        const QString cacheFile = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/icon-cache.kcache");
        QFile::remove(cacheFile);

        // we have Breeze via it's library and it inherits hicolor
        KConfigGroup cg(KSharedConfig::openConfig(), "Icons");
        cg.writeEntry("Theme", "breeze");
        cg.sync();

        QDir testDataDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
        testIconsDir = QDir(testDataDir.absoluteFilePath(QStringLiteral("icons")));

        // we will be recursively deleting these, so a sanity check is in order
        QVERIFY(testIconsDir.absolutePath().contains(QLatin1String("qttest")));

        testIconsDir.removeRecursively();

        // set up a minimal hicolor icon theme, in case it is not installed
        QVERIFY(testIconsDir.mkpath(QStringLiteral("hicolor/22x22/apps")));
        QVERIFY(QFile::copy(QStringLiteral(":/oxygen.theme"), testIconsDir.filePath(QStringLiteral("hicolor/index.theme"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("hicolor/22x22/apps/kde.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("hicolor/22x22/apps/org.kde.testapp.png"))));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("hicolor/22x22/actions"))); // we need the dir to exist since KIconThemes caches mDirs

        // Clear SHM cache
        KIconLoader::global()->reconfigure(QString());
    }

    void testValidIconName()
    {
        QIcon icon(new KIconEngine(QStringLiteral("kde"), KIconLoader::global()));
        QVERIFY(!icon.isNull());
        QVERIFY(!icon.name().isEmpty());
    }

    void testInvalidIconName()
    {
        QIcon icon(new KIconEngine(QStringLiteral("invalid-icon-name"), KIconLoader::global()));
        QVERIFY(icon.isNull());
        QVERIFY2(icon.name().isEmpty(), qPrintable(icon.name()));
    }

    void testFallbackIconName()
    {
        QIcon icon(new KIconEngine(QStringLiteral("kde-specific-icon"), KIconLoader::global()));
        QVERIFY(!icon.isNull());
        QCOMPARE(icon.name(), QStringLiteral("kde"));

        QIcon icon2(new KIconEngine(QStringLiteral("org.kde.testapp-specific-icon"), KIconLoader::global()));
        QVERIFY(!icon2.isNull());
        QCOMPARE(icon2.name(), QStringLiteral("org.kde.testapp"));
    }

    void testUnknownIconNotCached() // QIcon version of the test in kiconloader_unittest.cpp
    {
        // This is a test to ensure that "unknown" icons are cached as unknown
        // for performance reasons, but after a while they'll be looked up again
        // so that newly installed icons can be used without a reboot.

        kiconloader_ms_between_checks = 500000;

        QString actionIconsSubdir = QStringLiteral("hicolor/22x22/actions");
        QVERIFY(testIconsDir.mkpath(actionIconsSubdir));
        QString actionIconsDir = testIconsDir.filePath(actionIconsSubdir);

        QString nonExistingIconName = QStringLiteral("asvdfg_fhqwhgds");
        QString newIconPath = actionIconsDir + QLatin1String("/") + nonExistingIconName + QLatin1String(".png");
        QFile::remove(newIconPath);

        // Find a non-existent icon
        QIcon icon(new KIconEngine(nonExistingIconName, KIconLoader::global()));
        QVERIFY(icon.isNull());
        QVERIFY(icon.name().isEmpty());

        // Install the existing icon by copying.
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), newIconPath));

        // Attempt to find the icon again, the cache will still be used for now.
        QIcon icon2(new KIconEngine(nonExistingIconName, KIconLoader::global()));
        QVERIFY(icon2.isNull());
        QVERIFY(icon2.name().isEmpty());

        // Force a recheck to happen on next lookup
        kiconloader_ms_between_checks = 0;

        // Verify the icon can now be found.
        QIcon nowExistingIcon(new KIconEngine(nonExistingIconName, KIconLoader::global()));
        QVERIFY(!nowExistingIcon.isNull());
        QCOMPARE(nowExistingIcon.name(), nonExistingIconName);

        // And verify again, this time with the cache
        kiconloader_ms_between_checks = 50000;
        QIcon icon3(new KIconEngine(nonExistingIconName, KIconLoader::global()));
        QVERIFY(!icon3.isNull());
        QCOMPARE(icon3.name(), nonExistingIconName);
    }

    void testCenterIcon()
    {
        QIcon icon(new KIconEngine(QStringLiteral("kde"), KIconLoader::global()));
        QVERIFY(!icon.isNull());

        // "kde" icon is actually "test-22x22.png", so this is original icon image
        const QImage image = icon.pixmap(22, 22).toImage();

        // center vertically
        QVERIFY(icon.pixmap(22, 26).toImage().copy(0, 2, 22, 22) == image);

        // center horizontally
        QVERIFY(icon.pixmap(26, 22).toImage().copy(2, 0, 22, 22) == image);
    }

    void testNonSquareSvg()
    {
        QIcon icon(new KIconEngine(QStringLiteral(":/nonsquare.svg"), KIconLoader::global()));
        QVERIFY(!icon.isNull());

        // verify we get the content fully fitted in when asking for the right aspect ratio
        const QImage image = icon.pixmap(40, 20).toImage();
        QCOMPARE(image.pixelColor(0, 0), QColor(255, 0, 0));
        QCOMPARE(image.pixelColor(19, 9), QColor(255, 0, 0));
        QCOMPARE(image.pixelColor(39, 0), QColor(0, 255, 0));
        QCOMPARE(image.pixelColor(20, 9), QColor(0, 255, 0));
        QCOMPARE(image.pixelColor(0, 19), QColor(0, 0, 255));
        QCOMPARE(image.pixelColor(19, 10), QColor(0, 0, 255));
        QCOMPARE(image.pixelColor(39, 19), QColor(255, 255, 0));
        QCOMPARE(image.pixelColor(20, 10), QColor(255, 255, 0));

        // and now with a wrong aspect ratio
        QCOMPARE(icon.pixmap(40, 40).toImage().convertToFormat(image.format()).copy(0, 10, 40, 20), image);
    }

    void testMode_data()
    {
        QTest::addColumn<QIcon::Mode>("mode");
        QTest::addColumn<QColor>("expectedColor");

        QTest::addRow("normal") << QIcon::Normal << QColor(41, 187, 253, 255);
        QTest::addRow("disabled") << QIcon::Disabled << QColor(147, 147, 147, 127);
        QTest::addRow("active") << QIcon::Active << QColor(97, 216, 253, 255);
        QTest::addRow("selected") << QIcon::Selected << QColor(41, 187, 253, 255);
    }

    void testMode()
    {
        QFETCH(QIcon::Mode, mode);
        QFETCH(QColor, expectedColor);

        QIcon icon(new KIconEngine(QStringLiteral("kde"), KIconLoader::global()));
        QVERIFY(!icon.isNull());

        const QImage image = icon.pixmap(32, 32, mode).toImage();

        QCOMPARE(image.pixelColor(5, 5).name(), expectedColor.name());
    }

private:
    QDir testIconsDir;
};

QTEST_MAIN(KIconEngine_UnitTest)

#include "kiconengine_unittest.moc"
