// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "maincontroller/task/taskcommander.h"
#include "maincontroller/task/taskcommander_p.h"
#include "searcher/proxyworker.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QtConcurrent>
#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(TaskCommanderTest, ut_taskID)
{
    TaskCommander tc("");
    EXPECT_FALSE(tc.taskID().isEmpty());
}

TEST(TaskCommanderTest, ut_content)
{
    TaskCommander tc("");
    EXPECT_TRUE(tc.content().isEmpty());

    const QString content = "test";
    TaskCommander tc2(content);
    EXPECT_EQ(tc2.content(), content);
}

TEST(TaskCommanderTest, ut_getResults)
{
    TaskCommander tc("");
    EXPECT_TRUE(tc.getResults().isEmpty());

    GrandSearch::MatchedItemMap rets;
    rets.insert("test", {GrandSearch::MatchedItem()});

    tc.d->m_results = rets;

    EXPECT_EQ(tc.getResults(), rets);
}

TEST(TaskCommanderTest, ut_readBuffer)
{
    TaskCommander tc("");
    EXPECT_TRUE(tc.readBuffer().isEmpty());

    GrandSearch::MatchedItemMap rets;
    rets.insert("test", {GrandSearch::MatchedItem()});

    tc.d->m_buffer = rets;

    EXPECT_EQ(tc.readBuffer(), rets);
    EXPECT_TRUE(tc.d->m_buffer.isEmpty());
}

TEST(TaskCommanderTest, ut_isEmptyBuffer)
{
    TaskCommander tc("");
    EXPECT_TRUE(tc.isEmptyBuffer());

    GrandSearch::MatchedItemMap rets;
    rets.insert("test", {GrandSearch::MatchedItem()});

    tc.d->m_buffer = rets;

    EXPECT_FALSE(tc.isEmptyBuffer());
}

namespace  {
class TestWorker : public ProxyWorker
{
public:
    using ProxyWorker::ProxyWorker;
    void setContext(const QString &context) {ctx = context;}
    bool isAsync() const {return async;}
    bool working(void *context) {work = true;return true;}
    void terminate() {work = false;}
    Status status() {return Ready;}
    bool hasItem() const {return item;}
    GrandSearch::MatchedItemMap takeAll() {return items;}
public:
    bool work = false;
    bool async = false;
    bool item = false;
    GrandSearch::MatchedItemMap items;
    QString ctx = "";
};
}

TEST(TaskCommanderTest, ut_join)
{
    TaskCommander tc("test");
    tc.d->m_working = true;
    EXPECT_FALSE(tc.join(nullptr));
    EXPECT_TRUE(tc.d->m_allWorkers.isEmpty());

    auto worker = new TestWorker("test1");
    auto worker2 = new TestWorker("test2");
    {
        TaskCommanderPrivate *tcp = tc.d;
        tcp->m_working = false;

        EXPECT_TRUE(tc.join(worker));
        EXPECT_EQ(worker->parent(), tcp);
        EXPECT_EQ(worker->ctx, tcp->m_content);
        EXPECT_EQ(tcp->m_allWorkers.size(), 1);
        EXPECT_EQ(tcp->m_asyncWorkers.size(), 0);
        EXPECT_EQ(tcp->m_syncWorkers.size(), 1);

        stub_ext::StubExt stub;
        bool unearthed = false;
        stub.set_lamda(&TaskCommanderPrivate::onUnearthed, [&unearthed](){unearthed = true;});

        //测试直连
        emit worker->unearthed(worker);
        EXPECT_TRUE(unearthed);

        worker2->async = true;
        EXPECT_TRUE(tc.join(worker2));
        EXPECT_EQ(tcp->m_allWorkers.size(), 2);
        EXPECT_EQ(tcp->m_asyncWorkers.size(), 1);
        EXPECT_EQ(tcp->m_syncWorkers.size(), 1);
    }

    //TaskCommander释放worker
    EXPECT_NO_FATAL_FAILURE(worker);
    EXPECT_NO_FATAL_FAILURE(worker2);
}

TEST(TaskCommanderTest, ut_deleteSelf)
{
    TaskCommander *tc = new TaskCommander("test");
    ASSERT_TRUE(tc->d->m_asyncLine.isFinished());
    ASSERT_TRUE(tc->d->m_syncLine.isFinished());
    ASSERT_FALSE(tc->d->m_deleted);

    //释放自己，无内存泄漏
    EXPECT_NO_FATAL_FAILURE(tc->deleteSelf());
}

TEST(TaskCommanderTest, ut_deleteSelf_1)
{
    TaskCommander *tc = new TaskCommander("test");
    bool exit = false;
    tc->d->m_asyncLine.setFuture(QtConcurrent::run([&exit]() { while (!exit) {
            QThread::msleep(1);
        }}));
    ASSERT_FALSE(tc->d->m_asyncLine.isFinished());
    ASSERT_TRUE(tc->d->m_syncLine.isFinished());
    ASSERT_FALSE(tc->d->m_deleted);

    tc->deleteSelf();
    exit = true;
    ASSERT_TRUE(tc->d->m_deleted);
    tc->d->m_asyncLine.waitForFinished();
    EXPECT_NO_FATAL_FAILURE(delete tc);
}

TEST(TaskCommanderTest, ut_isFinished)
{
    TaskCommander tc("test");
    tc.d->m_finished = false;
    EXPECT_FALSE(tc.isFinished());

    tc.d->m_finished = true;
    EXPECT_TRUE(tc.isFinished());
}

TEST(TaskCommanderTest, ut_stop)
{
    TaskCommander tc("test");
    tc.d->m_working = true;
    tc.d->m_finished = false;

    auto worker = new TestWorker("test1");
    worker->work = true;
    tc.d->m_allWorkers.append(worker);

    tc.stop();
    EXPECT_FALSE(tc.d->m_working);
    EXPECT_TRUE(tc.d->m_finished);
    EXPECT_FALSE(worker->work);

    tc.d->m_allWorkers.clear();
    EXPECT_NO_FATAL_FAILURE(delete worker);
}

TEST(TaskCommanderTest, ut_start)
{
    TaskCommander tc("test");
    tc.d->m_working = true;

    ASSERT_FALSE(tc.start());

    tc.d->m_working = false;
    bool finished = false;
    QObject::connect(&tc, &TaskCommander::finished, &tc, [&finished](){finished = true;});

    //空任务
    ASSERT_TRUE(tc.d->m_asyncWorkers.isEmpty());
    ASSERT_TRUE(tc.d->m_syncWorkers.isEmpty());

    EXPECT_TRUE(tc.start());
    EXPECT_FALSE(tc.d->m_working);
    EXPECT_FALSE(finished);

    //测试队列
    QTest::qWaitFor([&finished](){return finished;}, 500);

    EXPECT_TRUE(finished);
}

TEST(TaskCommanderTest, ut_start_1)
{
    stub_ext::StubExt stub;
    bool finished = false;
    stub.set_lamda(&TaskCommanderPrivate::onWorkFinished, [&finished](){finished = true;});
    bool lineFinished = false;
    stub.set_lamda(&TaskCommanderPrivate::onFinished, [&lineFinished](){lineFinished = true;});

    TaskCommander tc("test");
    auto worker = new TestWorker("test1");
    worker->async = true;
    tc.d->m_asyncWorkers.append(worker);

    ASSERT_TRUE(tc.start());
    EXPECT_TRUE(tc.d->m_working);

    QTest::qWaitFor([&lineFinished](){return lineFinished;}, 500);
    EXPECT_FALSE(finished);
    EXPECT_TRUE(worker->work);
    EXPECT_EQ(tc.d->m_workingWorkers.size(), 1);
    EXPECT_TRUE(lineFinished);

    // 结束信号
    emit worker->asyncFinished(worker);
    //队列
    EXPECT_FALSE(finished);
    QTest::qWaitFor([&finished](){return finished;}, 500);
    EXPECT_TRUE(finished);

    EXPECT_NO_FATAL_FAILURE(delete worker);
}

TEST(TaskCommanderPrivate, ut_onUnearthed)
{
    TaskCommander tc("test");
    auto worker = new TestWorker("test1");
    worker->item = true;
    worker->items.insert("test", {MatchedItem()});
    tc.d->m_allWorkers.append(worker);

    bool matched = false;
    QObject::connect(&tc, &TaskCommander::matched, &tc, [&matched](){matched = true;});

    tc.d->onUnearthed(worker);

    EXPECT_EQ(tc.d->m_results, worker->items);
    EXPECT_EQ(tc.d->m_buffer, worker->items);

    //测试队列信号
    EXPECT_FALSE(matched);
    QTest::qWaitFor([&matched](){return matched;}, 500);
    EXPECT_TRUE(matched);

    EXPECT_NO_FATAL_FAILURE(delete worker);
}

TEST(TaskCommanderPrivate, ut_onFinished)
{
    TaskCommander *tc = new TaskCommander("test");
    bool finished = false;
    QObject::connect(tc, &TaskCommander::finished, tc, [&finished](){finished = true;}, Qt::DirectConnection);
    tc->d->m_deleted = false;
    tc->d->m_finished = true;

    tc->d->onFinished();
    EXPECT_TRUE(tc->d->m_finished);
    EXPECT_FALSE(finished);

    tc->d->m_finished = false;
    tc->d->onFinished();
    EXPECT_TRUE(tc->d->m_finished);
    EXPECT_TRUE(finished);

    tc->d->m_finished = false;
    finished = false;
    tc->d->m_workingWorkers.append(reinterpret_cast<ProxyWorker *>(1));
    tc->d->onFinished();
    EXPECT_FALSE(tc->d->m_finished);
    EXPECT_FALSE(finished);

    EXPECT_NO_FATAL_FAILURE(delete tc);
}

TEST(TaskCommanderPrivate, ut_onFinished_1)
{
    TaskCommander *tc = new TaskCommander("test");
    tc->d->m_deleted = true;
    bool deleted = false;

    tc->d->onFinished();
    QObject::connect(tc, &TaskCommander::destroyed, tc, [&deleted](){deleted = true;}, Qt::DirectConnection);
    ASSERT_FALSE(deleted);
    QTest::qWaitFor([&deleted](){return deleted;}, 500);
    EXPECT_TRUE(deleted);

    //TaskCommander应该被释放，无需在此处释放
    EXPECT_NO_FATAL_FAILURE(tc);
}
