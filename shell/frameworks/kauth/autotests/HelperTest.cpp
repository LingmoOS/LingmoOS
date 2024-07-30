/*
    SPDX-FileCopyrightText: 2012 Dario Freddi <drf@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "BackendsManager.h"
#include "TestHelper.h"

#include <kauth/actionreply.h>
#include <kauth/executejob.h>

#include <QRandomGenerator>
#include <QSignalSpy>
#include <QTest>
#include <QThread>
#include <QTimer>

#include "../src/backends/dbus/DBusHelperProxy.h"

Q_DECLARE_METATYPE(KAuth::Action::AuthStatus)

class HelperHandler : public QObject
{
    Q_OBJECT
public:
    HelperHandler();

private Q_SLOTS:
    void init();

Q_SIGNALS:
    void ready();

private:
    DBusHelperProxy *m_helperProxy;
    TestHelper *m_helper;
    QThread *m_thread;
};

class HelperTest : public QObject
{
    Q_OBJECT

public:
    HelperTest(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();
    void init()
    {
    }

    void testBasicActionExecution();
    void testExecuteJobSignals();
    void testTwoCalls();
    void testActionData();
    void testHelperFailure();

    void cleanup()
    {
    }
    void cleanupTestCase()
    {
    }

Q_SIGNALS:
    void changeCapabilities(KAuth::AuthBackend::Capabilities capabilities);

private:
    HelperHandler *m_handler;
};

HelperHandler::HelperHandler()
    : QObject(nullptr)
{
    /* Hello adventurer. What you see here might hurt your eyes, but let me explain why you don't want
       to touch this code. We are dealing with same-process async DBus requests, and if this seems obscure
       to you already please give up and forget about this function.
       Qt's local loop optimizations at the moment make it impossible to stream an async request to a process
       living on the same thread. So that's what we do: we instantiate a separate helperProxy and move it to
       a different thread - afterwards we can do everything as if we were in a separate process.

       If you are wondering if this means we'll have two helper proxies, you are right my friend. But please
       remember that helperProxy acts both as a client and as a server, so it makes total sense.

       tl;dr: Don't touch this and forget the weird questions in your head: it actually works.
    */

    m_thread = new QThread(this);
    moveToThread(m_thread);
    connect(m_thread, &QThread::started, this, &HelperHandler::init);
    m_thread->start();
}

void HelperHandler::init()
{
    qDebug() << "Initializing helper handler";
    qputenv("KAUTH_TEST_CALLER_ID", QDBusConnection::sessionBus().baseService().toUtf8());
    // Set up our Helper - of course, it is not in a separate process here so we need to copy what
    // HelperProxy::helperMain() does
    m_helperProxy = new DBusHelperProxy(QDBusConnection::sessionBus());
    m_helper = new TestHelper;
    // The timer is here just to prevent the app from crashing.
    QTimer *timer = new QTimer(nullptr);

    QVERIFY(m_helperProxy->initHelper(QLatin1String("org.kde.kf6auth.autotest")));

    m_helperProxy->setHelperResponder(m_helper);

    m_helper->setProperty("__KAuth_Helper_Shutdown_Timer", QVariant::fromValue(timer));
    timer->setInterval(10000);
    timer->start();

    // Make BackendsManager aware
    BackendsManager::setProxyForThread(m_thread, m_helperProxy);

    Q_EMIT ready();
}

void HelperTest::initTestCase()
{
    connect(this,
            SIGNAL(changeCapabilities(KAuth::AuthBackend::Capabilities)),
            KAuth::BackendsManager::authBackend(),
            SLOT(setNewCapabilities(KAuth::AuthBackend::Capabilities)));

    qRegisterMetaType<KAuth::Action::AuthStatus>();
    qRegisterMetaType<KJob *>();

    // Set up our HelperHandler
    m_handler = new HelperHandler;
    QEventLoop eventLoop;
    connect(m_handler, &HelperHandler::ready, &eventLoop, &QEventLoop::quit);
    qDebug() << "Waiting for HelperHandler to be initialized";
    eventLoop.exec();
}

void HelperTest::testBasicActionExecution()
{
    Q_EMIT changeCapabilities(KAuth::AuthBackend::AuthorizeFromHelperCapability);

    KAuth::Action action(QLatin1String("org.kde.kf6auth.autotest.standardaction"));
    action.setHelperId(QLatin1String("org.kde.kf6auth.autotest"));
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    KAuth::ExecuteJob *job = action.execute();

    QVERIFY(job->exec());

    QVERIFY(!job->error());
    QVERIFY(job->data().isEmpty());
}

void HelperTest::testExecuteJobSignals()
{
    KAuth::Action action(QLatin1String("org.kde.kf6auth.autotest.longaction"));
    action.setHelperId(QLatin1String("org.kde.kf6auth.autotest"));
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);

    KAuth::ExecuteJob *job = action.execute();

    QSignalSpy finishedSpy(job, &KJob::result);
    QSignalSpy newDataSpy(job, &KAuth::ExecuteJob::newData);
    QSignalSpy percentSpy(job, &KJob::percentChanged);
    QSignalSpy statusChangedSpy(job, &KAuth::ExecuteJob::statusChanged);

    QVERIFY(job->exec());

    QCOMPARE(finishedSpy.size(), 1);
    QCOMPARE(qobject_cast<KAuth::ExecuteJob *>(finishedSpy.first().first().value<KJob *>()), job);
    QCOMPARE(statusChangedSpy.size(), 1);
    QCOMPARE(statusChangedSpy.first().first().value<KAuth::Action::AuthStatus>(), KAuth::Action::AuthorizedStatus);
    QCOMPARE(percentSpy.size(), 100);
    for (ulong i = 1; i <= 100; ++i) {
        QCOMPARE((unsigned long)percentSpy.at(i - 1).last().toLongLong(), i);
        QCOMPARE(qobject_cast<KAuth::ExecuteJob *>(percentSpy.at(i - 1).first().value<KJob *>()), job);
    }
    QCOMPARE(newDataSpy.size(), 1);
    QCOMPARE(newDataSpy.first().first().value<QVariantMap>().value(QLatin1String("Answer")).toInt(), 42);

    QVERIFY(!job->error());
    QVERIFY(job->data().isEmpty());
}

void HelperTest::testTwoCalls()
{
    KAuth::Action action(QLatin1String("org.kde.kf6auth.autotest.standardaction"));
    action.setHelperId(QLatin1String("org.kde.kf6auth.autotest"));
    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    KAuth::ExecuteJob *job = action.execute();
    QVERIFY(job->exec());

    job = action.execute();
    QVERIFY(job->exec());
}

void HelperTest::testActionData()
{
    KAuth::Action action(QLatin1String("org.kde.kf6auth.autotest.echoaction"));
    action.setHelperId(QLatin1String("org.kde.kf6auth.autotest"));

    QVariantMap args;
    // Fill with random data (and test heavy structures while we're at it)
    auto *generator = QRandomGenerator::global();
    for (int i = 0; i < 150; ++i) {
        args.insert(QUuid::createUuid().toString(), generator->generate());
    }
    action.setArguments(args);

    QVERIFY(action.isValid());

    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    KAuth::ExecuteJob *job = action.execute();

    QVERIFY(job->exec());

    QVERIFY(!job->error());
    QCOMPARE(job->data(), args);
}

void HelperTest::testHelperFailure()
{
    KAuth::Action action(QLatin1String("org.kde.kf6auth.autotest.failingaction"));
    action.setHelperId(QLatin1String("org.kde.kf6auth.autotest"));
    QVERIFY(action.isValid());
    QCOMPARE(action.status(), KAuth::Action::AuthRequiredStatus);
    KAuth::ExecuteJob *job = action.execute();
    QVERIFY(!job->exec());
    QVERIFY(job->error());
}

QTEST_MAIN(HelperTest)
#include "HelperTest.moc"
