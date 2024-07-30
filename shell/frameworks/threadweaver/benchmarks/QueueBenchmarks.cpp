/* -*- C++ -*-
    This file contains a benchmark for job processing in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <numeric>

#include <QCoreApplication>
#include <QList>
#include <QString>
#include <QTest>
#include <QtDebug>

#include <ThreadWeaver/Collection>
#include <ThreadWeaver/Job>
#include <ThreadWeaver/Queueing>
#include <ThreadWeaver/Sequence>
#include <ThreadWeaver/ThreadWeaver>

class AccumulateJob : public ThreadWeaver::Job
{
public:
    explicit AccumulateJob()
        : m_count(0)
        , m_result(0)
    {
    }

    AccumulateJob(const AccumulateJob &a)
        : ThreadWeaver::Job()
        , m_count(a.m_count)
        , m_result(a.m_result)
    {
    }

    void setCount(quint64 count)
    {
        m_count = count;
    }

    quint64 result() const
    {
        return m_result;
    }

    void payload()
    {
        std::vector<quint64> numbers(m_count);
        std::generate(numbers.begin(), numbers.end(), []() -> quint64 {
            static quint64 i = 0;
            return i++;
        });
        m_result = std::accumulate(numbers.begin(), numbers.end(), 0);
    }

protected:
    void run(ThreadWeaver::JobPointer, ThreadWeaver::Thread *) override
    {
        payload();
    }

private:
    quint64 m_count;
    quint64 m_result;
};

class QueueBenchmarksTest : public QObject
{
    Q_OBJECT

public:
    QueueBenchmarksTest();

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void BaselineBenchmark();
    void BaselineBenchmark_data();
    void BaselineAsJobsBenchmark();
    void BaselineAsJobsBenchmark_data();
    void IndividualJobsBenchmark();
    void IndividualJobsBenchmark_data();
    void CollectionsBenchmark();
    void CollectionsBenchmark_data();
    void SequencesBenchmark();
    void SequencesBenchmark_data();

private:
    void defaultBenchmarkData(bool singleThreaded);
};

QueueBenchmarksTest::QueueBenchmarksTest()
{
}

void QueueBenchmarksTest::initTestCase()
{
}

void QueueBenchmarksTest::cleanupTestCase()
{
}

/** @brief BaselineBenchmark simply performs the same operations in a loop.
 *
 * The result amounts to what time the jobs used in the benchmark need to execute without queueing or thread
 * synchronization overhead. */
void QueueBenchmarksTest::BaselineBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c * b;
    Q_UNUSED(t); // in this case

    QList<AccumulateJob> jobs(n);
    for (int i = 0; i < n; ++i) {
        jobs[i].setCount(m);
    }

    // executeLocal needs to emit similar signals as execute(), to be comparable to the threaded variants.
    // BaselineAsJobsBenchmark does that. Compare BaselineAsJobsBenchmark and BaselineBenchmark to evaluate the overhead of executing
    // an operation in a job.
    QBENCHMARK {
        for (int i = 0; i < n; ++i) {
            jobs[i].payload();
        }
    }
}

void QueueBenchmarksTest::BaselineBenchmark_data()
{
    defaultBenchmarkData(true);
}

void QueueBenchmarksTest::BaselineAsJobsBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c * b;
    Q_UNUSED(t); // in this case

    QList<AccumulateJob> jobs(n);
    for (int i = 0; i < n; ++i) {
        jobs[i].setCount(m);
    }

    QBENCHMARK {
        for (int i = 0; i < n; ++i) {
            jobs[i].blockingExecute();
        }
    }
}

void QueueBenchmarksTest::BaselineAsJobsBenchmark_data()
{
    defaultBenchmarkData(true);
}

void QueueBenchmarksTest::IndividualJobsBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c * b;

    ThreadWeaver::Queue weaver;
    weaver.setMaximumNumberOfThreads(t);
    weaver.suspend();
    QList<AccumulateJob> jobs(n);
    {
        ThreadWeaver::QueueStream stream(&weaver);
        for (int i = 0; i < n; ++i) {
            jobs[i].setCount(m);
            stream << jobs[i];
        }
    }
    QBENCHMARK_ONCE {
        weaver.resume();
        weaver.finish();
    }
}

void QueueBenchmarksTest::IndividualJobsBenchmark_data()
{
    defaultBenchmarkData(false);
}

void QueueBenchmarksTest::CollectionsBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c * b;

    ThreadWeaver::Queue weaver;
    weaver.setMaximumNumberOfThreads(t);
    weaver.suspend();
    QList<AccumulateJob> jobs(n);

    // FIXME currently, memory management of the job sequences (they are deleted when they go out of scope)
    // is measured as part of the benchmark
    qDebug() << b << "blocks" << c << "operations, queueing...";
    // queue the jobs blockwise as collections
    for (int block = 0; block < b; ++block) {
        ThreadWeaver::Collection *collection = new ThreadWeaver::Collection();
        for (int operation = 0; operation < c; ++operation) {
            const int index = block * b + operation;
            jobs[index].setCount(m);
            *collection << jobs[index];
        }
        weaver.stream() << collection;
    }

    qDebug() << b << "blocks" << c << "operations, executing...";
    QBENCHMARK_ONCE {
        weaver.resume();
        weaver.finish();
    }
}

void QueueBenchmarksTest::CollectionsBenchmark_data()
{
    defaultBenchmarkData(false);
}

void QueueBenchmarksTest::SequencesBenchmark()
{
    QFETCH(int, m);
    QFETCH(int, c);
    QFETCH(int, b);
    QFETCH(int, t);
    const int n = c * b;

    ThreadWeaver::Queue weaver;
    weaver.setMaximumNumberOfThreads(t);
    weaver.suspend();
    QList<AccumulateJob> jobs(n);

    qDebug() << b << "blocks" << c << "operations, queueing...";
    // queue the jobs blockwise as collections
    for (int block = 0; block < b; ++block) {
        ThreadWeaver::Sequence *sequence = new ThreadWeaver::Sequence();
        for (int operation = 0; operation < c; ++operation) {
            const int index = block * b + operation;
            jobs[index].setCount(m);
            *sequence << jobs[index];
        }
        weaver.stream() << sequence;
    }

    qDebug() << b << "blocks" << c << "operations, executing...";
    QBENCHMARK_ONCE {
        weaver.resume();
        weaver.finish();
    }
}

void QueueBenchmarksTest::SequencesBenchmark_data()
{
    defaultBenchmarkData(false);
}

void QueueBenchmarksTest::defaultBenchmarkData(bool singleThreaded)
{
    QTest::addColumn<int>("m"); // number of quint64's to accumulate
    QTest::addColumn<int>("c"); // operations per block
    QTest::addColumn<int>("b"); // number of blocks, number of jobs is b*c
    QTest::addColumn<int>("t"); // number of worker threads

    const QList<int> threads = singleThreaded ? QList<int>() << 1 : QList<int>() << 1 << 2 << 4 << 8 << 16 << 32 << 64 << 128;
    const QList<int> ms = QList<int>() << 1 << 10 << 100 << 1000 << 10000 << 100000;
    for (int m : ms) {
        for (int t : threads) {
            const QString name = QString::fromLatin1("%1 threads, %2 values").arg(t).arg(m);
            // newRow expects const char*, but then qstrdup's it in the QTestData constructor. Eeeew.
            QTest::newRow(qPrintable(name)) << m << 256 << 256 << t;
        }
    }
}

QTEST_MAIN(QueueBenchmarksTest)

#include "QueueBenchmarks.moc"
