// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QTest>
#include <QTimer>
#include <gtest/gtest.h>
#include "dasync.h"
#include "dtimedloop.h"

#include "ut_dutil.h"

DCORE_USE_NAMESPACE
#if 0
// 为了方便托管 std::thread 而创建的辅助类
class Thread : public QObject {
    std::thread *m_thread = nullptr;
public:
    template<typename FUNC>
    Thread(FUNC &&func, QObject *parent = nullptr)
        : QObject  (parent)
        , m_thread (new std::thread(func))
    {
    }
    void detach() {
        m_thread->detach();
    }
    void join() {
        m_thread->join();
    }

    virtual ~Thread()
    {
        if (m_thread) {
            delete m_thread;
            m_thread = nullptr;
        }
    }
};

bool gInSubFlag = true;

// 全局内存托管，防止 asan 报错
QObject gRoot;
template <typename FUNC>
void DetachedRun(FUNC &&func) {
    Thread *thread = new Thread(func, &gRoot);
    if (!gInSubFlag) {
        func();
    }
    thread->detach();
}

class ut_DAsync : public testing::Test, public QObject
{
public:
    class Test : public QObject {
    public:
        Test(int in, QObject *parent = nullptr)
            : QObject   (parent)
            , count     (in)
        {
        }
        int count = 0;
    };

    ut_DAsync() { }
    virtual ~ut_DAsync() {}

    virtual void SetUp() {
        task1 = new DAsync<int, int>(this);
        // 测试 task2 在线程内部new能正常工作
        task3 = new DAsync<int, QString>(this);
        // task4~task7 测试固定的API，功能大同小异，在函数内部创建
        task8 = new DAsync<void, QString>(this);
        task9 = new DAsync<void, void>(this);
        task10 = new DAsync<int, void>(this);

        m_loop = new DTimedLoop(this);
        m_loop->setTimeDump(true);
    }

    virtual void TearDown() {
        // 释放资源要用 deleteLater 或者托管内存
        // 避免线程不同步时直接 delete 导致 asan 偶发性报使用释放掉的堆内存
    }
    // 首先要保证这些不同类型的模板参数的声明没有编译问题
    DAsync<int ,int>            *task1 = nullptr;
    DAsync<int ,int>            *task2 = nullptr;
    DAsync<int, QString>        *task3 = nullptr;
    DAsync<QString, QString>    *task4 = nullptr;
    DAsync<Test *, Test*>       *task5 = nullptr;
    DAsync<QString, void>       *task6 = nullptr;
    // 第一个模板参数是 void 的类型的仅执行一次函数调用
    DAsync<void, void>          *task7 = nullptr;
    DAsync<void, QString>       *task8 = nullptr;
    DAsync<void, void>          *task9 = nullptr;
    DAsync<int, void>           *task10 = nullptr;

    // m_loop 须是 static 的，asan 会有误报
    static DTimedLoop           *m_loop;
};

DTimedLoop *ut_DAsync::m_loop = nullptr;

TEST_F(ut_DAsync, testRunInCorrectThread)
{
    // 测试 post 中的函数一定在非主线程异步调用
    // 返回结果传到 then 中的函数在主线程中调用
    task1->post([](int arg) {

        HAVE_FUN(ASSERT_TRUE(!D_THREAD_IN_MAIN()));
        return arg;

    })->then([&](int arg) {

        ASSERT_EQ(arg, 1);
        HAVE_FUN(ASSERT_TRUE(D_THREAD_IN_MAIN()));
        m_loop->exit();

    })->start();

    task1->postData(1);

    m_loop->exec("testRunInCorrectThread");
}

TEST_F(ut_DAsync, testRunInSubThread)
{
    // 和上面 testRunInCorrectThread 测项类似
    // task2, 测试 task 在非主线程中依然能正确创建和运行
    bool startedFlag = false;
    DetachedRun([&]{
        // 这里用托管也可以的，但是会有警告
        task2 = new DAsync<int, int>(/*this*/);
        task2->post([](int arg) {

            HAVE_FUN(ASSERT_TRUE(!D_THREAD_IN_MAIN()));
            return arg;

        })->then([&](int arg) {

            static int i = 0;
            ASSERT_EQ(arg, i++);
            HAVE_FUN(ASSERT_TRUE(D_THREAD_IN_MAIN()));
            if (i > 3) m_loop->exit();

        })->start();

        startedFlag = true;
    });

    DetachedRun([&]{
        static int i = 0;
        while (true) {
            // 要自己设置 flag，因为在不同的线程中，
            // 到这里 task2 还不一定已经被创建完毕
            if (startedFlag) {
                task2->postData(i++);
                if (i > 3) {
                    break;
                }
            }
            usleep(100*1000);
        }
    });

    m_loop->exec("testRunInSubThread");
}

TEST_F(ut_DAsync, testMultiThreadSynchronization)
{
    // task3, 在子线程中输入 0~999, 在 post 中乘以 2 输出到主线程 then 中
    static int n = 1000;
    static int result = 0;

    task3->post([](int arg) -> QString {

        return QString("%1").arg(arg * 2);

    })->then([](QString arg) {

        static int i = 0;
        ASSERT_TRUE(arg == QString("%1").arg(i * 2));
        i++;
        result = n;

    })->start();

    DetachedRun([&] {
        int i = 0;
        while (i < n) {
            if (!task3->isFinished()) {
                task3->postData(i++);
            }
        }
        task3->cancelAll();
    });

    DetachedRun([&] {
        // 该线程启动后会一直阻塞等待，直到 cancelAll 被调用，
        // 说明任务结束了，就可以往下走，判断执行结果
        task3->waitForFinished(false);
        ASSERT_EQ(result, n);
        m_loop->exit();
    });

    m_loop->exec("testMultiThreadSynchronization");
}

TEST_F(ut_DAsync, testOneTimeTask)
{
    // task8, 测试一次性任务，确保两个函数只会进来执行一次
    task8->post([] {

        static int i = 0;
        return QString("testOneTimeTask%1").arg(i++);

    })->then([&](const QString &arg) {

        ASSERT_TRUE(arg == "testOneTimeTask0");
        m_loop->exit();

    })->start();
    m_loop->exec("test task8");

    // task9, 测试一次性任务，确保只有 post 的函数能够被执行到
    task9->post([&]{
        m_loop->exit();
    })->start();
    // task9->startUp();  # 或者在合适的时候调用
    m_loop->exec("test task9");

    // task10, 测试仅有 post 的任务的正确执行
    task10->post([&] (int arg) {
        static int j = 0;
        ASSERT_EQ(arg, j++);
        if (j == 2) {
            m_loop->exit();
        }
    });
    task10->postData(0);
    task10->startUp();
    task10->postData(1);
    m_loop->exec("test task10");
}

TEST_F(ut_DAsync, testFixedApi)
{
    // 测试这些固定的 API 能够正确处理不同的参数类型
    // task4
    task4 = new DAsync<QString, QString>(this);
    static int i = 0;
    while (i < 100) {
        task4->postData(QString::number(i++));
    }
    task4->post([](const QString &arg) -> QString {

        static int j = 0;
        HAVE_FUN(ASSERT_TRUE(arg == QString::number(j++)));
        return arg;

    })->then([](QString arg) {

        static int k = 0;
        ASSERT_TRUE(arg == QString::number(k++));
        if (k == 100) {
            m_loop->exit();
        }

    })->start();

    m_loop->exec("test task4");

    // 和上面的不一样的地方就是 postData 在 start 前后都调用了
    // task5
    i = 0;
    task5 = new DAsync<Test *, Test*>(this);
    while (i < 50) {
        task5->postData(new Test(i++, this));
    }

    task5->post([](Test *arg) -> Test * {

        static int j = 0;
        HAVE_FUN(ASSERT_TRUE(arg->count == j++));
        return arg;

    })->then([](Test *arg) {

        static int k = 0;
        HAVE_FUN(ASSERT_TRUE(arg->count == k++));
        if (k == 100) {
            m_loop->exit();
        }

    })->start();

    while (i < 100) {
        task5->postData(new Test(i++, this));
    }
    m_loop->exec("test task5");

    // task6
    i = 0;
    task6 = new DAsync<QString, void>(this);
    while (i < 50) {
        task6->postData(QString::number(i++));
    }
    task6->post([](QString arg) {

        static int j = 0;
        HAVE_FUN(ASSERT_TRUE(arg == QString::number(j++)));

    })->then([]() {

        static int k = 0;
        k++;
        if (k == 100) {
            m_loop->exit();
        }

    })->start(false);

    DetachedRun([&]{
        usleep(100 * 1000);

        while (i < 100) {
            task6->postData(QString::number(i++));
        }
        task6->startUp();
    });
    m_loop->exec("test task6");

    // task7
    task7 = new DAsync<void, void>(this);
    task7->post([]() {

        static int j = 0;
        HAVE_FUN(ASSERT_TRUE(0 == j++));

    })->then([]() {

        static int k = 0;
        HAVE_FUN(ASSERT_TRUE(0 == k++));
        m_loop->exit();

    })->start();
    m_loop->exec("test task7");
}
#endif
