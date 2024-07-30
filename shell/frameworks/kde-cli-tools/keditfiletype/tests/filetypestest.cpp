/* This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 David Faure <faure@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include <KService>

#include <KConfigGroup>
#include <KDesktopFile>
#include <KSycoca>

// Qt
#include <QDir>
#include <QLoggingCategory>
#include <QMimeDatabase>
#include <QProcess>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTest>

#include <mimetypedata.h>
#include <mimetypewriter.h>

// Unfortunately this isn't available in non-developer builds of Qt...
// extern Q_CORE_EXPORT int qmime_secondsBetweenChecks; // see qmimeprovider.cpp

class FileTypesTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase()
    {
        QLoggingCategory::setFilterRules(QStringLiteral("kf.coreaddons.kdirwatch.debug=true"));

        QStandardPaths::setTestModeEnabled(true);

        // update-mime-database needs to know about that test directory for the mimetype pattern change in testMimeTypePatterns to have an effect
        qputenv("XDG_DATA_HOME", QFile::encodeName(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)));

        m_mimeTypeCreatedSuccessfully = false;
        QStringList appsDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
        // qDebug() << appsDirs;
        m_localApps = appsDirs.first() + QLatin1Char('/');
        m_localConfig = QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation));
        QVERIFY(QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/mime/packages")));

        QFile::remove(m_localConfig.filePath(QStringLiteral("mimeapps.list")));
        QFile::remove(m_localConfig.filePath(QStringLiteral("kpartsrc")));
        QFile::remove(m_localConfig.filePath(QStringLiteral("filetypesrc")));

        // Create fake applications for some tests below.
        fakeApplication = QStringLiteral("fakeapplication.desktop");
        createDesktopFile(m_localApps + fakeApplication, {QStringLiteral("image/png")});
        fakeApplication2 = QStringLiteral("fakeapplication2.desktop");
        createDesktopFile(m_localApps + fakeApplication2, {QStringLiteral("image/png"), QStringLiteral("text/plain")});

        // Cleanup after testMimeTypePatterns if it failed mid-way
        const QString packageFileName =
            QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/") + QStringLiteral("packages/text-plain.xml");
        if (!packageFileName.isEmpty()) {
            QFile::remove(packageFileName);
            MimeTypeWriter::runUpdateMimeDatabase();
        }

        KService::Ptr fakeApplicationService = KService::serviceByStorageId(fakeApplication);
        QVERIFY(fakeApplicationService);
    }

    void testMimeTypeGroupAutoEmbed()
    {
        MimeTypeData data(QStringLiteral("text"));
        QCOMPARE(data.majorType(), QStringLiteral("text"));
        QCOMPARE(data.name(), QStringLiteral("text"));
        QVERIFY(data.isMeta());
        QCOMPARE(data.autoEmbed(), MimeTypeData::No); // text doesn't autoembed by default
        QVERIFY(!data.isDirty());
        data.setAutoEmbed(MimeTypeData::Yes);
        QCOMPARE(data.autoEmbed(), MimeTypeData::Yes);
        QVERIFY(data.isDirty());
        QVERIFY(!data.sync()); // save to disk. Should succeed, but return false (no need to run update-mime-database)
        QVERIFY(!data.isDirty());
        // Check what's on disk by creating another MimeTypeData instance
        MimeTypeData data2(QStringLiteral("text"));
        QCOMPARE(data2.autoEmbed(), MimeTypeData::Yes);
        QVERIFY(!data2.isDirty());
        data2.setAutoEmbed(MimeTypeData::No); // revert to default, for next time
        QVERIFY(data2.isDirty());
        QVERIFY(!data2.sync());
        QVERIFY(!data2.isDirty());

        // TODO test askSave after cleaning up the code
    }

    void testMimeTypeAutoEmbed()
    {
        QMimeDatabase db;
        MimeTypeData data(db.mimeTypeForName(QStringLiteral("text/plain")));
        QCOMPARE(data.majorType(), QStringLiteral("text"));
        QCOMPARE(data.minorType(), QStringLiteral("plain"));
        QCOMPARE(data.name(), QStringLiteral("text/plain"));
        QVERIFY(!data.isMeta());
        QCOMPARE(data.autoEmbed(), MimeTypeData::UseGroupSetting);
        QVERIFY(!data.isDirty());
        data.setAutoEmbed(MimeTypeData::Yes);
        QCOMPARE(data.autoEmbed(), MimeTypeData::Yes);
        QVERIFY(data.isDirty());
        QVERIFY(!data.sync()); // save to disk. Should succeed, but return false (no need to run update-mime-database)
        QVERIFY(!data.isDirty());
        // Check what's on disk by creating another MimeTypeData instance
        MimeTypeData data2(db.mimeTypeForName(QStringLiteral("text/plain")));
        QCOMPARE(data2.autoEmbed(), MimeTypeData::Yes);
        QVERIFY(!data2.isDirty());
        data2.setAutoEmbed(MimeTypeData::UseGroupSetting); // revert to default, for next time
        QVERIFY(data2.isDirty());
        QVERIFY(!data2.sync());
        QVERIFY(!data2.isDirty());
    }

    void testMimeTypePatterns()
    {
        // Given the text/plain mimetype
        QMimeDatabase db;
        MimeTypeData data(db.mimeTypeForName(QStringLiteral("text/plain")));
        QCOMPARE(data.name(), QStringLiteral("text/plain"));
        QCOMPARE(data.majorType(), QStringLiteral("text"));
        QCOMPARE(data.minorType(), QStringLiteral("plain"));
        QVERIFY(!data.isMeta());
        QStringList patterns = data.patterns();
        QVERIFY(patterns.contains(QStringLiteral("*.txt")));
        QVERIFY(!patterns.contains(QStringLiteral("*.toto")));

        // When the user changes the patterns
        const QStringList origPatterns = patterns;
        patterns.removeAll(QStringLiteral("*.txt"));
        patterns.append(QStringLiteral("*.toto")); // yes, a french guy wrote this, as you can see
        patterns.sort(); // for future comparisons
        QVERIFY(!data.isDirty());
        data.setPatterns(patterns);
        QVERIFY(data.isDirty());
        const bool needUpdateMimeDb = data.sync();
        QVERIFY(needUpdateMimeDb);
        MimeTypeWriter::runUpdateMimeDatabase();

        // Then the GUI and the QMimeDatabase API should show the new patterns
        QCOMPARE(data.patterns(), patterns);
        data.refresh(); // reload from the xml
        QCOMPARE(data.patterns(), patterns);
        // Check what's in QMimeDatabase
        QStringList newPatterns = db.mimeTypeForName(QStringLiteral("text/plain")).globPatterns();
        newPatterns.sort();
        QCOMPARE(newPatterns, patterns);
        QVERIFY(!data.isDirty());

        // And then removing the custom file by hand should revert to the initial state
        const QString packageFileName =
            QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/mime/") + QStringLiteral("packages/text-plain.xml");
        QVERIFY(!packageFileName.isEmpty());
        QFile::remove(packageFileName);
        MimeTypeWriter::runUpdateMimeDatabase();
        // Check what's in QMimeDatabase
        newPatterns = db.mimeTypeForName(QStringLiteral("text/plain")).globPatterns();
        newPatterns.sort();
        QCOMPARE(newPatterns, origPatterns);
    }

    void testAddService()
    {
        QMimeDatabase db;
        QString mimeTypeName = QStringLiteral("application/rtf"); // use inherited mimetype to test #321706
        MimeTypeData data(db.mimeTypeForName(mimeTypeName));
        QStringList appServices = data.appServices();
        // qDebug() << appServices;
        QVERIFY(appServices.contains(fakeApplication2));
        QVERIFY(!appServices.contains(fakeApplication)); // already there? hmm can't really test then
        QVERIFY(!data.isDirty());
        appServices.prepend(fakeApplication);
        data.setAppServices(appServices);
        QVERIFY(data.isDirty());
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        QVERIFY(!data.isDirty());
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in kpartsrc
        checkAddedAssociationsContains(mimeTypeName, fakeApplication);

        // Test reordering apps, i.e. move fakeApplication under oldPreferredApp
        appServices.removeFirst();
        appServices.insert(1, fakeApplication);
        data.setAppServices(appServices);
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        QVERIFY(!data.isDirty());
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in kpartsrc
        checkAddedAssociationsContains(mimeTypeName, fakeApplication);

        // Then we get the signal that kbuildsycoca changed
        data.refresh();

        // Now test removing (in the same test, since it's inter-dependent)
        QVERIFY(appServices.removeAll(fakeApplication) > 0);
        data.setAppServices(appServices);
        QVERIFY(data.isDirty());
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication);
    }

    void testRemoveTwice()
    {
        QMimeDatabase db;
        // Remove fakeApplication from image/png
        QString mimeTypeName = QStringLiteral("image/png");
        MimeTypeData data(db.mimeTypeForName(mimeTypeName));
        QStringList appServices = data.appServices();
        qDebug() << "initial list for" << mimeTypeName << appServices;
        QVERIFY(appServices.removeAll(fakeApplication) > 0);
        data.setAppServices(appServices);
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication);

        // Remove fakeApplication2 from image/png; must keep the previous entry in "Removed Associations"
        qDebug() << "Removing fakeApplication2";
        QVERIFY(appServices.removeAll(fakeApplication2) > 0);
        data.setAppServices(appServices);
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication2);

        // And now re-add fakeApplication2...
        qDebug() << "Re-adding fakeApplication2";
        appServices.prepend(fakeApplication2);
        data.setAppServices(appServices);
        QVERIFY(!data.sync()); // success, but no need to run update-mime-database
        runKBuildSycoca();
        // Check what's in ksycoca
        checkMimeTypeServices(mimeTypeName, appServices);
        // Check what's in mimeapps.list
        checkRemovedAssociationsContains(mimeTypeName, fakeApplication);
        checkRemovedAssociationsDoesNotContain(mimeTypeName, fakeApplication2);
    }

    void testCreateMimeType()
    {
        QMimeDatabase db;
        const QString mimeTypeName = QStringLiteral("fake/unit-test-fake-mimetype");
        // Clean up after previous runs if necessary
        if (MimeTypeWriter::hasDefinitionFile(mimeTypeName)) {
            MimeTypeWriter::removeOwnMimeType(mimeTypeName);
        }

        MimeTypeData data(mimeTypeName, true);
        data.setComment(QStringLiteral("Fake MimeType"));
        QStringList patterns = QStringList() << QStringLiteral("*.pkg.tar.gz");
        data.setPatterns(patterns);
        QVERIFY(data.isDirty());
        QVERIFY(data.sync());
        MimeTypeWriter::runUpdateMimeDatabase();
        // QMimeDatabase doesn't even try to update the cache if less than
        // 5000 ms have passed (can't use qmime_secondsBetweenChecks)
        QTest::qSleep(5000);
        QMimeType mime = db.mimeTypeForName(mimeTypeName);
        QVERIFY(mime.isValid());
        QCOMPARE(mime.comment(), QStringLiteral("Fake MimeType"));
        QCOMPARE(mime.globPatterns(), patterns); // must sort them if more than one

        // Testcase for the shaman.xml bug
        QCOMPARE(db.mimeTypeForFile(QStringLiteral("/whatever/foo.pkg.tar.gz")).name(), QStringLiteral("fake/unit-test-fake-mimetype"));

        m_mimeTypeCreatedSuccessfully = true;
    }

    void testDeleteMimeType()
    {
        QMimeDatabase db;
        if (!m_mimeTypeCreatedSuccessfully) {
            QSKIP("This test relies on testCreateMimeType");
        }
        const QString mimeTypeName = QStringLiteral("fake/unit-test-fake-mimetype");
        QVERIFY(MimeTypeWriter::hasDefinitionFile(mimeTypeName));
        MimeTypeWriter::removeOwnMimeType(mimeTypeName);
        MimeTypeWriter::runUpdateMimeDatabase();
        // QMimeDatabase doesn't even try to update the cache if less than
        // 5000 ms have passed (can't use qmime_secondsBetweenChecks)
        QTest::qSleep(5000);
        const QMimeType mime = db.mimeTypeForName(mimeTypeName);
        QVERIFY2(!mime.isValid(), qPrintable(mimeTypeName));
    }

    void testModifyMimeTypeComment() // of a system mimetype. And check that it's re-read correctly.
    {
        QMimeDatabase db;
        QString mimeTypeName = QStringLiteral("image/png");
        MimeTypeData data(db.mimeTypeForName(mimeTypeName));
        QCOMPARE(data.comment(), QString::fromLatin1("PNG image"));
        QString fakeComment = QStringLiteral("PNG image [testing]");
        data.setComment(fakeComment);
        QVERIFY(data.isDirty());
        QVERIFY(data.sync());
        MimeTypeWriter::runUpdateMimeDatabase();
        QMimeType mime = db.mimeTypeForName(mimeTypeName);
        QVERIFY(mime.isValid());
        QCOMPARE(mime.comment(), fakeComment);

        // Cleanup
        QVERIFY(MimeTypeWriter::hasDefinitionFile(mimeTypeName));
        MimeTypeWriter::removeOwnMimeType(mimeTypeName);
    }

    void cleanupTestCase()
    {
        const QString localAppsDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
        QFile::remove(localAppsDir + QLatin1String("/fakeapplication.desktop"));
        QFile::remove(localAppsDir + QLatin1String("/fakeapplication2.desktop"));
    }

private: // helper methods
    void checkAddedAssociationsContains(const QString &mimeTypeName, const QString &application)
    {
        const KConfig config(m_localConfig.filePath(QStringLiteral("mimeapps.list")), KConfig::NoGlobals);
        const KConfigGroup group(&config, "Added Associations");
        const QStringList addedEntries = group.readXdgListEntry(mimeTypeName);
        if (!addedEntries.contains(application)) {
            qWarning() << addedEntries << "does not contain" << application;
            QVERIFY(addedEntries.contains(application));
        }
    }

    void checkRemovedAssociationsContains(const QString &mimeTypeName, const QString &application)
    {
        const KConfig config(m_localConfig.filePath(QStringLiteral("mimeapps.list")), KConfig::NoGlobals);
        const KConfigGroup group(&config, "Removed Associations");
        const QStringList removedEntries = group.readXdgListEntry(mimeTypeName);
        if (!removedEntries.contains(application)) {
            qWarning() << removedEntries << "does not contain" << application;
            QVERIFY(removedEntries.contains(application));
        }
    }

    void checkRemovedAssociationsDoesNotContain(const QString &mimeTypeName, const QString &application)
    {
        const KConfig config(m_localConfig.filePath(QStringLiteral("mimeapps.list")), KConfig::NoGlobals);
        const KConfigGroup group(&config, "Removed Associations");
        const QStringList removedEntries = group.readXdgListEntry(mimeTypeName);
        if (removedEntries.contains(application)) {
            qWarning() << removedEntries << "contains" << application;
            QVERIFY(!removedEntries.contains(application));
        }
    }

    void runKBuildSycoca()
    {
        // Wait for notifyDatabaseChanged DBus signal
        // (The real KCM code simply does the refresh in a slot, asynchronously)

        QProcess proc;
        // proc.setProcessChannelMode(QProcess::ForwardedChannels);
        const QString kbuildsycoca = QStandardPaths::findExecutable(QLatin1String(KBUILDSYCOCA_EXENAME));
        QVERIFY(!kbuildsycoca.isEmpty());
        QStringList args;
        args << QStringLiteral("--testmode");
        proc.start(kbuildsycoca, args);
        QSignalSpy spy(KSycoca::self(), qOverload<>(&KSycoca::databaseChanged));
        proc.waitForFinished();
        qDebug() << "waiting for signal";
        QVERIFY(spy.wait(10000));
        qDebug() << "got signal";
    }

    void createDesktopFile(const QString &path, const QStringList &mimeTypes)
    {
        KDesktopFile file(path);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "FakeApplication");
        group.writeEntry("Type", "Application");
        group.writeEntry("Exec", "ls");
        group.writeXdgListEntry("MimeType", mimeTypes);
    }

    void checkMimeTypeServices(const QString &mimeTypeName, const QStringList &expectedServices)
    {
        QMimeDatabase db;
        MimeTypeData data2(db.mimeTypeForName(mimeTypeName));
        if (data2.appServices() != expectedServices) {
            qDebug() << "got" << data2.appServices() << "expected" << expectedServices;
        }
        QCOMPARE(data2.appServices(), expectedServices);
    }

    QString fakeApplication; // storage id of the fake application
    QString fakeApplication2; // storage id of the fake application2
    QString m_localApps;
    QDir m_localConfig;
    bool m_mimeTypeCreatedSuccessfully;
};

QTEST_MAIN(FileTypesTest)

#include "filetypestest.moc"
