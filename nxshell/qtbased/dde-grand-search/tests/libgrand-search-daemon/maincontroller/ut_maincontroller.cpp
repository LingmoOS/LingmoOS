// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "maincontroller/maincontroller.h"
#include "maincontroller/maincontroller_p.h"
#include "configuration/configer.h"
#include "maincontroller/task/taskcommander.h"
#include "searcher/searchergroup_p.h"
#include "global/searchhelper.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>

GRANDSEARCH_USE_NAMESPACE

TEST(MainControllerTest, ut_init_0)
{
    MainController mc;
    ASSERT_NE(mc.d, nullptr);
    ASSERT_EQ(mc.d->m_currentTask, nullptr);
    ASSERT_EQ(mc.d->m_searchers, nullptr);

    stub_ext::StubExt st;
    bool searcherGroup = false;
    st.set_lamda(&SearcherGroup::init, [&searcherGroup]() {
        searcherGroup = true;
        return false;
    });

    EXPECT_FALSE(mc.init());
    EXPECT_TRUE(searcherGroup);
    EXPECT_TRUE(mc.d->m_searchers != nullptr);
}

TEST(MainControllerTest, ut_init_1)
{
    MainController mc;
    stub_ext::StubExt st;
    bool searcherGroup = false;
    st.set_lamda(&SearcherGroup::init, [&searcherGroup]() {
        searcherGroup = true;
        return true;
    });

    bool configer = false;
    st.set_lamda(&Configer::init, [&configer]() {
        configer = true;
        return true;
    });

    EXPECT_TRUE(mc.init());
    EXPECT_TRUE(searcherGroup);
    EXPECT_TRUE(configer);
    EXPECT_TRUE(mc.d->m_searchers != nullptr);
}

TEST(MainControllerTest, ut_init_2)
{
    MainController mc;
    stub_ext::StubExt st;
    bool searcherGroup = false;
    st.set_lamda(&SearcherGroup::init, [&searcherGroup]() {
        searcherGroup = true;
        return true;
    });

    bool configer = false;
    st.set_lamda(&Configer::init, [&configer]() {
        configer = true;
        return false;
    });

    EXPECT_FALSE(mc.init());
    EXPECT_TRUE(searcherGroup);
    EXPECT_TRUE(configer);
    EXPECT_TRUE(mc.d->m_searchers != nullptr);
}

typedef void (MainControllerPrivate::*BuildWorker)(TaskCommander *, const QSet<QString> &);

TEST(MainControllerTest, ut_newSearch_0)
{
    MainController mc;
    EXPECT_FALSE(mc.newSearch(""));

    //启动成功
    stub_ext::StubExt st;
    bool terminate = false;
    st.set_lamda(&MainController::terminate, [&terminate]() {
        terminate = true;
    });

    bool buildWorker = false;
    st.set_lamda((BuildWorker)&MainControllerPrivate::buildWorker, [&buildWorker]() {
        buildWorker = true;
    });

    bool start = false;
    st.set_lamda(&TaskCommander::start, [&start]() {
        start = !start;
        return start;
    });

    EXPECT_TRUE(mc.newSearch("test"));
    EXPECT_TRUE(terminate);
    EXPECT_TRUE(buildWorker);
    EXPECT_TRUE(start);
    EXPECT_NE(mc.d->m_currentTask, nullptr);
    EXPECT_TRUE(mc.d->m_dormancy.isActive());
}

TEST(MainControllerTest, ut_newSearch_1)
{
    MainController mc;
    stub_ext::StubExt st;

    //启动失败

    //销毁上次任务
    TaskCommander *cmd = new TaskCommander("test");
    mc.d->m_currentTask = cmd;

    bool buildWorker = false;
    st.set_lamda((BuildWorker)&MainControllerPrivate::buildWorker, [&buildWorker]() {
        buildWorker = true;
    });

    bool start = true;
    st.set_lamda(&TaskCommander::start, [&start]() {
        start = !start;
        return start;
    });

    EXPECT_FALSE(mc.newSearch("test"));
    EXPECT_TRUE(buildWorker);
    EXPECT_FALSE(start);
    EXPECT_EQ(mc.d->m_currentTask, nullptr);
    EXPECT_FALSE(mc.d->m_dormancy.isActive());

    //newSearch接口不释放cmd，也无内存泄漏
    EXPECT_NO_FATAL_FAILURE(cmd);
}

TEST(MainControllerTest, ut_terminate_0)
{
    MainController mc;
    ASSERT_EQ(mc.d->m_currentTask, nullptr);

    stub_ext::StubExt st;
    bool stoped = false;
    st.set_lamda(&TaskCommander::stop, [&stoped]() {
        stoped = true;
    });

    bool del = false;
    st.set_lamda(&TaskCommander::deleteSelf, [&del]() {
        del = true;
    });

    mc.terminate();

    EXPECT_FALSE(stoped);
    EXPECT_FALSE(del);
    EXPECT_EQ(mc.d->m_currentTask, nullptr);

    stoped = false;
    del = false;

    TaskCommander *cmd = new TaskCommander("test");
    mc.d->m_currentTask = cmd;

    mc.terminate();

    EXPECT_TRUE(stoped);
    EXPECT_TRUE(del);
    EXPECT_EQ(mc.d->m_currentTask, nullptr);

    EXPECT_NO_FATAL_FAILURE(delete cmd);
}

TEST(MainControllerTest, ut_getResults_0)
{
    MainController mc;
    ASSERT_EQ(mc.d->m_currentTask, nullptr);
    EXPECT_TRUE(mc.getResults().isEmpty());

    TaskCommander *cmd = new TaskCommander("test");
    mc.d->m_currentTask = cmd;

    stub_ext::StubExt st;
    GrandSearch::MatchedItemMap rets;
    rets.insert("test", {GrandSearch::MatchedItem()});
    st.set_lamda(&TaskCommander::getResults, [&rets]() {
        return rets;
    });

    auto bytes = mc.getResults();

    EXPECT_FALSE(bytes.isEmpty());
    {
        GrandSearch::MatchedItemMap cmp;
        QDataStream stream(&bytes, QIODevice::ReadOnly);
        stream >> cmp;

        EXPECT_EQ(rets, cmp);
    }

    // 测试内存泄漏，MainController自动释放cmd
    EXPECT_NO_FATAL_FAILURE(cmd);
}

TEST(MainControllerTest, ut_readBuffer_0)
{
    MainController mc;
    ASSERT_EQ(mc.d->m_currentTask, nullptr);
    EXPECT_TRUE(mc.readBuffer().isEmpty());

    TaskCommander *cmd = new TaskCommander("test");
    mc.d->m_currentTask = cmd;

    stub_ext::StubExt st;
    GrandSearch::MatchedItemMap rets;
    rets.insert("test", {GrandSearch::MatchedItem()});
    st.set_lamda(&TaskCommander::readBuffer, [&rets]() {
        return rets;
    });

    auto bytes = mc.readBuffer();

    EXPECT_FALSE(bytes.isEmpty());
    {
        GrandSearch::MatchedItemMap cmp;
        QDataStream stream(&bytes, QIODevice::ReadOnly);
        stream >> cmp;

        EXPECT_EQ(rets, cmp);
    }

    // 测试内存泄漏，MainController自动释放cmd
    EXPECT_NO_FATAL_FAILURE(cmd);
}

TEST(MainControllerTest, ut_isEmptyBuffer_0)
{
    MainController mc;
    ASSERT_EQ(mc.d->m_currentTask, nullptr);
    EXPECT_TRUE(mc.isEmptyBuffer());

    TaskCommander *cmd = new TaskCommander("test");
    mc.d->m_currentTask = cmd;

    stub_ext::StubExt st;
    bool empty = true;
    st.set_lamda(&TaskCommander::isEmptyBuffer, [&empty]() {
        return empty;
    });
    EXPECT_TRUE(mc.isEmptyBuffer());

    empty = false;
    EXPECT_FALSE(mc.isEmptyBuffer());

    // MainController自动释放cmd
    EXPECT_NO_FATAL_FAILURE(cmd);
}

namespace {
class TSearcher : public Searcher
{
public:
    using Searcher::Searcher;
    QString name() const {return "test";}
    bool isActive() const{return true;}
    ProxyWorker *createWorker() const {return nullptr;}
    bool action(const QString &ac, const QString &item) {
        return ac == "open";
    }
};
}

TEST(MainControllerTest, ut_searcherAction_0)
{
    MainController mc;
    EXPECT_FALSE(mc.searcherAction("s","",""));
    EXPECT_FALSE(mc.searcherAction("","s",""));
    EXPECT_FALSE(mc.searcherAction("","","s"));

    auto searchers = new SearcherGroup();
    mc.d->m_searchers = searchers;

    auto tSearcher = new TSearcher();
    searchers->d->m_builtin.append(tSearcher);

    //无eracher
    EXPECT_FALSE(mc.searcherAction("test0","s","s"));

    //操作名错误
    EXPECT_FALSE(mc.searcherAction("test","s","s"));

    //有效
    EXPECT_TRUE(mc.searcherAction("test","open","s"));

    EXPECT_NO_FATAL_FAILURE(delete searchers);
    EXPECT_NO_FATAL_FAILURE(delete tSearcher);
}

TEST(MainControllerTestPrivate, ut_buildWorker_0)
{
    class T1Searcher : public TSearcher
    {
    public:
        bool isActive() const{return actived;}
        bool activate(){return active;}
        ProxyWorker *createWorker() const {return reinterpret_cast<ProxyWorker *>(worker);}
    public:
        bool actived = false;
        bool active = false;
        int worker = 0;
    };

    MainController mc;
    auto searchers = new SearcherGroup();
    mc.d->m_searchers = searchers;

    auto tSearcher = new T1Searcher();
    searchers->d->m_builtin.append(tSearcher);

    stub_ext::StubExt st;
    //空配置，测试默认值
    UserPreferencePointer upp(new UserPreference(QVariantHash()));
    st.set_lamda(&Configer::group, [&upp](Configer *, const QString &name) {
        if (name == GRANDSEARCH_PREF_SEARCHERENABLED)
            return upp;

        return UserPreferencePointer();
    });

    long long join = 0;
    bool joined = false;
    st.set_lamda(&TaskCommander::join, [&joined, &join](TaskCommander *, ProxyWorker *worker) {
        join = (long long)worker;
        joined = true;
        return true;
    });
    TaskCommander *cmd = new TaskCommander("test");

    // 已激活
    tSearcher->actived = true;

    // 已激活，无效worker
    mc.d->buildWorker(cmd);
    EXPECT_FALSE(joined);
    EXPECT_EQ(join, 0);

    // 已激活，有效woker
    tSearcher->worker = 1;
    joined = false;
    join = 0;

    mc.d->buildWorker(cmd);
    EXPECT_TRUE(joined);
    EXPECT_EQ(join, 1);

    // 未激活，激活成功，有效worker
    tSearcher->actived = false;
    tSearcher->active = true;
    tSearcher->worker = 2;
    joined = false;
    join = 0;

    mc.d->buildWorker(cmd);
    EXPECT_TRUE(joined);
    EXPECT_EQ(join, 2);

    // 未激活，激活失败，有效worker
    tSearcher->actived = false;
    tSearcher->active = false;
    tSearcher->worker = 3;
    joined = false;
    join = 0;

    mc.d->buildWorker(cmd);
    EXPECT_FALSE(joined);
    EXPECT_EQ(join, 0);

    // 激活成功，有效worker
    tSearcher->active = true;
    tSearcher->worker = 4;
    // 配置启用
    upp->setValue("test", true);
    joined = false;
    join = 0;

    mc.d->buildWorker(cmd);
    EXPECT_TRUE(joined);
    EXPECT_EQ(join, 4);

    // 配置禁用
    upp->setValue("test", false);
    joined = false;
    join = 0;

    mc.d->buildWorker(cmd);
    EXPECT_FALSE(joined);
    EXPECT_EQ(join, 0);

    EXPECT_NO_FATAL_FAILURE(delete searchers);
    EXPECT_NO_FATAL_FAILURE(delete tSearcher);
    EXPECT_NO_FATAL_FAILURE(delete cmd);
}

TEST(MainControllerTestPrivate, ut_dormancy_0)
{
    MainController mc;
    QTimer *t = &mc.d->m_dormancy;

    EXPECT_TRUE(t->isSingleShot());
    EXPECT_GT(t->interval(), 60 * 1000);

    stub_ext::StubExt st;
    bool dormancy = false;
    st.set_lamda(&MainControllerPrivate::dormancy, [&dormancy]() {
        dormancy = true;
        return ;
    });

    t->setInterval(2);
    t->start();

    auto ret = QTest::qWaitFor([&dormancy](){return dormancy;}, 100);
    EXPECT_TRUE(dormancy);
}

TEST(MainControllerTestPrivate, ut_dormancy_1)
{
    MainController mc;
    mc.d->dormancy();
    EXPECT_FALSE(mc.d->m_dormancy.isActive());

    mc.d->m_dormancy.stop();
    TaskCommander *cmd = new TaskCommander("test");
    mc.d->m_currentTask = cmd;

    stub_ext::StubExt st;
    bool finished = false;
    st.set_lamda(&TaskCommander::isFinished, [&finished]() {
        finished = !finished;
        return finished;
    });

    mc.d->dormancy();
    EXPECT_FALSE(mc.d->m_dormancy.isActive());
    EXPECT_TRUE(finished);

    mc.d->dormancy();
    EXPECT_TRUE(mc.d->m_dormancy.isActive());
    EXPECT_FALSE(finished);
}

TEST(MainControllerTestPrivate, ut_dormancy_2)
{
    MainController mc;

    auto searchers = new SearcherGroup();
    mc.d->m_searchers = searchers;

    stub_ext::StubExt st;
    bool dormancy = false;
    st.set_lamda(&SearcherGroup::dormancy, [&dormancy]() {
        dormancy = true;
        return ;
    });

    mc.d->dormancy();
    EXPECT_TRUE(dormancy);

    EXPECT_NO_FATAL_FAILURE(delete searchers);
}

TEST(MainControllerTestPrivate, ut_buildKeywordInJson_0)
{
    MainController mc;
    QStringList groupList;
    QStringList suffixList;
    QStringList keywordList;
    QJsonDocument doc;
    QJsonObject obj;
    QJsonArray groupArr, suffixArr, keywordArr;

    groupList.append("a");
    groupList.append("b");
    suffixList.append("aa");
    suffixList.append("bb");
    keywordList.append("");
    keywordList.append("aaa");
    keywordList.append("bbb");

    QString result = mc.d->buildKeywordInJson(groupList, suffixList, keywordList);
    QString resultTest = "{\"Group\":[\"a\",\"b\"],\"Keyword\":[\"aaa\",\"bbb\"],\"Suffix\":[\"aa\",\"bb\"]}";
    EXPECT_EQ(result, resultTest);
}

TEST(MainControllerTestPrivate, ut_checkSearcher_0)
{
    MainController mc;
    QStringList groupList;
    QStringList suffixList;
    QStringList keywordList;

    // searchHelper->getSearcherByGroupName打桩
    stub_ext::StubExt stu;
    stu.set_lamda(&SearchHelper::getSearcherByGroupName, []()->QStringList{
        return {};
    });
    // 类目, 后缀不为空
    groupList.append("text");
    groupList.append("aaa");
    suffixList.append("pdf");
    QStringList result = mc.d->checkSearcher(groupList, suffixList, keywordList);
    QStringList resultTest;
    resultTest.append("com.deepin.dde-grand-search.file-deepin");
    resultTest.append("com.deepin.dde-grand-search.file-fsearch");
    EXPECT_EQ(result, resultTest);

    // 清空groupList, suffixList
    groupList.clear();
    suffixList.clear();
    resultTest.clear();

    // 类目, 后缀为空
    keywordList.append("a");
    keywordList.append("b");
    result = mc.d->checkSearcher(groupList, suffixList, keywordList);
    resultTest.append("com.deepin.dde-grand-search.file-deepin");
    resultTest.append("com.deepin.dde-grand-search.file-fsearch");
    resultTest.append("com.deepin.dde-grand-search.app-desktop");
    EXPECT_EQ(result, resultTest);
}


