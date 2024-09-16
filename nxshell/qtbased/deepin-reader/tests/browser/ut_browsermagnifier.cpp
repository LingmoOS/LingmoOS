// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "BrowserMagniFier.h"
#include "BrowserPage.h"

#include <QPainter>

#include "stub.h"

#include <gtest/gtest.h>
using namespace deepin_reader;

/********测试BrowserAnnotation***********/
class TestReadMagnifierManager : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    QWidget *m_widget = nullptr;
    ReadMagnifierManager *m_tester = nullptr;
};
void TestReadMagnifierManager::SetUp()
{
    m_widget = new QWidget;
    m_tester = new ReadMagnifierManager(m_widget);
}

void TestReadMagnifierManager::TearDown()
{
    delete m_widget;
}

/*************桩函数*******************/
static QString g_funcName;
static void start_stub(QThread::Priority)
{
    g_funcName = __FUNCTION__;
}

static bool isVisible_stub()
{
    g_funcName = __FUNCTION__;
    return true;
}

QImage getImagePoint_stub(double, QPoint)
{
    g_funcName = __FUNCTION__;
    return QImage();
}
/*************测试用例****************/
TEST_F(TestReadMagnifierManager, UT_TestReadMagnifierManager_addTask_001)
{
    MagnifierInfo_t task;
    Stub s;
    s.set(ADDR(QThread, start), start_stub);

    m_tester->addTask(task);
    EXPECT_TRUE(m_tester->m_tTasklst.size() == 1);
    EXPECT_TRUE(g_funcName == "start_stub");
}

TEST_F(TestReadMagnifierManager, UT_TestReadMagnifierManager_run_001)
{
    MagnifierInfo_t task;
    task.target = static_cast<QObject *>(m_tester);
    task.slotFun = "isVisible";
    m_tester->m_tTasklst << task;
    Stub s;
    s.set(ADDR(QWidget, isVisible), isVisible_stub);
    s.set(ADDR(BrowserPage, getImagePoint), getImagePoint_stub);

    m_tester->run();
    EXPECT_TRUE(m_tester->m_tTasklst.size() == 0);
    EXPECT_TRUE(g_funcName == "getImagePoint_stub");
}

/********测试BrowserAnnotation***********/
class TestBrowserMagniFier : public ::testing::Test
{
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    BrowserMagniFier *m_tester = nullptr;
    DocSheet *m_docsheet = nullptr;
};


void TestBrowserMagniFier::SetUp()
{
    QString strPath = UTSOURCEDIR;
    strPath += "/files/normal.pdf";
    m_docsheet = new DocSheet(Dr::FileType::PDF, strPath, nullptr);
    m_tester = new BrowserMagniFier(m_docsheet->m_browser);
}

void TestBrowserMagniFier::TearDown()
{
    delete m_tester;
    delete m_docsheet;
}

/**********桩函数***********/
void setMagniFierImage_stub(const QImage &)
{
    g_funcName = __FUNCTION__;
}

void fillRect_stub(const QRect &, const QColor &)
{
    g_funcName = __FUNCTION__;
}
bool isNull_stub()
{
    return false;
}

void drawImage_stub(int, int, const QImage &, int, int, int, int, Qt::ImageConversionFlags)
{
    g_funcName = __FUNCTION__;
}
/**********测试用例************/
TEST_F(TestBrowserMagniFier, UT_TestBrowserMagniFier_updateImage_001)
{
    Stub s;
    s.set(ADDR(QThread, start), start_stub);
    m_tester->updateImage();
    EXPECT_TRUE(m_tester->m_readManager->m_tTasklst.size() == 1);
}

TEST_F(TestBrowserMagniFier, UT_TestBrowserMagniFier_showMagnigierImage_001)
{
    Stub s;
    s.set(ADDR(QThread, start), start_stub);

    QPoint mousePos(100, 100);
    QPoint magnifierPos(100, 100);
    double scaleFactor = 1;

    m_tester->showMagnigierImage(mousePos, magnifierPos, scaleFactor);
    EXPECT_TRUE(m_tester->m_readManager->m_tTasklst.size() == 1);
}

TEST_F(TestBrowserMagniFier, UT_TestBrowserMagniFier_onUpdateMagnifierImage_001)
{
    Stub s;
    s.set(ADDR(BrowserMagniFier, setMagniFierImage), setMagniFierImage_stub);

    m_tester->m_lastPoint = QPoint(0, 0);
    m_tester->m_lastScaleFactor = 1.0;
    MagnifierInfo_t task;
    task.mousePos = m_tester->m_lastPoint;
    task.scaleFactor = m_tester->m_lastScaleFactor;
    QImage image;
    m_tester->onUpdateMagnifierImage(task, image);
    EXPECT_TRUE(g_funcName == "setMagniFierImage_stub");
}

TEST_F(TestBrowserMagniFier, UT_TestBrowserMagniFier_setMagniFierImage_001)
{
    Stub s;
    s.set(static_cast<void (QPainter::*)(const QRect &, const QColor &)>(ADDR(QPainter, fillRect)), fillRect_stub);

    QImage image;
    m_tester->setMagniFierImage(image);
    EXPECT_TRUE(g_funcName == "fillRect_stub");

    s.set(ADDR(QImage, isNull), isNull_stub);
    s.set(static_cast<void (QPainter::*)(int, int, const QImage &, int, int, int, int, Qt::ImageConversionFlags)>(ADDR(QPainter, drawImage)), drawImage_stub);
    m_tester->setMagniFierImage(image);
    EXPECT_TRUE(g_funcName == "drawImage_stub");
}
