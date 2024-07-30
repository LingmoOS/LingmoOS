/*
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kservicetest.h"

#include "setupxdgdirs.h"

#include <locale.h>

#include <QTest>

#include <../src/services/kserviceutil_p.h> // for KServiceUtilPrivate
#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>
#include <kapplicationtrader.h>
#include <kbuildsycoca_p.h>
#include <ksycoca.h>

#include <KPluginMetaData>
#include <kservicegroup.h>

#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QThread>

#include <QDebug>
#include <QLoggingCategory>
#include <QMimeDatabase>

QTEST_MAIN(KServiceTest)

extern KSERVICE_EXPORT int ksycoca_ms_between_checks;

static void eraseProfiles()
{
    QString profilerc = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String{"/profilerc"};
    if (!profilerc.isEmpty()) {
        QFile::remove(profilerc);
    }

    profilerc = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String{"/servicetype_profilerc"};
    if (!profilerc.isEmpty()) {
        QFile::remove(profilerc);
    }
}

void KServiceTest::initTestCase()
{
    // Set up a layer in the bin dir so ksycoca finds the Application servicetypes
    setupXdgDirs();
    QStandardPaths::setTestModeEnabled(true);

    // A non-C locale is necessary for some tests.
    // This locale must have the following properties:
    //   - some character other than dot as decimal separator
    // If it cannot be set, locale-dependent tests are skipped.
    setlocale(LC_ALL, "fr_FR.utf8");
    QLocale::setDefault(QLocale(QStringLiteral("fr_FR")));
    m_hasNonCLocale = (setlocale(LC_ALL, nullptr) == QByteArray("fr_FR.utf8"));
    if (!m_hasNonCLocale) {
        qDebug() << "Setting locale to fr_FR.utf8 failed";
    }

    eraseProfiles();

    if (!KSycoca::isAvailable()) {
        runKBuildSycoca();
    }

    // Create some fake services for the tests below, and ensure they are in ksycoca.

    bool mustUpdateKSycoca = false;

    // org.kde.deleteme.desktop: deleted and recreated by testDeletingService, don't use in other tests
    const QString deleteMeApp = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1String("/org.kde.deleteme.desktop");
    if (!QFile::exists(deleteMeApp)) {
        QVERIFY(QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)));
        const QString src = QFINDTESTDATA("org.kde.deleteme.desktop");
        QVERIFY(!src.isEmpty());
        QVERIFY2(QFile::copy(src, deleteMeApp), qPrintable(deleteMeApp));
        qDebug() << "Created" << deleteMeApp;
        mustUpdateKSycoca = true;
    }

    // org.kde.faketestapp.desktop
    const QString testApp = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1String("/org.kde.faketestapp.desktop");
    if (!QFile::exists(testApp)) {
        QVERIFY(QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)));
        const QString src = QFINDTESTDATA("org.kde.faketestapp.desktop");
        QVERIFY(!src.isEmpty());
        QVERIFY2(QFile::copy(src, testApp), qPrintable(testApp));
        qDebug() << "Created" << testApp;
        mustUpdateKSycoca = true;
    }

    // otherfakeapp.desktop
    const QString otherTestApp = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1String("/org.kde.otherfakeapp.desktop");
    if (!QFile::exists(otherTestApp)) {
        QVERIFY(QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)));
        const QString src = QFINDTESTDATA("org.kde.otherfakeapp.desktop");
        QVERIFY(!src.isEmpty());
        QVERIFY2(QFile::copy(src, otherTestApp), qPrintable(otherTestApp));
        qDebug() << "Created" << otherTestApp;
        mustUpdateKSycoca = true;
    }
    // testnames.desktop
    const QString namesTestApp = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1String("/org.kde.testnames.desktop");
    if (!QFile::exists(namesTestApp)) {
        QVERIFY(QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation)));
        const QString src = QFINDTESTDATA("org.kde.testnames.desktop");
        QVERIFY(!src.isEmpty());
        QVERIFY2(QFile::copy(src, namesTestApp), qPrintable(namesTestApp));
        qDebug() << "Created" << namesTestApp;
        mustUpdateKSycoca = true;
    }

    if (mustUpdateKSycoca) {
        // Update ksycoca in ~/.qttest after creating the above
        runKBuildSycoca(true);
    }
    QVERIFY(KService::serviceByDesktopName(QStringLiteral("org.kde.faketestapp")));
    QVERIFY(KService::serviceByDesktopName(QStringLiteral("org.kde.otherfakeapp")));
    QVERIFY(KService::serviceByDesktopName(QStringLiteral("org.kde.testnames")));
}

void KServiceTest::runKBuildSycoca(bool noincremental)
{
    QSignalSpy spy(KSycoca::self(), &KSycoca::databaseChanged);

    KBuildSycoca builder;
    QVERIFY(builder.recreate(!noincremental));
    if (spy.isEmpty()) {
        qDebug() << "waiting for signal";
        QVERIFY(spy.wait(10000));
        qDebug() << "got signal";
    }
}

void KServiceTest::cleanupTestCase()
{
    KBuildSycoca builder;
    builder.recreate();
}

void KServiceTest::testByName()
{
    if (!KSycoca::isAvailable()) {
        QSKIP("ksycoca not available");
    }

    const QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("applications/org.kde.faketestapp.desktop"));
    KService::Ptr myService = KService::serviceByDesktopPath(filePath);
    QVERIFY(myService);
    QCOMPARE(myService->name(), QStringLiteral("Konsole"));
}

void KServiceTest::testConstructorFullPath()
{
    const QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("applications/org.kde.faketestapp.desktop"));
    QVERIFY(QFile::exists(filePath));
    KService service(filePath);
    QVERIFY(service.isValid());
    QCOMPARE(service.name(), QStringLiteral("Konsole"));
}

void KServiceTest::testConstructorKDesktopFile() // as happens inside kbuildsycoca.cpp
{
    const QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("applications/org.kde.faketestapp.desktop"));
    KDesktopFile desktopFile(filePath);
    QCOMPARE(KService(&desktopFile, filePath).name(), QStringLiteral("Konsole"));
}

void KServiceTest::testCopyConstructor()
{
    const QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("applications/org.kde.faketestapp.desktop"));
    QVERIFY(QFile::exists(filePath));
    KDesktopFile desktopFile(filePath);
    // Test making a copy of a KService that will go out of scope
    KService::Ptr service;
    {
        KService origService(&desktopFile);
        service = new KService(origService);
    }
    QVERIFY(service->isValid());
    QCOMPARE(service->name(), QStringLiteral("Konsole"));
}

void KServiceTest::testCopyInvalidService()
{
    KService::Ptr service;
    {
        KService origService{QString()}; // this still sets a d_ptr, so no problem;
        QVERIFY(!origService.isValid());
        service = new KService(origService);
    }
    QVERIFY(!service->isValid());
}

void KServiceTest::testProperty()
{
    ksycoca_ms_between_checks = 0;

    // Let's try creating a desktop file and ensuring it's noticed by the timestamp check
    QTest::qWait(1000);
    QString fakeAppPath = KService::newServicePath(false, QStringLiteral("org.kde.foo"));
    if (!QFile::exists(fakeAppPath)) {
        KDesktopFile file(fakeAppPath);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "Foo");
        group.writeEntry("Type", "Application");
        group.writeEntry("X-Flatpak-RenamedFrom", "foo;bar;");
        group.writeEntry("Exec", "bla");
        group.writeEntry("X-GNOME-UsesNotifications", true);
        qDebug() << "created" << fakeAppPath;
    }

    KService::Ptr fakeApp = KService::serviceByDesktopName(QStringLiteral("org.kde.foo"));
    QVERIFY(fakeApp);
    QStringList expectedRename{QStringLiteral("foo"), QStringLiteral("bar")};
    QCOMPARE(fakeApp->property<QStringList>(QStringLiteral("X-Flatpak-RenamedFrom")), expectedRename);
    QCOMPARE(fakeApp->property<bool>(QStringLiteral("X-GNOME-UsesNotifications")), true);
    QVERIFY(!fakeApp->property<QString>(QStringLiteral("Name")).isEmpty());
    QVERIFY(fakeApp->property<QString>(QStringLiteral("Name[fr]")).isEmpty());

    // Restore value
    ksycoca_ms_between_checks = 1500;
}

void KServiceTest::testAllServices()
{
    if (!KSycoca::isAvailable()) {
        QSKIP("ksycoca not available");
    }
    const KService::List lst = KService::allServices();
    QVERIFY(!lst.isEmpty());
    bool foundTestApp = false;

    for (const KService::Ptr &service : lst) {
        QVERIFY(service->isType(KST_KService));

        const QString name = service->name();
        const QString entryPath = service->entryPath();
        if (entryPath.contains(QLatin1String{"fake"})) {
            qDebug() << name << "entryPath=" << entryPath << "menuId=" << service->menuId();
        }
        QVERIFY(!name.isEmpty());
        QVERIFY(!entryPath.isEmpty());

        KService::Ptr lookedupService = KService::serviceByDesktopPath(entryPath);
        QVERIFY(lookedupService); // not null
        QCOMPARE(lookedupService->entryPath(), entryPath);

        if (service->isApplication()) {
            const QString menuId = service->menuId();
            if (menuId.isEmpty()) {
                qWarning("%s has an empty menuId!", qPrintable(entryPath));
            } else if (menuId == QLatin1String{"org.kde.faketestapp.desktop"}) {
                foundTestApp = true;
            }
            QVERIFY(!menuId.isEmpty());
            lookedupService = KService::serviceByMenuId(menuId);
            QVERIFY(lookedupService); // not null
            QCOMPARE(lookedupService->menuId(), menuId);
        }
    }
    QVERIFY(foundTestApp);
}

void KServiceTest::testByStorageId()
{
    if (!KSycoca::isAvailable()) {
        QSKIP("ksycoca not available");
    }
    QVERIFY(!QStandardPaths::locate(QStandardPaths::ApplicationsLocation, QStringLiteral("org.kde.faketestapp.desktop")).isEmpty());
    QVERIFY(KService::serviceByMenuId(QStringLiteral("org.kde.faketestapp.desktop")));
    QVERIFY(!KService::serviceByMenuId(QStringLiteral("org.kde.faketestapp"))); // doesn't work, extension mandatory
    QVERIFY(!KService::serviceByMenuId(QStringLiteral("faketestapp.desktop"))); // doesn't work, full filename mandatory
    QVERIFY(KService::serviceByStorageId(QStringLiteral("org.kde.faketestapp.desktop")));
    QVERIFY(KService::serviceByStorageId(QStringLiteral("org.kde.faketestapp")));

    QVERIFY(KService::serviceByDesktopName(QStringLiteral("org.kde.faketestapp")));
    QCOMPARE(KService::serviceByDesktopName(QStringLiteral("org.kde.faketestapp"))->menuId(), QStringLiteral("org.kde.faketestapp.desktop"));
}

void KServiceTest::testSubseqConstraints()
{
    auto test = [](const char *pattern, const char *text, bool sensitive) {
        return KApplicationTrader::isSubsequence(QString::fromLatin1(pattern), QString::fromLatin1(text), sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
    };

    // Case Sensitive
    QVERIFY2(!test("", "", 1), "both empty");
    QVERIFY2(!test("", "something", 1), "empty pattern");
    QVERIFY2(!test("something", "", 1), "empty text");
    QVERIFY2(test("lngfile", "somereallylongfile", 1), "match ending");
    QVERIFY2(test("somelong", "somereallylongfile", 1), "match beginning");
    QVERIFY2(test("reallylong", "somereallylongfile", 1), "match middle");
    QVERIFY2(test("across", "a 23 c @#! r o01 o 5 s_s", 1), "match across");
    QVERIFY2(!test("nocigar", "soclosebutnociga", 1), "close but no match");
    QVERIFY2(!test("god", "dog", 1), "incorrect letter order");
    QVERIFY2(!test("mismatch", "mIsMaTcH", 1), "case sensitive mismatch");

    // Case insensitive
    QVERIFY2(test("mismatch", "mIsMaTcH", 0), "case insensitive match");
    QVERIFY2(test("tryhards", "Try Your Hardest", 0), "uppercase text");
    QVERIFY2(test("TRYHARDS", "try your hardest", 0), "uppercase pattern");
}

void KServiceTest::testActionsAndDataStream()
{
    KService::Ptr service = KService::serviceByStorageId(QStringLiteral("org.kde.faketestapp.desktop"));
    QVERIFY(service);
    QVERIFY(service->property<QString>(QStringLiteral("Name[fr]")).isEmpty());
    const QList<KServiceAction> actions = service->actions();
    QCOMPARE(actions.count(), 2); // NewWindow, NewTab
    const KServiceAction newTabAction = actions.at(1);
    QCOMPARE(newTabAction.name(), QStringLiteral("NewTab"));
    QCOMPARE(newTabAction.exec(), QStringLiteral("konsole --new-tab"));
    QCOMPARE(newTabAction.icon(), QStringLiteral("tab-new"));
    QCOMPARE(newTabAction.noDisplay(), false);
    QVERIFY(!newTabAction.isSeparator());
    QCOMPARE(newTabAction.service()->name(), service->name());
}

void KServiceTest::testServiceGroups()
{
    KServiceGroup::Ptr root = KServiceGroup::root();
    QVERIFY(root);
    qDebug() << root->groupEntries().count();

    KServiceGroup::Ptr group = root;
    QVERIFY(group);
    const KServiceGroup::List list = group->entries(true, // sorted
                                                    true, // exclude no display entries,
                                                    false, // allow separators
                                                    true); // sort by generic name

    qDebug() << list.count();
    for (KServiceGroup::SPtr s : list) {
        qDebug() << s->name() << s->entryPath();
    }

    // No unit test here yet, but at least this can be valgrinded for errors.
}

void KServiceTest::testDeletingService()
{
    // workaround unexplained inotify issue (in CI only...)
    QTest::qWait(1000);

    const QString serviceName = QStringLiteral("org.kde.deleteme");
    KService::Ptr fakeService = KService::serviceByDesktopName(serviceName);
    QVERIFY(fakeService); // see initTestCase; it should be found.

    // Test deleting a service
    const QString servPath = fakeService->locateLocal();
    QVERIFY(QFile::exists(servPath));
    QFile::remove(servPath);
    runKBuildSycoca();
    ksycoca_ms_between_checks = 0; // need it to check the ksycoca mtime
    QVERIFY(!KService::serviceByDesktopName(serviceName)); // not in ksycoca anymore

    // Restore value
    ksycoca_ms_between_checks = 1500;

    QVERIFY(fakeService); // the whole point of refcounting is that this KService instance is still valid.
    QVERIFY(!QFile::exists(servPath));

    // Recreate it, for future tests
    const QString deleteMeApp = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QLatin1String("/org.kde.deleteme.desktop");
    const QString src = QFINDTESTDATA("org.kde.deleteme.desktop");
    QVERIFY2(QFile::copy(src, deleteMeApp), qPrintable(deleteMeApp));

    runKBuildSycoca();

    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
        m_sycocaUpdateDone.ref();
    }
}

#include <QFutureSynchronizer>
#include <QThreadPool>
#include <QtConcurrentRun>

// Testing for concurrent access to ksycoca from multiple threads
// It's especially interesting to run this test as ./kservicetest testThreads
// so that even the ksycoca initialization is happening from N threads at the same time.
// Use valgrind --tool=helgrind to see the race conditions.

void KServiceTest::testReaderThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    QFutureSynchronizer<void> sync;
    sync.addFuture(QtConcurrent::run(&KServiceTest::testAllServices, this));
    sync.addFuture(QtConcurrent::run(&KServiceTest::testAllServices, this));
    sync.addFuture(QtConcurrent::run(&KServiceTest::testAllServices, this));
    sync.addFuture(QtConcurrent::run(&KServiceTest::testAllServices, this));
    sync.addFuture(QtConcurrent::run(&KServiceTest::testAllServices, this));
    sync.waitForFinished();
    QThreadPool::globalInstance()->setMaxThreadCount(1); // delete those threads
}

void KServiceTest::testThreads()
{
    QThreadPool::globalInstance()->setMaxThreadCount(10);
    QFutureSynchronizer<void> sync;
    sync.addFuture(QtConcurrent::run(&KServiceTest::testAllServices, this));
    sync.addFuture(QtConcurrent::run(&KServiceTest::testDeletingService, this));

    // process events (DBus, inotify...), until we get all expected signals
    QTRY_COMPARE_WITH_TIMEOUT(m_sycocaUpdateDone.loadRelaxed(), 1, 15000); // not using a bool, just to silence helgrind
    qDebug() << "Joining all threads";
    sync.waitForFinished();
}

void KServiceTest::testCompleteBaseName()
{
    // dots in filename before .desktop extension:
    QCOMPARE(KServiceUtilPrivate::completeBaseName(QStringLiteral("/home/x/.local/share/applications/org.kde.fakeapp.desktop")),
             QStringLiteral("org.kde.fakeapp"));
}

void KServiceTest::testEntryPathToName()
{
    QCOMPARE(KService(QStringLiteral("c.desktop")).name(), QStringLiteral("c"));
    QCOMPARE(KService(QStringLiteral("a.b.c.desktop")).name(), QStringLiteral("a.b.c")); // dots in filename before .desktop extension
    QCOMPARE(KService(QStringLiteral("/hallo/a.b.c.desktop")).name(), QStringLiteral("a.b.c"));
}

void KServiceTest::testAliasFor()
{
    if (!KSycoca::isAvailable()) {
        QSKIP("ksycoca not available");
    }
    KService::Ptr testapp = KService::serviceByDesktopName(QStringLiteral("org.kde.faketestapp"));
    QVERIFY(testapp);
    QCOMPARE(testapp->aliasFor(), QStringLiteral("org.kde.okular"));
}

void KServiceTest::testMimeType()
{
    if (!KSycoca::isAvailable()) {
        QSKIP("ksycoca not available");
    }

    KService::Ptr testapp = KService::serviceByDesktopName(QStringLiteral("org.kde.otherfakeapp"));
    QVERIFY(testapp);
    QCOMPARE(testapp->mimeTypes(), {QStringLiteral("application/pdf")});
}

void KServiceTest::testProtocols()
{
    if (!KSycoca::isAvailable()) {
        QSKIP("ksycoca not available");
    }

    KService::Ptr testapp = KService::serviceByDesktopName(QStringLiteral("org.kde.otherfakeapp"));
    QVERIFY(testapp);
    QStringList expectedProtocols{QStringLiteral("http"), QStringLiteral("tel")};
    QCOMPARE(testapp->supportedProtocols(), expectedProtocols);
}

void KServiceTest::testServiceActionService()
{
    if (!KSycoca::isAvailable()) {
        QSKIP("ksycoca not available");
    }

    const QString filePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("applications/org.kde.faketestapp.desktop"));
    QVERIFY(QFile::exists(filePath));
    KService service(filePath);
    QVERIFY(service.isValid());

    const KServiceAction action = service.actions().first();
    QCOMPARE(action.service()->property<bool>(QStringLiteral("DBusActivatable")), true);
    QCOMPARE(action.service()->actions().size(), 2);
}

void KServiceTest::testUntranslatedNames()
{
    const QString name = QStringLiteral("Name");
    const QString genericName = QStringLiteral("GenericName");
    QLatin1String translationPostfix(" trans");

    KService::Ptr app = KService::serviceByDesktopName(QStringLiteral("org.kde.testnames"));
    QVERIFY(app);
    QVERIFY(app->isValid());
    QCOMPARE(app->untranslatedName(), name);
    QCOMPARE(app->untranslatedGenericName(), genericName);
    QCOMPARE(app->name(), name + translationPostfix);
    QCOMPARE(app->genericName(), genericName + translationPostfix);
    // Property access
    QCOMPARE(app->property<QString>(QStringLiteral("UntranslatedName")), name);
    QCOMPARE(app->property<QString>(QStringLiteral("UntranslatedGenericName")), genericName);
}

void KServiceTest::testStartupNotify()
{
    {
        QString fakeAppPath = KService::newServicePath(false, QStringLiteral("org.kde.startupnotify1"));
        KDesktopFile file(fakeAppPath);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "Foo");
        group.writeEntry("Type", "Application");
        group.writeEntry("StartupNotify", true);
        group.sync();

        KService fakeApp(fakeAppPath);
        QVERIFY(fakeApp.isValid());
        QVERIFY(fakeApp.startupNotify().value());
    }

    {
        QString fakeAppPath = KService::newServicePath(false, QStringLiteral("org.kde.startupnotify2"));
        KDesktopFile file(fakeAppPath);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "Foo");
        group.writeEntry("Type", "Application");
        group.writeEntry("StartupNotify", false);
        group.sync();

        KService fakeApp(fakeAppPath);
        QVERIFY(fakeApp.isValid());
        QVERIFY(!fakeApp.startupNotify().value());
    }

    {
        QString fakeAppPath = KService::newServicePath(false, QStringLiteral("org.kde.startupnotify1"));
        KDesktopFile file(fakeAppPath);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "Foo");
        group.writeEntry("Type", "Application");
        group.writeEntry("X-KDE-StartupNotify", true);
        group.sync();

        KService fakeApp(fakeAppPath);
        QVERIFY(fakeApp.isValid());
        QVERIFY(fakeApp.startupNotify().value());
    }

    {
        QString fakeAppPath = KService::newServicePath(false, QStringLiteral("org.kde.startupnotify2"));
        KDesktopFile file(fakeAppPath);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "Foo");
        group.writeEntry("Type", "Application");
        group.writeEntry("X-KDE-StartupNotify", false);
        group.sync();

        KService fakeApp(fakeAppPath);
        QVERIFY(fakeApp.isValid());
        QVERIFY(!fakeApp.startupNotify().value());
    }

    {
        QString fakeAppPath = KService::newServicePath(false, QStringLiteral("org.kde.startupnotify3"));
        KDesktopFile file(fakeAppPath);
        KConfigGroup group = file.desktopGroup();
        group.writeEntry("Name", "Foo");
        group.writeEntry("Type", "Application");
        group.sync();

        KService fakeApp(fakeAppPath);
        QVERIFY(fakeApp.isValid());
        QVERIFY(!fakeApp.startupNotify().has_value());
    }
}

#include "moc_kservicetest.cpp"
