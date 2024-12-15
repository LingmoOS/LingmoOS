// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QObject>
#include <gtest/gtest.h>
#include <QTest>
#include <QtConcurrent>
#include <QSignalSpy>
#include <DThreadUtils>
#include <string>
#include <thread>
#include <chrono>

DCORE_USE_NAMESPACE

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)

class ThreadUtils : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void testCallInMainThread();
};

void ThreadUtils::testCallInMainThread()
{
    DThreadUtil::runInMainThread([]() {
        bool result = QThread::currentThread() == QCoreApplication::instance()->thread();
        ASSERT_TRUE(result);
    });

    auto fe = QtConcurrent::run([] {
        ASSERT_TRUE(DThreadUtil::runInMainThread([](QThread *thread) -> bool {
            return QThread::currentThread() == QCoreApplication::instance()->thread() && QThread::currentThread() != thread;
        }, QThread::currentThread()));
    });

    ASSERT_TRUE(QTest::qWaitFor([&] {
        return fe.isFinished();
    }));
}

class ut_DThreadUtils : public testing::Test
{
public:
    virtual void SetUp()
    {
        m_threadutil = new ThreadUtils();
    }
    virtual void TearDown()
    {
        delete m_threadutil;
    }

protected:
    ThreadUtils *m_threadutil = nullptr;
};

TEST_F(ut_DThreadUtils, CallInMainThread)
{
    ASSERT_TRUE(m_threadutil);
    m_threadutil->testCallInMainThread();
}

#else

class ut_DThreadUtils :public testing::Test{
public:
    virtual void SetUp() 
    {
        t = new QThread();
        t->start();
        m_threadutil = new DThreadUtils(t);
    }

    virtual void TearDown()
    {
        t->exit();
        t->wait();
        delete t;
        delete m_threadutil;
    }

protected:
    QThread *t{nullptr};
    DThreadUtils *m_threadutil{nullptr};
};

class QWorker : public QObject {
    Q_OBJECT
public:
    explicit QWorker(QObject *parent = nullptr):QObject(parent){}
    ~QWorker() = default;
public Q_SLOTS:
    int testFunc(int i, double j) {
        int r =qFloor(i + j);
        wait();
        emit testFuncTrigger(r);
        return r;
    }

    void setWaitForSecs(int sec){
        m_waitSecs = sec;
    }
    void wait() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait_for(lock,  std::chrono::seconds(m_waitSecs));
    }

    void notifyOne() {
        m_cv.notify_one();
    }

Q_SIGNALS:
    void testFuncTrigger(int v);
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    int m_waitSecs = 2;
};

class CallableObject{

public:
    CallableObject() = default;
    ~CallableObject() = default;

    QString operator()(const QString &str){
        s += str;
        std::this_thread::sleep_for(std::chrono::microseconds(100));
        return s;
    }

    QString testFunc(const QString &){
        return s;
    }

private:
    QString s{"CallableObject: "};
};

TEST_F(ut_DThreadUtils, testThread)
{
    auto tmp = m_threadutil->thread();
    EXPECT_EQ(tmp, t);
}

TEST_F(ut_DThreadUtils, testRunWithQObj)
{
    QWorker w;
    QSignalSpy spy(&w, &QWorker::testFuncTrigger);
    auto result = m_threadutil->run(&w, &QWorker::testFunc, 10, 24.6);
    std::this_thread::sleep_for(std::chrono::microseconds(100));
    EXPECT_TRUE(result.isStarted());
    EXPECT_TRUE(result.isRunning());
    w.notifyOne();
    result.waitForFinished();
    EXPECT_TRUE(result.isFinished());
    auto raw = result.result();
    EXPECT_EQ(raw, 34);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DThreadUtils,testRunWithLambda)
{
    auto threadId1 = std::this_thread::get_id();
    auto result = m_threadutil->run([](decltype(threadId1) id){
        EXPECT_NE(std::this_thread::get_id(), id);
        return true;
    }, threadId1);
    result.waitForFinished();
    auto raw = result.result();
    EXPECT_TRUE(raw);
}

TEST_F(ut_DThreadUtils,testRunWithCallableObj)
{
    CallableObject obj;
    QString tmp{"Hello"};
    auto result1 = m_threadutil->run(obj, tmp);
    result1.waitForFinished();
    auto raw1 = result1.result();
    EXPECT_EQ(raw1, QString{"CallableObject: Hello"});

    auto result2 = m_threadutil->run(&obj, &CallableObject::testFunc, tmp);
    result2.waitForFinished();
    auto raw2 = result2.result();
    EXPECT_EQ(raw2, QString{"CallableObject: "});
}

TEST_F(ut_DThreadUtils, testExecWithQObj)
{
    QWorker w;
    w.setWaitForSecs(0);
    QSignalSpy spy(&w, SIGNAL(testFuncTrigger(int)));
    auto result = m_threadutil->exec(&w, &QWorker::testFunc, 10, 24.6);
    EXPECT_EQ(result, 34);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DThreadUtils, testExecWithLambda)
{
    auto threadId1 = std::this_thread::get_id();
    auto result = m_threadutil->exec (
        [](decltype(threadId1) id) {
            EXPECT_NE(std::this_thread::get_id(), id);
            return true;
        }, threadId1);
    EXPECT_TRUE(result);
}

TEST_F(ut_DThreadUtils, testExecWithCallableObj)
{
    CallableObject obj;
    QString tmp{"Hello"};
    auto result1 = m_threadutil->exec(obj, tmp);
    EXPECT_EQ(result1, QString{"CallableObject: Hello"});

    auto result2 = m_threadutil->exec(&obj, &CallableObject::testFunc, tmp);
    EXPECT_EQ(result2, QString{"CallableObject: "});
}

TEST_F(ut_DThreadUtils, testDirectlyInvoke)
{
    DThreadUtils tu(QThread::currentThread());
    QWorker w;
    w.setWaitForSecs(0);
    QSignalSpy spy(&w, SIGNAL(testFuncTrigger(int)));
    auto result = tu.run(&w, &QWorker::testFunc, 10, 24.6);
    auto raw = result.result();  // no wait
    EXPECT_EQ(raw, 34);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(ut_DThreadUtils, testCancel)
{
    CallableObject obj;
    QString tmp{"Hello"};
    int cancelCounter{0};
    auto result1 = m_threadutil->run(obj, tmp);
    auto cancelResult = result1.onCanceled([&cancelCounter]() {
        cancelCounter += 1;
        return QString{"failed"};
    });

    result1.cancel();
    EXPECT_FALSE(result1.isFinished());
    EXPECT_FALSE(result1.isValid());
    EXPECT_TRUE(result1.isCanceled());
    cancelResult.waitForFinished();
    EXPECT_EQ(cancelCounter, 1);
    EXPECT_EQ(cancelResult.result(), QString{"failed"});
}

TEST_F(ut_DThreadUtils, testDestructCancel)
{
    auto w = new QWorker{};
    auto failedCounter{0};
    auto result = m_threadutil->run(w, &QWorker::testFunc, 10, 24.6);
    delete w;
    auto failedResult = result.onFailed([&failedCounter]() {
        failedCounter += 1;
        return -1;
    });
    EXPECT_FALSE(result.isValid());
    failedResult.waitForFinished();
    EXPECT_EQ(failedCounter, 1);
    EXPECT_EQ(failedResult.result(), -1);
}

#endif

#include "ut_dthreadutils.moc"
