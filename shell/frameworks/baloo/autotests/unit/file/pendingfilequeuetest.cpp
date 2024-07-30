/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2011 Sebastian Trueg <trueg@kde.org>
    SPDX-FileCopyrightText: 2013-2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "pendingfilequeue.h"

#include <QTest>
#include <QSignalSpy>
#include <qtimer.h>

namespace Baloo {

class PendingFileQueueTest : public QObject
{
    Q_OBJECT

public:
    PendingFileQueueTest();

private Q_SLOTS:
    void testTimers();
    void testTimeout();
    void testRequeue();
    void testDeleteCreate();
    void testCreateDelete();
    void testCreateDelete2();
};

PendingFileQueueTest::PendingFileQueueTest()
{
}

class TimerEventEater : public QObject
{
    Q_OBJECT

public:
    TimerEventEater(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject *object, QEvent *event) override {
        Q_UNUSED(object);
        Q_UNUSED(event);
        return true;
    }
};

void PendingFileQueueTest::testTimers()
{
    QString myUrl(QStringLiteral("/tmp"));

    PendingFileQueue queue;
    queue.setMinimumTimeout(1);
    queue.setTrackingTime(1);

    QSignalSpy spy(&queue, SIGNAL(indexModifiedFile(QString)));
    QVERIFY(spy.isValid());

    PendingFile file(myUrl);
    file.setModified();
    queue.enqueue(file);

    // The signal should be emitted immediately
    QVERIFY(spy.wait(50));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().constFirst().toString(), myUrl);
    QCOMPARE(queue.m_recentlyEmitted.count(), 1);

    // Enqueue the url again. This time it should wait for should wait for the
    // minimumTimeout
    queue.enqueue(file);

    QTest::qWait(100);
    QCOMPARE(queue.m_pendingFiles.count(), 1);
    QCOMPARE(queue.m_recentlyEmitted.count(), 1);
    QVERIFY(spy.isEmpty());

    QVERIFY(spy.wait(1500));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().constFirst().toString(), myUrl);
    QCOMPARE(queue.m_pendingFiles.count(), 0);
    QCOMPARE(queue.m_recentlyEmitted.count(), 1);

    // Wait for Tracking Time
    QTest::qWait(1500);
    QCOMPARE(queue.m_recentlyEmitted.count(), 0);
}

void PendingFileQueueTest::testTimeout()
{
    QString file1Url(QStringLiteral("file1_url"));
    QString file2Url(QStringLiteral("file2_url"));

    QTime currentTime = QTime::currentTime();

    PendingFileQueue queue;
    queue.setMinimumTimeout(2);

    auto eventEater = new TimerEventEater(this);
    queue.m_cacheTimer.installEventFilter(eventEater);
    queue.m_clearRecentlyEmittedTimer.installEventFilter(eventEater);
    queue.m_pendingFilesTimer.installEventFilter(eventEater);

    QSignalSpy spy(&queue, SIGNAL(indexModifiedFile(QString)));
    QVERIFY(spy.isValid());

    PendingFile file1(file1Url);
    PendingFile file2(file2Url);
    file1.setModified();
    file2.setModified();

    // Enqueue, and process the event queue
    queue.enqueue(file1);
    QTimer::singleShot(0, [&queue, currentTime] { queue.processCache(currentTime); });

    // The signal should be emitted immediately
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().constFirst().toString(), file1Url);

    // Enqueue file1 again, and also file2. This time, only file2
    // should be signaled immediately
    queue.enqueue(file1);
    queue.enqueue(file2);
    QTimer::singleShot(0, [&queue, currentTime] { queue.processCache(currentTime); });

    QVERIFY(spy.wait(50));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().constFirst().toString(), file2Url);

    // Advance time 1.5 seconds, and let the pending queue be processed.
    // Nothing should be signaled, as the timeout is 2 seconds
    currentTime = currentTime.addMSecs(1500);
    QTimer::singleShot(0, [&queue, currentTime] { queue.processPendingFiles(currentTime); });
    QVERIFY(!spy.wait(50));

    currentTime = currentTime.addMSecs(1000);
    QTimer::singleShot(0, [&queue, currentTime] { queue.processPendingFiles(currentTime); });
    QVERIFY(spy.wait(0));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().constFirst().toString(), file1Url);
}

void PendingFileQueueTest::testRequeue()
{
    QString myUrl(QStringLiteral("/tmp"));

    QTime currentTime = QTime::currentTime();

    PendingFileQueue queue;
    queue.setMinimumTimeout(2);
    queue.setMaximumTimeout(5);

    auto eventEater = new TimerEventEater(this);
    queue.m_cacheTimer.installEventFilter(eventEater);
    queue.m_clearRecentlyEmittedTimer.installEventFilter(eventEater);
    queue.m_pendingFilesTimer.installEventFilter(eventEater);

    QSignalSpy spy(&queue, SIGNAL(indexModifiedFile(QString)));
    QVERIFY(spy.isValid());

    PendingFile file(myUrl);
    file.setModified();
    queue.enqueue(file);
    QTimer::singleShot(0, [&queue, currentTime] { queue.processCache(currentTime); });

    // The signal should be emitted immediately
    QVERIFY(spy.wait());
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().constFirst().toString(), myUrl);

    // Send many events. The first one should enqueue it with the minimumTimeout, and each
    // successive one should double the timeout up to maxTimeout
    for (int i = 0; i < 3; i++) {
        queue.enqueue(file);
        currentTime = currentTime.addMSecs(20);
        QTimer::singleShot(0, [&queue, currentTime] { queue.processCache(currentTime); });
        spy.wait(0);
    }

    // Signal should be emitted after 5 seconds (min(2 * 2 * 2, maxTimeout))
    int elapsed10thSeconds = 0;
    while (true) {
        currentTime = currentTime.addMSecs(100);
        QTimer::singleShot(0, [&queue, currentTime] { queue.processPendingFiles(currentTime); });
        if (spy.wait(0)) {
            break;
        }
        QVERIFY2(elapsed10thSeconds <= 50, "Signal emitted late");
        elapsed10thSeconds++;
    }
    QVERIFY2(elapsed10thSeconds > 40, "Signal emitted early");

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().constFirst().toString(), myUrl);
}

void PendingFileQueueTest::testDeleteCreate()
{
    QString myUrl(QStringLiteral("/dir/testfile"));

    QTime currentTime = QTime::currentTime();

    PendingFileQueue queue;

    auto eventEater = new TimerEventEater(this);
    queue.m_cacheTimer.installEventFilter(eventEater);
    queue.m_clearRecentlyEmittedTimer.installEventFilter(eventEater);
    queue.m_pendingFilesTimer.installEventFilter(eventEater);

    QSignalSpy spyModified(&queue, SIGNAL(indexModifiedFile(QString)));
    QSignalSpy spyRemoved(&queue, SIGNAL(removeFileIndex(QString)));
    QVERIFY(spyModified.isValid());
    QVERIFY(spyRemoved.isValid());

    PendingFile file1(myUrl);
    PendingFile file2(myUrl);
    file1.setDeleted();
    file2.setModified();

    queue.enqueue(file1);
    queue.enqueue(file2);
    QTimer::singleShot(0, [&queue, currentTime] { queue.processCache(currentTime); });

    // The signals should be emitted immediately
    QVERIFY(spyModified.wait());

    QCOMPARE(spyRemoved.count(), 1);
    QCOMPARE(spyRemoved.takeFirst().constFirst().toString(), myUrl);

    QCOMPARE(spyModified.count(), 1);
    QCOMPARE(spyModified.takeFirst().constFirst().toString(), myUrl);
}

void PendingFileQueueTest::testCreateDelete()
{
    QString myUrl(QStringLiteral("/dir/testfile"));

    QTime currentTime = QTime::currentTime();

    PendingFileQueue queue;

    auto eventEater = new TimerEventEater(this);
    queue.m_cacheTimer.installEventFilter(eventEater);
    queue.m_clearRecentlyEmittedTimer.installEventFilter(eventEater);
    queue.m_pendingFilesTimer.installEventFilter(eventEater);

    QSignalSpy spyModified(&queue, SIGNAL(indexModifiedFile(QString)));
    QSignalSpy spyRemoved(&queue, SIGNAL(removeFileIndex(QString)));
    QVERIFY(spyModified.isValid());
    QVERIFY(spyRemoved.isValid());

    // Actually same file, just different events
    PendingFile file_modified(myUrl);
    PendingFile file_delete(myUrl);
    file_modified.setModified();
    file_delete.setDeleted();

    QTimer::singleShot(0, [&] {
        queue.enqueue(file_modified);
        queue.enqueue(file_delete);
    });
    QTimer::singleShot(0, [&queue, currentTime] { queue.processCache(currentTime); });

    // The Removed signal should be emitted immediately
    QVERIFY(spyRemoved.wait());

    // The Modified signal should be suppressed
    QCOMPARE(spyModified.count(), 0);

    QCOMPARE(spyRemoved.count(), 1);
    QCOMPARE(spyRemoved.takeFirst().constFirst().toString(), myUrl);
}

void PendingFileQueueTest::testCreateDelete2()
{
    QString myUrl(QStringLiteral("/dir/testfile"));

    QTime currentTime = QTime::currentTime();

    PendingFileQueue queue;

    auto eventEater = new TimerEventEater(this);
    queue.m_cacheTimer.installEventFilter(eventEater);
    queue.m_clearRecentlyEmittedTimer.installEventFilter(eventEater);
    queue.m_pendingFilesTimer.installEventFilter(eventEater);

    QSignalSpy spyModified(&queue, SIGNAL(indexModifiedFile(QString)));
    QSignalSpy spyRemoved(&queue, SIGNAL(removeFileIndex(QString)));
    QVERIFY(spyModified.isValid());
    QVERIFY(spyRemoved.isValid());

    PendingFile file_modified(myUrl);
    PendingFile file_delete(myUrl);
    file_modified.setModified();
    file_delete.setDeleted();

    // Prime the recent files list
    queue.enqueue(file_modified);
    QTimer::singleShot(0, [&queue, currentTime] { queue.processCache(currentTime); });
    QVERIFY(spyModified.wait());
    QCOMPARE(spyModified.count(), 1);
    QCOMPARE(spyModified.takeFirst().constFirst().toString(), myUrl);
    QCOMPARE(queue.m_recentlyEmitted.count(), 1);

    // Modify the file again
    QTimer::singleShot(0, [&] { queue.enqueue(file_modified); });
    QTimer::singleShot(0, [&queue, currentTime] { queue.processCache(currentTime); });
    // Process the timer, the file should be pending now
    QTest::qWait(10);
    QCOMPARE(queue.m_pendingFiles.count(), 1);

    // Let 5 seconds pass (minimum pending timeout)
    currentTime = currentTime.addMSecs(5000);
    QTimer::singleShot(0, [&] { queue.enqueue(file_delete); });
    // The "process" timer fires 10ms after the enqueue, plenty of time for the pending timer to fire
    QTimer::singleShot(0, [&queue, currentTime] { queue.processPendingFiles(currentTime); });
    currentTime = currentTime.addMSecs(10);
    QTimer::singleShot(0, [&queue, currentTime] { queue.processCache(currentTime); });

    // The Removed signal should be emitted immediately
    QVERIFY(spyRemoved.wait());
    QCOMPARE(spyRemoved.count(), 1);
    QCOMPARE(spyRemoved.takeFirst().constFirst().toString(), myUrl);

    // The Modified signal should be suppressed, the file no longer be pending
    QCOMPARE(spyModified.count(), 0);
    QCOMPARE(queue.m_pendingFiles.count(), 0);
}

} // namespace

QTEST_GUILESS_MAIN(Baloo::PendingFileQueueTest)

#include "pendingfilequeuetest.moc"
