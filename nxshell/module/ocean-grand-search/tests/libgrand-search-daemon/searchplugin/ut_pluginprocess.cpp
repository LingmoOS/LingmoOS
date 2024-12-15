// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "global/grandsearch_global.h"
#include "searchplugin/process/pluginprocess.h"

#include <stubext.h>

#include <gtest/gtest.h>

#include <QTest>
#include <QProcess>

GRANDSEARCH_USE_NAMESPACE

TEST(PluginProcess, ut_addProgram)
{
    PluginProcess pp;
    EXPECT_TRUE(pp.addProgram("test", "/usr/bin/test -t test"));
    ASSERT_EQ(pp.m_processes.size(), 1);
    EXPECT_TRUE(pp.m_programs.contains("test"));

    auto pcs = pp.m_processes.value("test");
    ASSERT_NE(pcs, nullptr);
    EXPECT_EQ(pcs->program(), QString("/usr/bin/test"));
    QStringList args{"-t","test"};
    EXPECT_EQ(pcs->arguments(), args);

    stub_ext::StubExt st;
    bool stachanged = false;
    st.set_lamda(&PluginProcess::processStateChanged, [&stachanged]() {
        stachanged = true;
    });

    EXPECT_FALSE(pp.addProgram("test", "/usr/bin/test2 -t test2"));

    emit pcs->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    //队列
    EXPECT_FALSE(stachanged);

    stachanged = false;
    QTest::qWaitFor([&stachanged](){return stachanged;}, 30);
    EXPECT_TRUE(stachanged);
}

TEST(PluginProcess, ut_setWatched)
{
    PluginProcess pp;
    pp.setWatched("test", true);
    EXPECT_TRUE(pp.m_watch.contains("test"));

    pp.setWatched("test", false);
    EXPECT_FALSE(pp.m_watch.contains("test"));
}

TEST(PluginProcess, ut_clear)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");
    pp.setWatched("test", true);

    EXPECT_FALSE(pp.m_watch.isEmpty());
    EXPECT_FALSE(pp.m_processes.isEmpty());
    EXPECT_FALSE(pp.m_programs.isEmpty());

    pp.clear();
    EXPECT_TRUE(pp.m_watch.isEmpty());
    EXPECT_TRUE(pp.m_processes.isEmpty());
    EXPECT_TRUE(pp.m_programs.isEmpty());
}

TEST(PluginProcess, ut_timerEvent)
{
    QTimerEvent e(-1);
    stub_ext::StubExt st;
    QProcess *p1 = (QProcess *)1;
    st.set_lamda(&PluginProcess::checkStability, [&p1](PluginProcess *, QProcess *p) {
        p1 = p;
    });

    QProcess *p2 = (QProcess *)2;
    st.set_lamda(&PluginProcess::removeChecklist, [&p2](PluginProcess *, QProcess *p) {
        p2 = p;
    });
    PluginProcess pp;
    pp.timerEvent(&e);
    EXPECT_EQ((long)p1, 1);
    EXPECT_EQ((long)p2, 2);
    QProcess pcs;
    pp.m_checklist.insert(&pcs, -1);

    pp.timerEvent(&e);
    EXPECT_EQ(p1, &pcs);
    EXPECT_EQ(p2, &pcs);
}

TEST(PluginProcess, ut_startProgram)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");

    bool started = false;
    stub_ext::StubExt st;
    auto startFunc = (void (QProcess::*)(QProcess::OpenMode))(&QProcess::start);
    st.set_lamda(startFunc, [&started]() {
        started = true;
    });

    for (int i = 0; i < 4; ++i) {
        started = false;
        bool ret = pp.startProgram("test");
        if (i > 2) { //超过三次启动失败
            EXPECT_FALSE(ret);
            EXPECT_FALSE(started);
        } else {
            EXPECT_TRUE(ret);
            EXPECT_TRUE(started);
        }
        //启动次数
        EXPECT_EQ(pp.m_restartCount.value(pp.m_processes.value("test")), i + 1);
    }
}

TEST(PluginProcess, ut_terminate)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");

    stub_ext::StubExt st;
    QProcess::ProcessState sta = QProcess::Running;
    st.set_lamda(&QProcess::state, [&sta]() {
        return sta;
    });

    bool bTerminate = false;
    st.set_lamda(&QProcess::terminate, [&bTerminate]() {
        bTerminate = true;
    });

    bool killed = false;
    bool finished = true;
    int waitMeces = 0;
    st.set_lamda(&QProcess::waitForFinished, [&finished, &waitMeces](QProcess *, int msecs) {
        waitMeces = msecs;
        return finished;
    });

    st.set_lamda(&QProcess::kill, [&killed, &finished]() {
        killed = true;
    });

    QProcess *pcs = nullptr;
    st.set_lamda(&PluginProcess::removeChecklist, [&pcs](PluginProcess *,QProcess *p ) {
        pcs = p;
    });

    pp.m_restartCount.insert(pp.m_processes.value("test"), 1);

    pp.terminate("test2");
    EXPECT_FALSE(bTerminate);
    EXPECT_FALSE(killed);
    EXPECT_EQ(waitMeces, 0);
    EXPECT_EQ(pp.m_restartCount.value(pp.m_processes.value("test")), 1);
    EXPECT_EQ(pcs, nullptr);

    sta = QProcess::NotRunning;
    pcs = nullptr;
    bTerminate = false;
    killed = false;
    waitMeces = 0;
    pp.m_restartCount.insert(pp.m_processes.value("test"), 2);
    pp.terminate("test");
    EXPECT_FALSE(bTerminate);
    EXPECT_FALSE(killed);
    EXPECT_EQ(waitMeces, 0);
    EXPECT_EQ(pp.m_restartCount.value(pp.m_processes.value("test")), 2);
    EXPECT_EQ(pcs, nullptr);

#if defined(Q_PROCESSOR_X86)
    finished = true;
    sta = QProcess::Running;
    pcs = nullptr;
    bTerminate = false;
    killed = false;
    waitMeces = 0;
    pp.m_restartCount.insert(pp.m_processes.value("test"), 3);
    pp.terminate("test");
    EXPECT_TRUE(bTerminate);
    EXPECT_FALSE(killed);
    EXPECT_EQ(waitMeces, 1000);
    EXPECT_TRUE(pp.m_restartCount.isEmpty());
    EXPECT_EQ(pcs, pp.m_processes.value("test"));

    bTerminate = false;
    killed = false;
    waitMeces = 0;
    finished = false;
    pp.terminate("test");
    EXPECT_TRUE(bTerminate);
    EXPECT_TRUE(killed);
    EXPECT_EQ(waitMeces, 1000);
#endif
}

TEST(PluginProcess, ut_processStateChanged_queue)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");
    stub_ext::StubExt st;
    bool called = false;
    st.set_lamda(&PluginProcess::processStateChanged, [&called]() {
        called = true;
    });

    auto pcs = pp.m_processes.value("test");
    emit pcs->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    EXPECT_FALSE(called);

    //必须队列响应
    called = false;
    QTest::qWaitFor([&called](){return called;}, 30);
    EXPECT_TRUE(called);
}

TEST(PluginProcess, ut_processStateChanged)
{
    PluginProcess pp;
    pp.addProgram("test", "/usr/bin/test -t test");
    stub_ext::StubExt st;
    QProcess::ProcessState state = QProcess::Running;
    bool called = false;
    st.set_lamda(&QProcess::state, [&state, &called]() {
        called = true;
        return state;
    });

    QProcess *pcs = nullptr;
    st.set_lamda(&PluginProcess::addChecklist, [&pcs](PluginProcess *,QProcess *p) {
        pcs = p;
    });

    QString name;
    st.set_lamda(&PluginProcess::startProgram, [&name](PluginProcess *,const QString &p) {
        name = p;
        return true;
    });

    auto p = pp.m_processes.value("test");
    emit p->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    QTest::qWaitFor([&called](){return called;}, 300);
    EXPECT_TRUE(called);
    EXPECT_EQ(pcs, p);
    EXPECT_TRUE(name.isEmpty());

    name.clear();
    pcs = nullptr;
    called = false;
    state = QProcess::NotRunning;
    emit p->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    QTest::qWaitFor([&called](){return called;}, 300);
    EXPECT_TRUE(called);
    EXPECT_EQ(pcs, nullptr);
    EXPECT_TRUE(name.isEmpty());

    name.clear();
    pcs = nullptr;
    called = false;
    pp.m_watch.insert("test", true);
    emit p->stateChanged(QProcess::Running, QProcess::QPrivateSignal());
    QTest::qWaitFor([&called](){return called;}, 300);
    EXPECT_TRUE(called);
    EXPECT_EQ(pcs, nullptr);
    EXPECT_EQ(name, QString("test"));
}

TEST(PluginProcess, ut_removeChecklist)
{
    PluginProcess pp;
    stub_ext::StubExt st;
    int tid = -1;
    st.set_lamda(&PluginProcess::killTimer, [&tid](QObject *, int id) {
        tid = id;
    });

    QProcess pcs;
    pp.removeChecklist(&pcs);
    EXPECT_EQ(tid, -1);

    tid = -1;
    pp.m_checklist.insert(&pcs, 0);
    pp.removeChecklist(&pcs);
    EXPECT_EQ(tid, -1);
    EXPECT_TRUE(pp.m_checklist.isEmpty());

    tid = -1;
    pp.m_checklist.insert(&pcs, 4396);
    pp.removeChecklist(&pcs);
    EXPECT_EQ(tid, 4396);
    EXPECT_TRUE(pp.m_checklist.isEmpty());
}

TEST(PluginProcess, ut_addChecklist)
{
    PluginProcess pp;
    QProcess pcs;
    stub_ext::StubExt st;
    QProcess *rp = nullptr;
    st.set_lamda(&PluginProcess::removeChecklist, [&rp](PluginProcess *,QProcess *p) {
        rp = p;
    });

    pp.addChecklist(&pcs);
    auto id = pp.m_checklist.value(&pcs, -1);
    EXPECT_EQ(rp, &pcs);
    ASSERT_GT(id, 0);
}

TEST(PluginProcess, ut_checkStability)
{
    PluginProcess pp;
    QProcess pcs;
    stub_ext::StubExt st;
    QProcess::ProcessState state = QProcess::NotRunning;
    bool called;
    st.set_lamda(&QProcess::state, [&state, &called]() {
        called = true;
        return state;
    });
    pp.m_restartCount.insert(&pcs, 1);
    pp.checkStability(&pcs);
    EXPECT_EQ(pp.m_restartCount.value(&pcs), 1);

    state = QProcess::Running;
    pp.checkStability(&pcs);
    EXPECT_TRUE(pp.m_restartCount.isEmpty());
}
