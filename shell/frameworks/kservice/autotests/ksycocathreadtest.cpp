/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QSignalSpy>
#include <QTest>

#include <QDebug>
#include <QFile>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <kbuildsycoca_p.h>

#include <KConfig>
#include <KConfigGroup>
#include <KDesktopFile>
#include <QMutex>
#include <kservicegroup.h>
#include <ksycoca.h>

#include "setupxdgdirs.h"

static QString fakeAppDesktopFile()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String{"/applications/org.kde.testapp.desktop"};
}

static QSet<QThread *> s_threadsWhoSawFakeService; // clazy:exclude=non-pod-global-static
static QMutex s_setMutex; // clazy:exclude=non-pod-global-static
static int threadsWhoSawFakeService()
{
    QMutexLocker locker(&s_setMutex);
    return s_threadsWhoSawFakeService.count();
}
static QAtomicInt s_fakeServiceDeleted = 0; // clazy:exclude=non-pod-global-static

class WorkerObject : public QObject
{
    Q_OBJECT
public:
    WorkerObject()
        : QObject()
    {
    }

public Q_SLOTS:
    void work()
    {
        // qDebug() << QThread::currentThread() << "working...";

        QMimeDatabase db;
        const QList<QMimeType> allMimeTypes = db.allMimeTypes();
        Q_ASSERT(!allMimeTypes.isEmpty());

        const KService::List lst = KService::allServices();
        Q_ASSERT(!lst.isEmpty());

        for (const KService::Ptr &service : lst) {
            Q_ASSERT(service->isType(KST_KService));
            const QString name = service->name();
            const QString entryPath = service->entryPath();
            // qDebug() << name << "entryPath=" << entryPath << "menuId=" << service->menuId();
            Q_ASSERT(!name.isEmpty());
            Q_ASSERT(!entryPath.isEmpty());

            KService::Ptr lookedupService = KService::serviceByDesktopPath(entryPath);
            if (!lookedupService) {
                if (entryPath == QLatin1String{"threadfakeservice.desktop"} && s_fakeServiceDeleted) { // ok, it got deleted meanwhile
                    continue;
                }
                qWarning() << entryPath << "is gone!";
            }
            Q_ASSERT(lookedupService); // not null
            QCOMPARE(lookedupService->entryPath(), entryPath);

            if (service->isApplication()) {
                const QString menuId = service->menuId();
                if (menuId.isEmpty()) {
                    qWarning("%s has an empty menuId!", qPrintable(entryPath));
                }
                Q_ASSERT(!menuId.isEmpty());
                lookedupService = KService::serviceByMenuId(menuId);
                if (!lookedupService) {
                    if (menuId == QLatin1String{"threadfakeservice"} && s_fakeServiceDeleted) { // ok, it got deleted meanwhile
                        continue;
                    }
                    qWarning() << menuId << "is gone!";
                }
                Q_ASSERT(lookedupService); // not null
                QCOMPARE(lookedupService->menuId(), menuId);
            }
        }

        KServiceGroup::Ptr root = KServiceGroup::root();
        Q_ASSERT(root);

        if (KService::serviceByDesktopPath(QStringLiteral("threadfakeservice.desktop"))) {
            QMutexLocker locker(&s_setMutex);
            s_threadsWhoSawFakeService.insert(QThread::currentThread());
        }
    }
};

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    WorkerThread()
        : QThread()
        , m_stop(false)
    {
    }
    void run() override
    {
        WorkerObject wo;
        while (!m_stop) {
            wo.work();
        }
    }
    virtual void stop()
    {
        m_stop = true;
    }

private:
    QAtomicInt m_stop; // bool
};

/**
 * Threads with an event loop will be able to process "database changed" signals.
 * Threads without an event loop (like WorkerThread) cannot, so they will keep using
 * the old data.
 */
class EventLoopThread : public WorkerThread
{
    Q_OBJECT
public:
    void run() override
    {
        // WorkerObject must belong to this thread, this is why we don't
        // have the slot work() in WorkerThread itself. Typical QThread trap!
        WorkerObject wo;
        QTimer timer;
        connect(&timer, &QTimer::timeout, &wo, &WorkerObject::work);
        timer.start(100);
        exec();
    }
    void stop() override
    {
        quit();
    }
};

// This code runs in the main thread
class KSycocaThreadTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCreateService();
    void testDeleteService()
    {
        deleteFakeService();
        QTimer::singleShot(1000, this, SLOT(slotFinish()));
    }
    void slotFinish()
    {
        qDebug() << "Terminating";
        for (int i = 0; i < threads.size(); i++) {
            threads[i]->stop();
        }
        for (int i = 0; i < threads.size(); i++) {
            threads[i]->wait();
        }
        cleanupTestCase();
        QCoreApplication::instance()->quit();
    }

private:
    void createFakeService();
    void deleteFakeService();
    QList<WorkerThread *> threads;
};

static void runKBuildSycoca()
{
    QSignalSpy spy(KSycoca::self(), &KSycoca::databaseChanged);

    KBuildSycoca builder;
    QVERIFY(builder.recreate());
    qDebug() << "waiting for signal";
    QVERIFY(spy.wait(20000));
    qDebug() << "got signal";
}

void KSycocaThreadTest::initTestCase()
{
    // Set up a layer in the bin dir so ksycoca finds the Application servicetypes
    setupXdgDirs();
    QStandardPaths::setTestModeEnabled(true);

    createFakeService();

    qDebug() << "Created org.kde.testapp, running kbuilsycoca";
    runKBuildSycoca();
    // Process the event
    int count = 0;
    while (!KService::serviceByDesktopName(QStringLiteral("org.kde.testapp"))) {
        qApp->processEvents();
        if (++count == 20) {
            qFatal("sycoca doesn't have org.kde.testapp.desktop");
        }
    }
}

void KSycocaThreadTest::cleanupTestCase()
{
    QFile::remove(fakeAppDesktopFile());
}

// duplicated from kcoreaddons/autotests/kdirwatch_unittest.cpp
static void waitUntilAfter(const QDateTime &ctime)
{
    int totalWait = 0;
    QDateTime now;
    Q_FOREVER {
        now = QDateTime::currentDateTime();
        if (now.toSecsSinceEpoch() == ctime.toSecsSinceEpoch()) // truncate milliseconds
        {
            totalWait += 50;
            QTest::qWait(50);
        } else {
            QVERIFY(now > ctime); // can't go back in time ;)
            QTest::qWait(50); // be safe
            break;
        }
    }
    // if (totalWait > 0)
    qDebug() << "Waited" << totalWait << "ms so that now" << now.toString(Qt::ISODate) << "is >" << ctime.toString(Qt::ISODate);
}

void KSycocaThreadTest::testCreateService()
{
    // Wait one second so that ksycoca can detect a mtime change
    // ## IMHO this is a Qt bug, QFileInfo::lastModified() should include milliseconds
    waitUntilAfter(QDateTime::currentDateTime());

    createFakeService();
    QVERIFY(QFile::exists(fakeAppDesktopFile()));
    qDebug() << "executing kbuildsycoca (1)";
    runKBuildSycoca();

    QTRY_VERIFY(KService::serviceByDesktopName(QStringLiteral("org.kde.testapp")));

    // Now wait to check that all threads saw that new service
    QTRY_COMPARE_WITH_TIMEOUT(threadsWhoSawFakeService(), threads.size(), 20000);
}

void KSycocaThreadTest::deleteFakeService()
{
    s_fakeServiceDeleted = 1;

    qDebug() << "now deleting the fake service";
    KService::Ptr fakeService = KService::serviceByDesktopName(QStringLiteral("org.kde.testapp"));
    QVERIFY(fakeService);
    const QString servPath = fakeAppDesktopFile();
    QFile::remove(servPath);

    QSignalSpy spy(KSycoca::self(), &KSycoca::databaseChanged);

    QVERIFY(spy.isValid());

    qDebug() << "executing kbuildsycoca (2)";
    runKBuildSycoca();

    QCOMPARE(spy.count(), 1);

    QVERIFY(fakeService); // the whole point of refcounting is that this KService instance is still valid.
    QVERIFY(!QFile::exists(servPath));
}

void KSycocaThreadTest::createFakeService()
{
    KDesktopFile file(fakeAppDesktopFile());
    KConfigGroup group = file.desktopGroup();
    group.writeEntry("Name", "Foo");
    group.writeEntry("Type", "Application");
    group.writeEntry("Exec", "bla");
}

QTEST_MAIN(KSycocaThreadTest)
#include "ksycocathreadtest.moc"
