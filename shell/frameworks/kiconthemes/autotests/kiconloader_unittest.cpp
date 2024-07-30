/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <kiconloader.h>

#include <QDir>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTest>

#include <KConfigGroup>
#include <KIconTheme>
#include <KSharedConfig>

extern KICONTHEMES_EXPORT void uintToHex(uint32_t colorData, QChar *buffer);

class KIconLoader_UnitTest : public QObject
{
    Q_OBJECT
public:
    KIconLoader_UnitTest()
        : testSizes({12, 22, 32, 42, 82, 132, 243})
    {
    }

private:
    QDir testDataDir;
    QDir testIconsDir;
    QString appName;
    QDir appDataDir;
    const QList<int> testSizes;

private Q_SLOTS:
    void initTestCase()
    {
        QStandardPaths::setTestModeEnabled(true);

        // ensure we don't use the breeze icon set from our lib for these tests but the fake we set up below
        KIconTheme::forceThemeForTests(QStringLiteral("fakebreeze"));

        // we use an own fakebreeze that fallbacks to fakeoxygen
        KConfigGroup cg(KSharedConfig::openConfig(), "Icons");
        cg.writeEntry("Theme", "fakebreeze");
        cg.sync();

        testDataDir = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
        testIconsDir = QDir(testDataDir.absoluteFilePath(QStringLiteral("icons")));

        appName = QStringLiteral("kiconloader_unittest");
        appDataDir = QDir(testDataDir.absoluteFilePath(appName));

        // we will be recursively deleting these, so a sanity check is in order
        QVERIFY(testIconsDir.absolutePath().contains(QLatin1String("qttest")));
        QVERIFY(appDataDir.absolutePath().contains(QLatin1String("qttest")));

        testIconsDir.removeRecursively();
        appDataDir.removeRecursively();

        QVERIFY(appDataDir.mkpath(QStringLiteral("pics")));
        QVERIFY(QFile::copy(QStringLiteral(":/app-image.png"), appDataDir.filePath(QStringLiteral("pics/image1.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/app-image.png"), appDataDir.filePath(QStringLiteral("pics/image2.png"))));

        // setup fake oxygen as fallback
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakeoxygen/22x22/actions")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakeoxygen/22x22/animations")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakeoxygen/22x22/apps")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakeoxygen/22x22/mimetypes")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakeoxygen/32x32/apps")));
        QVERIFY(QFile::copy(QStringLiteral(":/oxygen.theme"), testIconsDir.filePath(QStringLiteral("fakeoxygen/index.theme"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/apps/kde.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/anim-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/animations/process-working.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/mimetypes/text-plain.png"))));
        QVERIFY(
            QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/mimetypes/application-octet-stream.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/mimetypes/image-x-generic.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/mimetypes/video-x-generic.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/mimetypes/x-office-document.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/mimetypes/audio-x-generic.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/mimetypes/unknown.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-32x32.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/32x32/apps/kde.png"))));

        // set up a minimal fake Breeze icon theme, fallback to oxygen
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakebreeze/22x22/actions")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakebreeze/22x22/animations")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakebreeze/22x22/apps")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakebreeze/22x22/mimetypes")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakebreeze/22x22/appsNoContext")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakebreeze/22x22/appsNoType")));
        QVERIFY(testIconsDir.mkpath(QStringLiteral("fakebreeze/22x22/appsNoContextOrType")));

        const QString breezeThemeFile = testIconsDir.filePath(QStringLiteral("fakebreeze/index.theme"));
        QVERIFY(QFile::copy(QStringLiteral(":/breeze.theme"), breezeThemeFile));
        // kde.png is missing, it should fallback to oxygen
        // QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/apps/kde.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"),
                            testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/appsNoContext/iconindirectorywithoutcontext.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"),
                            testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/appsNoType/iconindirectorywithouttype.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"),
                            testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/appsNoContextOrType/iconindirectorywithoutcontextortype.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/anim-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/animations/process-working.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/mimetypes/text-plain.png"))));
        QVERIFY(
            QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/mimetypes/application-octet-stream.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/mimetypes/image-x-generic.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/mimetypes/video-x-generic.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/mimetypes/x-office-document.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/mimetypes/audio-x-generic.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/mimetypes/unknown.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/coloredsvgicon.svg"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/apps/coloredsvgicon.svg"))));

        // prepare some icons for our actions test
        // when querying breeze for 'one-two', we expect
        // 'one' from breeze instead of oxygen's 'one-two'.
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakeoxygen/22x22/actions/one-two.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/actions/one.png"))));

        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/actions/one-symbolic.png"))));
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), testIconsDir.filePath(QStringLiteral("fakebreeze/22x22/actions/three.png"))));

        QVERIFY(QFile::setPermissions(breezeThemeFile, QFileDevice::ReadOwner | QFileDevice::WriteOwner));
        KConfig configFile(breezeThemeFile);
        KConfigGroup iconThemeGroup = configFile.group("Icon Theme");
        QVERIFY(iconThemeGroup.isValid());
        QStringList dirs = iconThemeGroup.readEntry("Directories", QStringList());
        for (int i : testSizes) {
            const QString relDir = QStringLiteral("%1x%1/emblems").arg(i);
            const QString dir = testIconsDir.filePath(QStringLiteral("fakebreeze/") + relDir);
            QVERIFY(QDir().mkpath(dir));

            QPixmap img(i, i);
            img.fill(Qt::red);
            QVERIFY(img.save(dir + QStringLiteral("/red.png")));

            dirs += relDir;
            KConfigGroup dirGroup = configFile.group(relDir);
            dirGroup.writeEntry("Size", i);
            dirGroup.writeEntry("Context", "Emblems");
            dirGroup.writeEntry("Type", "Fixed");
        }
        iconThemeGroup.writeEntry("Directories", dirs);
        QVERIFY(configFile.sync());
    }

    void cleanupTestCase()
    {
        if (testIconsDir != QDir()) {
            testIconsDir.removeRecursively();
        }
        if (appDataDir != QDir()) {
            appDataDir.removeRecursively();
        }
    }

    void init()
    {
        // Remove icon cache
        const QString cacheFile = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QStringLiteral("/icon-cache.kcache");
        QFile::remove(cacheFile);

        // Clear SHM cache
        KIconLoader::global()->reconfigure(QString());
    }

    void testUnknownIconNotCached()
    {
        // This is a test to ensure that "unknown" icons do not pin themselves
        // in the icon loader. Or in other words, if an "unknown" icon is
        // returned, but the appropriate icon is subsequently installed
        // properly, the next request for that icon should return the new icon
        // instead of the unknown icon.

        QString actionIconsSubdir = QStringLiteral("fakeoxygen/22x22/actions");
        QVERIFY(testIconsDir.mkpath(actionIconsSubdir));
        QString actionIconsDir = testIconsDir.filePath(actionIconsSubdir);

        QString nonExistingIconName = QStringLiteral("fhqwhgads_homsar");
        QString newIconPath = actionIconsDir + QLatin1String("/") + nonExistingIconName + QLatin1String(".png");
        QFile::remove(newIconPath);

        KIconLoader iconLoader;

        // Find a non-existent icon, allowing unknown icon to be returned
        QPixmap nonExistingIcon = iconLoader.loadIcon(nonExistingIconName, KIconLoader::Toolbar);
        QCOMPARE(nonExistingIcon.isNull(), false);

        // Install the existing icon by copying.
        QVERIFY(QFile::copy(QStringLiteral(":/test-22x22.png"), newIconPath));

        // Verify the icon can now be found.
        QPixmap nowExistingIcon = iconLoader.loadIcon(nonExistingIconName, KIconLoader::Toolbar);
        QVERIFY(nowExistingIcon.cacheKey() != nonExistingIcon.cacheKey());
        QCOMPARE(iconLoader.iconPath(nonExistingIconName, KIconLoader::Toolbar), newIconPath);
    }

    void testLoadIconCanReturnNull()
    {
        // This is a test for the "canReturnNull" argument of KIconLoader::loadIcon().
        // We try to load an icon that doesn't exist, first with canReturnNull=false (the default)
        // then with canReturnNull=true.
        KIconLoader iconLoader;
        // We expect a warning here... This doesn't work though, due to the extended debug
        // QTest::ignoreMessage(QtWarningMsg, "KIconLoader::loadIcon: No such icon \"this-icon-does-not-exist\"");
        QPixmap pix = iconLoader.loadIcon(QStringLiteral("this-icon-does-not-exist"), KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));
        // Try it again, to see if the cache interfers
        pix = iconLoader.loadIcon(QStringLiteral("this-icon-does-not-exist"), KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));
        // And now set canReturnNull to true
        pix =
            iconLoader.loadIcon(QStringLiteral("this-icon-does-not-exist"), KIconLoader::Desktop, 16, KIconLoader::DefaultState, QStringList(), nullptr, true);
        QVERIFY(pix.isNull());
        // Try getting the "unknown" icon again, to see if the above call didn't put a null icon into the cache...
        pix = iconLoader.loadIcon(QStringLiteral("this-icon-does-not-exist"), KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));

        // Another one, to clear "last" cache
        pix = iconLoader.loadIcon(QStringLiteral("this-icon-does-not-exist-either"), KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));

        // Now load the initial one again - do we get the warning again?
        pix = iconLoader.loadIcon(QStringLiteral("this-icon-does-not-exist"), KIconLoader::Desktop, 16);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(16, 16));

        pix = iconLoader.loadIcon(QStringLiteral("#crazyname"), KIconLoader::NoGroup, 1600);
        QVERIFY(!pix.isNull());
        QCOMPARE(pix.size(), QSize(1600, 1600));
    }

    void testAppPicsDir()
    {
        KIconLoader appIconLoader(appName);
        QString iconPath = appIconLoader.iconPath(QStringLiteral("image1"), KIconLoader::User);
        QCOMPARE(iconPath, appDataDir.filePath(QStringLiteral("pics/image1.png")));
        QVERIFY(QFile::exists(iconPath));

        // Load it again, to use the "last loaded" cache
        QString iconPath2 = appIconLoader.iconPath(QStringLiteral("image1"), KIconLoader::User);
        QCOMPARE(iconPath, iconPath2);
        // Load something else, to clear the "last loaded" cache
        QString iconPathTextEdit = appIconLoader.iconPath(QStringLiteral("image2"), KIconLoader::User);
        QCOMPARE(iconPathTextEdit, appDataDir.filePath(QStringLiteral("pics/image2.png")));
        QVERIFY(QFile::exists(iconPathTextEdit));
        // Now load kdialog again, to use the real kiconcache
        iconPath2 = appIconLoader.iconPath(QStringLiteral("image1"), KIconLoader::User);
        QCOMPARE(iconPath, iconPath2);
    }

    void testAppPicsDir_KDE_icon()
    {
        // #### This test is broken; it passes even if appName is set to foobar, because
        // QIcon::pixmap returns an unknown icon if it can't find the real icon...
        KIconLoader appIconLoader(appName);
        // Now using KDE::icon. Separate test so that KIconLoader isn't fully inited.
        QIcon icon = KDE::icon(QStringLiteral("image1"), &appIconLoader);
        {
            QPixmap pix = icon.pixmap(QSize(22, 22));
            QVERIFY(!pix.isNull());
        }
        // Qt's implementation of actualSize does not crop to squares, ensure we don't do that either
        QCOMPARE(icon.actualSize(QSize(96, 22)), QSize(96, 22));
        QCOMPARE(icon.actualSize(QSize(22, 96)), QSize(22, 96));
        QCOMPARE(icon.actualSize(QSize(22, 16)), QSize(22, 16));

        // Can we ask for a really small size?
        {
            QPixmap pix8 = icon.pixmap(QSize(8, 8));
            QCOMPARE(pix8.size(), QSize(8, 8));
        }
    }

    void testLoadMimeTypeIcon_data()
    {
        QTest::addColumn<QString>("iconName");
        QTest::addColumn<QString>("expectedFileName");

        QTest::newRow("existing icon") << QStringLiteral("text-plain") << QStringLiteral("text-plain.png");
        QTest::newRow("octet-stream icon") << QStringLiteral("application-octet-stream") << QStringLiteral("application-octet-stream.png");
        QTest::newRow("non-existing icon") << QStringLiteral("foo-bar") << QStringLiteral("application-octet-stream.png");
        // Test this again, because now we won't go into the "fast path" of loadMimeTypeIcon anymore.
        QTest::newRow("existing icon again") << QStringLiteral("text-plain") << QStringLiteral("text-plain.png");
        QTest::newRow("generic fallback") << QStringLiteral("image-foo-bar") << QStringLiteral("image-x-generic.png");
        QTest::newRow("video generic fallback") << QStringLiteral("video-foo-bar") << QStringLiteral("video-x-generic.png");
        QTest::newRow("image-x-generic itself") << QStringLiteral("image-x-generic") << QStringLiteral("image-x-generic.png");
        QTest::newRow("x-office-document icon") << QStringLiteral("x-office-document") << QStringLiteral("x-office-document.png");
        QTest::newRow("unavailable generic icon") << QStringLiteral("application/x-font-vfont") << QStringLiteral("application-octet-stream.png");
        QTest::newRow("#184852") << QStringLiteral("audio/x-tuxguitar") << QStringLiteral("audio-x-generic.png");
        QTest::newRow("#178847") << QStringLiteral("image/x-compressed-xcf") << QStringLiteral("image-x-generic.png");
        QTest::newRow("mimetype generic icon") << QStringLiteral("application-x-fluid") << QStringLiteral("x-office-document.png");
    }

    void testLoadMimeTypeIcon()
    {
        QFETCH(QString, iconName);
        QFETCH(QString, expectedFileName);
        KIconLoader iconLoader;
        QString path;
        QPixmap pix = iconLoader.loadMimeTypeIcon(iconName, KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(!pix.isNull());
        QCOMPARE(path.section(QLatin1Char('/'), -1), expectedFileName);

        // do the same test using a global iconloader, so that
        // we get into the final return statement, which can only happen
        // if d->extraDesktopIconsLoaded becomes true first....
        QString path2;
        pix = KIconLoader::global()->loadMimeTypeIcon(iconName, KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path2);
        QVERIFY(!pix.isNull());
        QCOMPARE(path2, path);
    }

    void testHasIcon()
    {
        // Do everything twice to check code paths that might use a cache
        QVERIFY(KIconLoader::global()->hasIcon(QStringLiteral("kde")));
        QVERIFY(KIconLoader::global()->hasIcon(QStringLiteral("kde")));
        KIconLoader iconLoader;
        QVERIFY(iconLoader.hasIcon(QStringLiteral("kde")));

        QVERIFY(KIconLoader::global()->hasIcon(QStringLiteral("process-working")));
        QVERIFY(KIconLoader::global()->hasIcon(QStringLiteral("process-working")));
        QVERIFY(!KIconLoader::global()->hasIcon(QStringLiteral("no-such-icon-exists")));
        QVERIFY(!KIconLoader::global()->hasIcon(QStringLiteral("no-such-icon-exists")));
    }

    void testIconPath()
    {
        // Test iconPath with non-existing icon
        const QString path = KIconLoader::global()->iconPath(QStringLiteral("nope-no-such-icon"), KIconLoader::Desktop, true /*canReturnNull*/);
        QVERIFY2(path.isEmpty(), qPrintable(path));

        const QString unknownPath = KIconLoader::global()->iconPath(QStringLiteral("nope-no-such-icon"), KIconLoader::Desktop, false);
        QVERIFY(!unknownPath.isEmpty());
        QVERIFY(QFile::exists(unknownPath));
    }

    void testCorrectFallback()
    {
        // we want to prefer icons from the same theme

        // so if we have something like:
        /*
            oxygen:
                one-two

            breeze:
                one
        */
        // and we ask for 'one-two', we expect to see 'one' from breeze instead
        // of 'one-two' from oxygen.
        QString path;
        KIconLoader::global()->loadIcon(QStringLiteral("one-two"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(path.contains("fakebreeze/22x22/actions"));
    }

    void testPathStore()
    {
        QString path;
        QPixmap pix = KIconLoader::global()->loadIcon(QStringLiteral("kde"), KIconLoader::Desktop, 0, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(!path.isEmpty());
        QVERIFY(QFile::exists(path));
        QVERIFY2(path.contains(QLatin1String("32x32")), qPrintable(path));
        QCOMPARE(pix.size(), QSize(32, 32));

        // Compare with iconPath()
        QString path2 = KIconLoader::global()->iconPath(QStringLiteral("kde"), KIconLoader::Desktop);
        QCOMPARE(path2, path);

        // Now specify a size
        pix = KIconLoader::global()->loadIcon(QStringLiteral("kde"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(!path.isEmpty());
        QVERIFY(QFile::exists(path));
        QVERIFY2(path.contains(QLatin1String("22x22")), qPrintable(path));
        QCOMPARE(pix.size(), QSize(24, 24));

        QVERIFY(KIconLoader::global()->hasIcon(QStringLiteral("kde")));

        path = QString();
        KIconLoader::global()
            ->loadIcon(QStringLiteral("does_not_exist"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path, true /* canReturnNull */);
        QVERIFY2(path.isEmpty(), qPrintable(path));

        path = QStringLiteral("some filler to check loadIcon() clears the variable");
        KIconLoader::global()
            ->loadIcon(QStringLiteral("does_not_exist"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path, true /* canReturnNull */);
        QVERIFY2(path.isEmpty(), qPrintable(path));

        // Test that addAppDir doesn't break loading of icons from the old known paths
        KIconLoader loader;
        // only addAppDir
        loader.addAppDir(QStringLiteral("kiconloader_unittest"));
        path = QString();
        loader.loadIcon(QStringLiteral("kde"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(!path.isEmpty());

        path = QString();
        loader.loadIcon(QStringLiteral("image1"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(!path.isEmpty());

        // only reconfigure
        KIconLoader loader2;
        path = QString();
        loader2.reconfigure(QStringLiteral("kiconloader_unittest"));
        loader2.loadIcon(QStringLiteral("kde"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(!path.isEmpty());
        loader2.loadIcon(QStringLiteral("image1"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(!path.isEmpty());

        // both addAppDir and reconfigure
        KIconLoader loader3;
        path = QString();
        loader3.addAppDir(QStringLiteral("kiconloader_unittest"));
        loader3.reconfigure(QStringLiteral("kiconloader_unittest"));
        loader3.loadIcon(QStringLiteral("kde"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(!path.isEmpty());

        path = QString();
        loader3.loadIcon(QStringLiteral("image1"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(!path.isEmpty());
    }

    void testPathsNoContextType()
    {
        {
            QString path;
            KIconLoader::global()
                ->loadIcon(QStringLiteral("iconindirectorywithoutcontext"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
            QVERIFY(path.endsWith(QLatin1String("appsNoContext/iconindirectorywithoutcontext.png")));
        }
        {
            QString path;
            KIconLoader::global()
                ->loadIcon(QStringLiteral("iconindirectorywithouttype"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
            QVERIFY(path.endsWith(QLatin1String("appsNoType/iconindirectorywithouttype.png")));
        }
        {
            QString path;
            KIconLoader::global()
                ->loadIcon(QStringLiteral("iconindirectorywithoutcontextortype"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
            QVERIFY(path.endsWith(QLatin1String("appsNoContextOrType/iconindirectorywithoutcontextortype.png")));
        }
    }

    void testLoadIconNoGroupOrSize() // #246016
    {
        QPixmap pix = KIconLoader::global()->loadIcon(QStringLiteral("connected"), KIconLoader::NoGroup);
        QVERIFY(!pix.isNull());
    }

    void testUnknownIcon()
    {
        QPixmap pix = KIconLoader::unknown();
        QPixmap pix2 = KIconLoader::unknown();
        QCOMPARE(pix.cacheKey(), pix2.cacheKey());
    }

    void testAppropriateSizes()
    {
        const KIconLoader iconLoader;
        const QRegularExpression rx(QStringLiteral("/(\\d+)x\\d+/"));
        for (int i = 1; i < testSizes.last() * 1.2; i += 3) {
            QString path;
            QPixmap pix = iconLoader.loadIcon(QStringLiteral("red"), KIconLoader::Desktop, i, KIconLoader::DefaultState, QStringList(), &path);
            QVERIFY(!path.isEmpty());
            QVERIFY(!pix.isNull());

            const QRegularExpressionMatch match = rx.match(path);
            QVERIFY(match.isValid());

            const int foundSize = match.captured(1).toInt();
            int ts = testSizes.last();
            for (int w = 0; w < testSizes.size(); w++) {
                const int curr = testSizes[w];
                if (curr >= i) {
                    ts = curr;
                    break;
                }
            }
            QVERIFY(ts == foundSize);
        }
    }

    void testColoredSvgIcon()
    {
        QPalette pal = qApp->palette();
        pal.setColor(QPalette::WindowText, QColor(255, 0, 0));
        qApp->setPalette(pal);
        QImage img = KIconLoader::global()->loadIcon(QStringLiteral("coloredsvgicon"), KIconLoader::NoGroup).toImage();
        QVERIFY(!img.isNull());
        // Has the image been recolored to red,
        // that is the color we wrote in kdeglobals as text color?
        QCOMPARE(img.pixel(0, 0), (uint)4294901760);
    }

    void testUintToHex()
    {
        // HEX (ARGB format without the #): ff6496c8
        QColor testColorNoAlpha(100, 150, 200);

        // The ARGB string in which the composed hex value is stored.
        QString argbHex(8, Qt::Uninitialized);

        // Verify the ARGB hex (ff6496c8)
        uintToHex(testColorNoAlpha.rgba(), argbHex.data());
        QCOMPARE(argbHex, QStringLiteral("ff6496c8"));

        // HEX (ARGB format without the #): 7b6496c8
        QColor testColorWithAlpha(100, 150, 200, 123);

        // Test uintToHex to verify its ARGB output.
        uintToHex(testColorWithAlpha.rgba(), argbHex.data());
        QCOMPARE(argbHex, QStringLiteral("7b6496c8"));
    }

    void testQDirSetSearchPaths()
    {
        // setup search path for testprefix: => we shall find the iconinspecialsearchpath.svg afterwards, see e.g. bug 434451
        QDir::setSearchPaths("testprefix", QStringList(":/searchpathdefineddir"));
        QPixmap pix = KIconLoader::global()->loadIcon(QStringLiteral("testprefix:iconinspecialsearchpath.svg"),
                                                      KIconLoader::NoGroup,
                                                      24,
                                                      KIconLoader::DefaultState,
                                                      QStringList(),
                                                      nullptr,
                                                      true);
        QVERIFY(!pix.isNull());
    }

    void testCorrectSymbolicFallback()
    {
        // Try to find "one-two-symbolic", but should fall back to "one-symbolic" which exists
        QString path;
        KIconLoader::global()->loadIcon(QStringLiteral("one-two-symbolic"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(path.contains("fakebreeze/22x22/actions"));
        QVERIFY(path.contains("-symbolic"));

        // Try to find the non-existent "three-four-symbolic" but only "three" exists. It should fall back to the non-symbolic
        KIconLoader::global()->loadIcon(QStringLiteral("three-four-symbolic"), KIconLoader::Desktop, 24, KIconLoader::DefaultState, QStringList(), &path);
        QVERIFY(path.contains("fakebreeze/22x22/actions"));
        QVERIFY(!path.contains("-symbolic"));
    }
};

QTEST_MAIN(KIconLoader_UnitTest)

#include "kiconloader_unittest.moc"
