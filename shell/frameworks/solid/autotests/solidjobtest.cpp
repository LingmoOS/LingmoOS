/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include <Solid/Job>

using namespace Solid;

class MockSolidJob : public Solid::Job
{
    Q_OBJECT
public:
    enum Error {
        Foo = Job::UserDefinedError,
        Bar,
    };
    bool execWithError();
    void startWithError();
private Q_SLOTS:
    virtual void doStart();
    void emitQueued()
    {
        emitResult();
    }

private:
    Q_DECLARE_PRIVATE(Job)
};

void MockSolidJob::doStart()
{
    QMetaObject::invokeMethod(this, "emitQueued", Qt::QueuedConnection);
}

bool MockSolidJob::execWithError()
{
    setError(Bar);
    setErrorText(QStringLiteral("Error Bar happened"));
    return exec();
}

void MockSolidJob::startWithError()
{
    setError(Foo);
    setErrorText(QStringLiteral("Error Foo happened"));
    start();
}

class testSolidJob : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAsyncAndResult();
    void testAsyncWithError();
    void testAutoDelete();
    void testSync();
    void testError();
};

void testSolidJob::testAsyncAndResult()
{
    MockSolidJob *job = new MockSolidJob();
    QSignalSpy spy(job, SIGNAL(result(Solid::Job *)));

    job->start();

    QVERIFY(spy.wait());
    // Result is emitted
    QCOMPARE(spy.count(), 1);
    // Result argument is the job that emitted it
    QCOMPARE(spy.takeFirst().first().value<MockSolidJob *>(), job);
}

void testSolidJob::testAsyncWithError()
{
    MockSolidJob *job = new MockSolidJob();
    QSignalSpy spy(job, SIGNAL(result(Solid::Job *)));

    job->startWithError();
    QVERIFY(spy.wait()); // Even on error, we get a result

    MockSolidJob *rJob = spy.takeFirst().first().value<MockSolidJob *>();
    QCOMPARE(rJob->error(), (int)MockSolidJob::Foo);
    QCOMPARE(rJob->errorText(), QStringLiteral("Error Foo happened"));
}

void testSolidJob::testAutoDelete()
{
    MockSolidJob *job = new MockSolidJob();
    QSignalSpy spy(job, SIGNAL(destroyed(QObject *)));
    QSignalSpy spyResult(job, SIGNAL(destroyed(QObject *)));

    job->start();
    QVERIFY(spy.wait());
    QVERIFY(!spyResult.isEmpty()); // Make sure result was emitted as well
}

void testSolidJob::testSync()
{
    MockSolidJob *job = new MockSolidJob();
    QSignalSpy spy(job, SIGNAL(destroyed(QObject *)));

    QVERIFY(job->exec());
    QVERIFY(spy.wait()); // Jobs started with exec should also autodelete
}

void testSolidJob::testError()
{
    MockSolidJob *job = new MockSolidJob();
    QVERIFY(!job->execWithError());

    QCOMPARE(job->error(), (int)MockSolidJob::Bar);
    QCOMPARE(job->errorText(), QStringLiteral("Error Bar happened"));
}

QTEST_MAIN(testSolidJob)

#include "solidjobtest.moc"
