/*
    SPDX-FileCopyrightText: 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMMON_TEST_H
#define COMMON_TEST_H

#include <QCoreApplication>
#include <QDate>
#include <QFuture>
#include <QFutureWatcher>
#include <QObject>
#include <QTest>

class Test : public QObject
{
    Q_OBJECT
public:
    Test(QObject *parent = nullptr);

protected:
    enum WhenToFail {
        DontFail = 0,
        FailIfTrue = 1,
        FailIfFalse = 2,
    };

    template<typename _ReturnType, typename _Continuation>
    void continue_future(const QFuture<_ReturnType> &future, _Continuation &&continuation)
    {
        if (!future.isFinished()) {
            auto watcher = new QFutureWatcher<decltype(future.result())>();
            QObject::connect(
                watcher,
                &QFutureWatcherBase::finished,
                watcher,
                [=] {
                    continuation(watcher->result());
                    watcher->deleteLater();
                },
                Qt::QueuedConnection);

            watcher->setFuture(future);

        } else {
            continuation(future.result());
        }
    }

    template<typename _Continuation>
    void continue_future(const QFuture<void> &future, _Continuation &&continuation)
    {
        if (!future.isFinished()) {
            auto watcher = new QFutureWatcher<void>();
            QObject::connect(
                watcher,
                &QFutureWatcherBase::finished,
                watcher,
                [=] {
                    continuation();
                    watcher->deleteLater();
                },
                Qt::QueuedConnection);

            watcher->setFuture(future);

        } else {
            continuation();
        }
    }

    template<typename T>
    static inline void wait_until(T condition, const char *msg, int msecs = 300)
    {
        auto start = QTime::currentTime();

        while (!condition()) {
            QCoreApplication::processEvents();

            auto now = QTime::currentTime();
            QVERIFY2(start.msecsTo(now) < msecs, msg);
            if (start.msecsTo(now) >= msecs)
                break;
        }
    }

#define TEST_WAIT_UNTIL(C)                                                                                                                                     \
    wait_until(                                                                                                                                                \
        [&]() -> bool {                                                                                                                                        \
            return C;                                                                                                                                          \
        },                                                                                                                                                     \
        "Timeout waiting for: " #C);
#define TEST_WAIT_UNTIL_WITH_TIMEOUT(C, T)                                                                                                                     \
    wait_until(                                                                                                                                                \
        [&]() -> bool {                                                                                                                                        \
            return C;                                                                                                                                          \
        },                                                                                                                                                     \
        "Timeout waiting for: " #C,                                                                                                                            \
        T);

    template<typename T>
    static bool check(T what, WhenToFail wtf = DontFail, const char *msg = nullptr)
    {
        bool result = what();

        if ((wtf == FailIfTrue && result) || (wtf == FailIfFalse && !result)) {
            qFatal(
                "\n"
                "\n"
                "!!! > \n"
                "!!! > %s\n"
                "!!! > \n",
                msg);
        }

        return result;
    }

    static bool inEmptySession();
    static bool isActivityManagerRunning();

Q_SIGNALS:
    void testFinished();
};

#define CHECK_CONDITION(A, B) check(A, B, #A " raised " #B)

// Pretty print
#include <iostream>

#if defined(Q_NO_DEBUG) || !defined(Q_OS_LINUX)
#define TEST_CHUNK(Name)
#else
inline void _test_chunk(const QString &message)
{
    std::cerr << '\n' << message.toStdString() << "\n" << std::string(message.length(), '-') << '\n';
}
#define TEST_CHUNK(Name) _test_chunk(Name);
#endif

#endif /* TEST_H */
