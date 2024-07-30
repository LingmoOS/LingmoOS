/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmimeassociations_p.h"
#include "ksycoca_p.h"
#include "setupxdgdirs.h"
#include <KConfigGroup>
#include <KDesktopFile>
#include <QDebug>
#include <QDir>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QTest>
#include <kapplicationtrader.h>
#include <kbuildsycoca_p.h>
#include <kservicefactory_p.h>
#include <ksycoca.h>

using namespace Qt::StringLiterals;

// We need a factory that returns the same KService::Ptr every time it's asked for a given service.
// Otherwise the changes to the service's serviceTypes by KMimeAssociationsTest have no effect
class FakeServiceFactory : public KServiceFactory
{
public:
    FakeServiceFactory(KSycoca *db)
        : KServiceFactory(db)
    {
    }
    ~FakeServiceFactory() override;

    KService::Ptr findServiceByMenuId(const QString &name) override
    {
        // qDebug() << name;
        KService::Ptr result = m_cache.value(name);
        if (!result) {
            result = KServiceFactory::findServiceByMenuId(name);
            m_cache.insert(name, result);
        }
        // qDebug() << name << result.data();
        return result;
    }
    KService::Ptr findServiceByDesktopPath(const QString &name) override
    {
        KService::Ptr result = m_cache.value(name); // yeah, same cache, I don't care :)
        if (!result) {
            result = KServiceFactory::findServiceByDesktopPath(name);
            m_cache.insert(name, result);
        }
        return result;
    }

private:
    QMap<QString, KService::Ptr> m_cache;
};

static QString menusDir()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String{"/menus"};
}

// Helper method for all the trader tests, comes from kmimetypetest.cpp
static bool offerListHasService(const KService::List &offers, const QString &entryPath, bool expected /* if set, show error if not found */)
{
    // ksycoca resolves to canonical paths, so do it here as well
    const QString realPath = QFileInfo(entryPath).canonicalFilePath();
    Q_ASSERT(!realPath.isEmpty());

    bool found = false;
    for (const KService::Ptr &serv : offers) {
        if (serv->entryPath() == realPath) {
            if (found) { // should be there only once
                qWarning("ERROR: %s was found twice in the list", qPrintable(realPath));
                return false; // make test fail
            }
            found = true;
        }
    }
    if (!found && expected) {
        qWarning() << "ERROR:" << realPath << "not found in offer list. Here's the full list:";
        for (const KService::Ptr &serv : offers) {
            qDebug() << serv->entryPath();
        }
    }
    return found;
}

static void writeAppDesktopFile(const QString &path, const QStringList &mimeTypes)
{
    KDesktopFile file(path);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "FakeApplication");
    group.writeEntry("Type", "Application");
    group.writeEntry("Exec", "ls");
    group.writeEntry("OnlyShowIn", "KDE;UDE");
    group.writeEntry("NotShowIn", "GNOME");
    group.writeXdgListEntry("MimeType", mimeTypes);
}

static void writeNonKDEAppDesktopFile(const QString &path, const QStringList &mimeTypes) // bug 427469
{
    KDesktopFile file(path);
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "FakeApplication");
    group.writeEntry("Type", "Application");
    group.writeEntry("Exec", "ls");
    group.writeEntry("NotShowIn", "KDE");
    group.writeXdgListEntry("MimeType", mimeTypes);
}

/**
 * This unit test verifies the parsing of mimeapps.list files, both directly
 * and via kbuildsycoca (and making trader queries).
 */
class KMimeAssociationsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase()
    {
        setupXdgDirs();
        QStandardPaths::setTestModeEnabled(true);
        // The Plasma bit makes no sense, but this is just to test that this is treated as a colon-separated list
        qputenv("XDG_CURRENT_DESKTOP", "KDE:Plasma");

        m_localConfig = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/');
        QDir(m_localConfig).removeRecursively();
        QVERIFY(QDir().mkpath(m_localConfig));
        m_localApps = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1Char('/');
        QDir(m_localApps).removeRecursively();
        QVERIFY(QDir().mkpath(m_localApps));
        QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + QLatin1Char('/');
        QDir(cacheDir).removeRecursively();

        QDir(menusDir()).removeRecursively();
        QDir(menusDir()).mkpath(QStringLiteral("."));
        QFile::copy(QFINDTESTDATA("test-applications.menu"), menusDir() + QLatin1String("/applications.menu"));

        // Create fake application (associated with text/plain in mimeapps.list)
        fakeTextApplication = m_localApps + QLatin1String{"faketextapplication.desktop"};
        writeAppDesktopFile(fakeTextApplication, QStringList() << QStringLiteral("text/plain"));

        // Create fake application (associated with text/plain in mimeapps.list)
        fakeTextApplicationPrefixed = m_localApps + QLatin1String{"fakepfx/faketextapplicationpfx.desktop"};
        writeAppDesktopFile(fakeTextApplicationPrefixed, QStringList() << QStringLiteral("text/plain"));

        // A fake "default" application for text/plain (high initial preference, but not in mimeapps.list)
        fakeDefaultTextApplication = m_localApps + QLatin1String{"fakedefaulttextapplication.desktop"};
        writeAppDesktopFile(fakeDefaultTextApplication, QStringList() << QStringLiteral("text/plain"));

        // An app (like emacs) listing explicitly the derived mimetype (c-src); not in mimeapps.list
        // This interacted badly with mimeapps.list listing another app for text/plain, but the
        // lookup found this app first, due to c-src. The fix: ignoring derived mimetypes when
        // the base mimetype is already listed.
        //
        // Also include aliases (msword), to check they don't cancel each other out.
        fakeCSrcApplication = m_localApps + QLatin1String{"fakecsrcmswordapplication.desktop"};
        writeAppDesktopFile(fakeCSrcApplication,
                            QStringList() << QStringLiteral("text/plain") << QStringLiteral("text/c-src") << QStringLiteral("application/vnd.ms-word")
                                          << QStringLiteral("application/msword"));

        fakeJpegApplication = m_localApps + QLatin1String{"fakejpegapplication.desktop"};
        writeAppDesktopFile(fakeJpegApplication, QStringList() << QStringLiteral("image/jpeg"));

        fakeArkApplication = m_localApps + QLatin1String{"fakearkapplication.desktop"};
        writeAppDesktopFile(fakeArkApplication, QStringList() << QStringLiteral("application/zip"));

        fakeHtmlApplication = m_localApps + QLatin1String{"fakehtmlapplication.desktop"};
        writeAppDesktopFile(fakeHtmlApplication, QStringList() << QStringLiteral("text/html"));

        fakeHtmlApplicationPrefixed = m_localApps + QLatin1String{"fakepfx/fakehtmlapplicationpfx.desktop"};
        writeAppDesktopFile(fakeHtmlApplicationPrefixed, QStringList() << QStringLiteral("text/html"));

        fakeOktetaApplication = m_localApps + QLatin1String{"fakeoktetaapplication.desktop"};
        writeAppDesktopFile(fakeOktetaApplication, QStringList() << QStringLiteral("application/octet-stream"));

        const QString fakeGnomeRoller = m_localApps + QLatin1String{"fake.org.gnome.FileRoller.desktop"};
        writeNonKDEAppDesktopFile(fakeGnomeRoller, QStringList() << QStringLiteral("application/x-7z-compressed"));

        const QString fakeNautilus = m_localApps + QLatin1String{"fake.org.gnome.Nautilus.desktop"};
        writeNonKDEAppDesktopFile(fakeNautilus, QStringList() << QStringLiteral("application/x-7z-compressed"));

        // Update ksycoca in ~/.qttest after creating the above
        runKBuildSycoca();

        // Create factory on the heap and don't delete it. This must happen after
        // Sycoca is built, in case it did not exist before.
        // It registers to KSycoca, which deletes it at end of program execution.
        KServiceFactory *factory = new FakeServiceFactory(KSycoca::self());
        KSycocaPrivate::self()->m_serviceFactory = factory;
        QCOMPARE(KSycocaPrivate::self()->serviceFactory(), factory);

        // For debugging: print all services and their storageId
#if 0
        const KService::List lst = KService::allServices();
        QVERIFY(!lst.isEmpty());
        for (const KService::Ptr &serv : lst) {
            qDebug() << serv->entryPath() << serv->storageId() /*<< serv->desktopEntryName()*/;
        }
#endif

        KService::Ptr fakeApplicationService = KService::serviceByStorageId(QStringLiteral("faketextapplication.desktop"));
        QVERIFY(fakeApplicationService);

        m_mimeAppsFileContents =
            "[Added Associations]\n"
            "image/jpeg=fakejpegapplication.desktop;\n"
            "text/html=fakehtmlapplication.desktop;fakehtmlapplicationpfx.desktop;\n"
            "text/plain=fakepfx-faketextapplicationpfx.desktop;fakedefaulttextapplication.desktop;gvim.desktop;wine.desktop;idontexist.desktop;\n"
            // test alias resolution
            "application/x-pdf=fakejpegapplication.desktop;\n"
            // test x-scheme-handler (#358159) (missing trailing ';' as per xdg-mime bug...)
            "x-scheme-handler/mailto=faketextapplication.desktop\n"
            // test association with octet-stream (#425154)
            "application/octet-stream=fakeoktetaapplication.desktop\n"
            // test a non-kde app (#427469)
            "application/x-7z-compressed=fake.org.gnome.FileRoller.desktop;\n"
            "application/msword=fakecsrcmswordapplication.desktop\n"
            "[Added KParts/ReadOnlyPart Associations]\n"
            "text/plain=katepart.desktop;\n"
            "[Removed Associations]\n"
            "image/jpeg=firefox.desktop;\n"
            "text/html=gvim.desktop;abiword.desktop;\n"
            "[Default Applications]\n"
            "text/plain=faketextapplication.desktop;second-faketextapplicationpfx.desktop\n";
        // Expected results
        preferredApps[QStringLiteral("image/jpeg")] << QStringLiteral("fakejpegapplication.desktop");
        preferredApps[QStringLiteral("application/pdf")] << QStringLiteral("fakejpegapplication.desktop");
        preferredApps[QStringLiteral("text/plain")] << QStringLiteral("faketextapplication.desktop") << QStringLiteral("second-faketextapplicationpfx.desktop")
                                                    << QStringLiteral("fakepfx-faketextapplicationpfx.desktop") << QStringLiteral("gvim.desktop");
        preferredApps[QStringLiteral("text/x-csrc")] << QStringLiteral("faketextapplication.desktop")
                                                     << QStringLiteral("fakepfx-faketextapplicationpfx.desktop") << QStringLiteral("gvim.desktop");
        preferredApps[QStringLiteral("text/html")] << QStringLiteral("fakehtmlapplication.desktop") << QStringLiteral("fakepfx-fakehtmlapplicationpfx.desktop");
        preferredApps[QStringLiteral("application/msword")] << QStringLiteral("fakecsrcmswordapplication.desktop");
        preferredApps[QStringLiteral("x-scheme-handler/mailto")] << QStringLiteral("faketextapplication.desktop");
        preferredApps[QStringLiteral("text/x-python")] << QStringLiteral("faketextapplication.desktop");
        preferredApps[QStringLiteral("application/x-7z-compressed")] << QStringLiteral("fake.org.gnome.FileRoller.desktop");
        removedApps[QStringLiteral("application/x-7z-compressed")] << QStringLiteral("fake.org.gnome.Nautilus.desktop");
        removedApps[QStringLiteral("image/jpeg")] << QStringLiteral("firefox.desktop");
        removedApps[QStringLiteral("text/html")] << QStringLiteral("gvim.desktop") << QStringLiteral("abiword.desktop");

        // Clean-up non-existing apps
        removeNonExisting(preferredApps);
        removeNonExisting(removedApps);
    }

    void cleanupTestCase()
    {
        QFile::remove(m_localConfig + QLatin1String{"/mimeapps.list"});
        runKBuildSycoca();
    }

    void testParseSingleFile()
    {
        KOfferHash offerHash;
        KMimeAssociations parser(offerHash, KSycocaPrivate::self()->serviceFactory());

        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QFile tempFile(tempDir.path() + QLatin1String{"/mimeapps.list"});
        QVERIFY(tempFile.open(QIODevice::WriteOnly));
        tempFile.write(m_mimeAppsFileContents);
        const QString fileName = tempFile.fileName();
        tempFile.close();

        // QTest::ignoreMessage(QtDebugMsg, "findServiceByDesktopPath: idontexist.desktop not found");
        parser.parseMimeAppsList(fileName, 100);

        for (auto it = preferredApps.cbegin(), endIt = preferredApps.cend(); it != endIt; ++it) {
            const QString mime = it.key();
            // The data for derived types and aliases isn't for this test (which only looks at mimeapps.list)
            if (mime == QLatin1String("text/x-csrc") //
                || mime == QLatin1String("text/x-python") //
                || mime == QLatin1String("application/msword")) {
                continue;
            }
            const QList<KServiceOffer> offers = offerHash.offersFor(mime);
            for (const QString &service : it.value()) {
                KService::Ptr serv = KService::serviceByStorageId(service);
                if (serv && !offersContains(offers, serv)) {
                    qDebug() << "expected offer" << serv->entryPath() << "not in offers for" << mime << ":";
                    for (const KServiceOffer &offer : offers) {
                        qDebug() << offer.service()->storageId();
                    }
                    QFAIL("offer does not have servicetype");
                }
            }
        }

        for (auto it = removedApps.cbegin(), end = removedApps.cend(); it != end; ++it) {
            const QString mime = it.key();
            const QList<KServiceOffer> offers = offerHash.offersFor(mime);
            for (const QString &service : it.value()) {
                KService::Ptr serv = KService::serviceByStorageId(service);
                if (serv && offersContains(offers, serv)) {
                    // qDebug() << serv.data() << serv->entryPath() << "does not have" << mime;
                    QFAIL("offer should not have servicetype");
                }
            }
        }
    }

    void testGlobalAndLocalFiles()
    {
        KOfferHash offerHash;
        KMimeAssociations parser(offerHash, KSycocaPrivate::self()->serviceFactory());

        // Write global file
        QTemporaryDir tempDirGlobal;
        QVERIFY(tempDirGlobal.isValid());

        QFile tempFileGlobal(tempDirGlobal.path() + QLatin1String{"/mimeapps.list"});
        QVERIFY(tempFileGlobal.open(QIODevice::WriteOnly));
        QByteArray globalAppsFileContents =
            "[Added Associations]\n"
            "image/jpeg=firefox.desktop;\n" // removed by local config
            "text/html=firefox.desktop;\n" // mdv
            "image/png=fakejpegapplication.desktop;\n";
        tempFileGlobal.write(globalAppsFileContents);
        const QString globalFileName = tempFileGlobal.fileName();
        tempFileGlobal.close();

        // We didn't keep it, so we need to write the local file again
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        QFile tempFile(tempDir.path() + QLatin1String{"/mimeapps.list"});
        QVERIFY(tempFile.open(QIODevice::WriteOnly));
        tempFile.write(m_mimeAppsFileContents);
        const QString fileName = tempFile.fileName();
        tempFile.close();

        parser.parseMimeAppsList(globalFileName, 1000);
        parser.parseMimeAppsList(fileName, 1050); // += 50 is correct.

        QList<KServiceOffer> offers = offerHash.offersFor(QStringLiteral("image/jpeg"));
        std::stable_sort(offers.begin(), offers.end()); // like kbuildservicefactory.cpp does
        const QStringList expectedJpegApps = preferredApps[QStringLiteral("image/jpeg")];
        QCOMPARE(assembleOffers(offers), expectedJpegApps);

        offers = offerHash.offersFor(QStringLiteral("text/html"));
        std::stable_sort(offers.begin(), offers.end());
        QStringList textHtmlApps = preferredApps[QStringLiteral("text/html")];
        if (KService::serviceByStorageId(QStringLiteral("firefox.desktop"))) {
            textHtmlApps.append(QStringLiteral("firefox.desktop"));
        }
        qDebug() << assembleOffers(offers);
        QCOMPARE(assembleOffers(offers), textHtmlApps);

        offers = offerHash.offersFor(QStringLiteral("image/png"));
        std::stable_sort(offers.begin(), offers.end());
        QCOMPARE(assembleOffers(offers), QStringList() << QStringLiteral("fakejpegapplication.desktop"));
    }

    void testSetupRealFile()
    {
        writeToMimeApps(m_mimeAppsFileContents);

        // Test a trader query
        KService::List offers = KApplicationTrader::queryByMimeType(QStringLiteral("image/jpeg"));
        QVERIFY(!offers.isEmpty());
        QCOMPARE(offers.first()->storageId(), QStringLiteral("fakejpegapplication.desktop"));

        // Now the generic variant of the above test:
        // for each mimetype, check that the preferred apps are as specified
        for (auto it = preferredApps.cbegin(), endIt = preferredApps.cend(); it != endIt; ++it) {
            const QString mime = it.key();
            const KService::List offers = KApplicationTrader::queryByMimeType(mime);
            const QStringList offerIds = assembleServices(offers, it.value().count());
            if (offerIds != it.value()) {
                qDebug() << "offers for" << mime << ":";
                for (int i = 0; i < offers.count(); ++i) {
                    qDebug() << "   " << i << ":" << offers[i]->storageId();
                }
                qDebug() << " Expected:" << it.value();
                const QStringList expectedPreferredServices = it.value();
                for (int i = 0; i < expectedPreferredServices.count(); ++i) {
                    qDebug() << mime << i << expectedPreferredServices[i];
                    // QCOMPARE(expectedPreferredServices[i], offers[i]->storageId());
                }
            }
            QCOMPARE(offerIds, it.value());
        }
        for (auto it = removedApps.constBegin(), endIt = removedApps.constEnd(); it != endIt; ++it) {
            const QString mime = it.key();
            const KService::List offers = KApplicationTrader::queryByMimeType(mime);
            const QStringList offerIds = assembleServices(offers);
            for (const QString &service : it.value()) {
                const QString error = QStringLiteral("Offers for %1 should not contain %2").arg(mime, service);
                QVERIFY2(!offerIds.contains(service), qPrintable(error));
            }
        }
    }

    void testMultipleInheritance()
    {
        // application/x-shellscript inherits from both text/plain and application/x-executable
        KService::List offers = KApplicationTrader::queryByMimeType(QStringLiteral("application/x-shellscript"));
        QVERIFY(offerListHasService(offers, fakeTextApplication, true));
    }

    void testRemoveAssociationFromParent()
    {
        // I removed kate from text/plain, and it would still appear in text/x-java.

        // First, let's check our fake app is associated with text/plain
        KService::List offers = KApplicationTrader::queryByMimeType(QStringLiteral("text/plain"));
        QVERIFY(offerListHasService(offers, fakeTextApplication, true));

        writeToMimeApps(
            QByteArray("[Removed Associations]\n"
                       "text/plain=faketextapplication.desktop;\n"));

        offers = KApplicationTrader::queryByMimeType(QStringLiteral("text/plain"));
        QVERIFY(!offerListHasService(offers, fakeTextApplication, false));

        offers = KApplicationTrader::queryByMimeType(QStringLiteral("text/x-java"));
        QVERIFY(!offerListHasService(offers, fakeTextApplication, false));
    }

    void testRemovedImplicitAssociation() // remove (implicit) assoc from derived mimetype
    {
        // #164584: Removing ark from opendocument.text didn't work
        const QString opendocument = QStringLiteral("application/vnd.oasis.opendocument.text");

        // [sanity checking of s-m-i installation]
        QMimeType mime = QMimeDatabase().mimeTypeForName(opendocument);
        QVERIFY(mime.isValid());
        if (!mime.inherits(QStringLiteral("application/zip"))) {
            // CentOS patches out the application/zip inheritance from application/vnd.oasis.opendocument.text!! Grmbl.
            QSKIP("Broken distro where application/vnd.oasis.opendocument.text doesn't inherit from application/zip");
        }

        KService::List offers = KApplicationTrader::queryByMimeType(opendocument);
        QVERIFY(offerListHasService(offers, fakeArkApplication, true));

        writeToMimeApps(
            QByteArray("[Removed Associations]\n"
                       "application/vnd.oasis.opendocument.text=fakearkapplication.desktop;\n"));

        offers = KApplicationTrader::queryByMimeType(opendocument);
        QVERIFY(!offerListHasService(offers, fakeArkApplication, false));

        offers = KApplicationTrader::queryByMimeType(QStringLiteral("application/zip"));
        QVERIFY(offerListHasService(offers, fakeArkApplication, true));
    }

    void testRemovedImplicitAssociation178560()
    {
        // #178560: Removing ark from interface/x-winamp-skin didn't work
        // Using application/x-kns (another zip-derived mimetype) nowadays.
        const QString mime = QStringLiteral("application/x-kns");

        // That mimetype comes from kcoreaddons, let's make sure it's properly installed
        {
            QMimeDatabase db;
            QMimeType mime = db.mimeTypeForName(QStringLiteral("application/x-kns"));
            QVERIFY(mime.isValid());
            QCOMPARE(mime.name(), QStringLiteral("application/x-kns"));
            QVERIFY(mime.inherits(QStringLiteral("application/zip")));
        }

        KService::List offers = KApplicationTrader::queryByMimeType(mime);
        QVERIFY(offerListHasService(offers, fakeArkApplication, true));

        writeToMimeApps(
            QByteArray("[Removed Associations]\n"
                       "application/x-kns=fakearkapplication.desktop;\n"));

        offers = KApplicationTrader::queryByMimeType(mime);
        QVERIFY(!offerListHasService(offers, fakeArkApplication, false));

        offers = KApplicationTrader::queryByMimeType(QStringLiteral("application/zip"));
        QVERIFY(offerListHasService(offers, fakeArkApplication, true));
    }

    // remove assoc from a mime which is both a parent and a derived mimetype
    void testRemovedMiddleAssociation()
    {
        // More tricky: x-theme inherits x-desktop inherits text/plain,
        // if we remove an association for x-desktop then x-theme shouldn't
        // get it from text/plain...

        KService::List offers;
        writeToMimeApps(
            QByteArray("[Removed Associations]\n"
                       "application/x-desktop=faketextapplication.desktop;\n"));

        offers = KApplicationTrader::queryByMimeType(QStringLiteral("text/plain"));
        QVERIFY(offerListHasService(offers, fakeTextApplication, true));

        offers = KApplicationTrader::queryByMimeType(QStringLiteral("application/x-desktop"));
        QVERIFY(!offerListHasService(offers, fakeTextApplication, false));

        offers = KApplicationTrader::queryByMimeType(QStringLiteral("application/x-theme"));
        QVERIFY(!offerListHasService(offers, fakeTextApplication, false));
    }

    void testCorrectFallbackOrder()
    {
        // Verify that a "more specific type" is used before a "less-specifc"
        // application/ecmascript inherits from both application/x-executable and text/plain
        // In this example we should use both of these first which has an app set compared to the less
        // specifc application/octet-stream via application/x-executable
        auto queryOnlyDirect = [](const QString &mimeType) {
            return KApplicationTrader::queryByMimeType(mimeType, [&mimeType](const KService::Ptr &service) {
                return service->mimeTypes().contains(mimeType);
            });
        };
        const auto offers = assembleServices(KApplicationTrader::queryByMimeType(u"text/javascript"_s));
        const auto textOffers = assembleServices(queryOnlyDirect(u"text/plain"_s));
        const auto appOffers = assembleServices(queryOnlyDirect(u"application/x-executable"_s));
        auto octetOffers = assembleServices(KApplicationTrader::queryByMimeType(u"application/octet-stream"_s));
        // Apps that only support octet-stream but not text/plain nor application/x-executable
        octetOffers.removeIf([&textOffers, &appOffers](const QString &app) {
            return textOffers.contains(app) || appOffers.contains(app);
        });
        QCOMPARE(offers.mid(offers.count() - octetOffers.size(), octetOffers.size()), octetOffers);
    }

private:
    typedef QMap<QString /*mimetype*/, QStringList> ExpectedResultsMap;

    void runKBuildSycoca()
    {
        // Wait for notifyDatabaseChanged DBus signal
        // (The real KCM code simply does the refresh in a slot, asynchronously)
        QSignalSpy spy(KSycoca::self(), &KSycoca::databaseChanged);

        KBuildSycoca builder;
        QVERIFY(builder.recreate());
        if (spy.isEmpty()) {
            spy.wait();
        }
    }

    void writeToMimeApps(const QByteArray &contents)
    {
        QString mimeAppsPath = m_localConfig + QLatin1String{"/mimeapps.list"};
        QFile mimeAppsFile(mimeAppsPath);
        QVERIFY(mimeAppsFile.open(QIODevice::WriteOnly));
        mimeAppsFile.write(contents);
        mimeAppsFile.close();

        runKBuildSycoca();
    }

    static bool offersContains(const QList<KServiceOffer> &offers, KService::Ptr serv)
    {
        for (const KServiceOffer &offer : offers) {
            if (offer.service()->storageId() == serv->storageId()) {
                return true;
            }
        }
        return false;
    }
    static QStringList assembleOffers(const QList<KServiceOffer> &offers)
    {
        QStringList lst;
        for (const KServiceOffer &offer : offers) {
            lst.append(offer.service()->storageId());
        }
        return lst;
    }
    static QStringList assembleServices(const QList<KService::Ptr> &services, int maxCount = -1)
    {
        QStringList lst;
        for (const KService::Ptr &service : services) {
            lst.append(service->storageId());
            if (maxCount > -1 && lst.count() == maxCount) {
                break;
            }
        }
        return lst;
    }

    void removeNonExisting(ExpectedResultsMap &erm)
    {
        for (auto it = erm.begin(), endIt = erm.end(); it != endIt; ++it) {
            QMutableStringListIterator serv_it(it.value());
            while (serv_it.hasNext()) {
                if (!KService::serviceByStorageId(serv_it.next())) {
                    // qDebug() << "removing non-existing entry" << serv_it.value();
                    serv_it.remove();
                }
            }
        }
    }
    QString m_localApps;
    QString m_localConfig;
    QByteArray m_mimeAppsFileContents;
    QString fakeTextApplication;
    QString fakeTextApplicationPrefixed;
    QString fakeDefaultTextApplication;
    QString fakeCSrcApplication;
    QString fakeJpegApplication;
    QString fakeHtmlApplication;
    QString fakeHtmlApplicationPrefixed;
    QString fakeArkApplication;
    QString fakeOktetaApplication;

    ExpectedResultsMap preferredApps;
    ExpectedResultsMap removedApps;
};

FakeServiceFactory::~FakeServiceFactory()
{
}

QTEST_GUILESS_MAIN(KMimeAssociationsTest)

#include "kmimeassociationstest.moc"
