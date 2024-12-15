// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageRenderThread.h"
#include "DocSheet.h"
#include "BrowserPage.h"
#include "stub.h"

#include <QDebug>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include <gtest/gtest.h>
/********测试PageRenderThread***********/
class TestPageRenderThread : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    PageRenderThread *m_tester = nullptr;
};

void TestPageRenderThread::SetUp()
{
    m_tester = PageRenderThread::instance();
}

void TestPageRenderThread::TearDown()
{
    PageRenderThread::destroyForever();
}

/**********桩函数*************/
static QString g_funcName;
static void start_stub(QThread::Priority)
{
    g_funcName = __FUNCTION__;
    qInfo() << "chendu" << __FUNCTION__;
}

static bool wait_stub(unsigned long)
{
    g_funcName = __FUNCTION__;
    qInfo() << "chendu" << __FUNCTION__;
    return true;
}
void run_stub()
{
    g_funcName = __FUNCTION__;
    qInfo() << "chendu" << __FUNCTION__;
}
/*********测试用例**********/
//TEST_F(TestPageRenderThread, UT_PageRenderThread_clearImageTasks_001)
//{
//    typedef void (*fptr)(PageRenderThread *);
//    fptr A_foo = (fptr)(&PageRenderThread::run);   //获取虚函数地址
//    Stub s;
////    s.set(ADDR(QThread, start), start_stub);
////    s.set(ADDR(QThread, wait), wait_stub);
//    s.set(A_foo, run_stub);
//    EXPECT_TRUE(m_tester->clearImageTasks(nullptr, nullptr, 0));

//    QString strPath = UTSOURCEDIR;
//    strPath += "/files/normal.pdf";
//    DocSheet *docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
//    BrowserPage *browserpage = new BrowserPage(docsheet->m_browser, 0, docsheet);
//    DocPageNormalImageTask task;
//    task.sheet = docsheet;
//    task.page = browserpage;
//    task.rect = QRect(0, 0, 500, 800);
//    m_tester->appendTask(task);
//    EXPECT_TRUE(m_tester->clearImageTasks(docsheet, browserpage, 0));

//}
