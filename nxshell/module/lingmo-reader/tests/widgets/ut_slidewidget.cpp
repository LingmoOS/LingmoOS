// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SlideWidget.h"
#include "SlidePlayWidget.h"
#include "Global.h"
#include "DocSheet.h"
#include "ut_defines.h"
#include "MainWindow.h"
#include "ReaderImageThreadPoolManager.h"
#include "stub.h"
#include "Application.h"

#include <gtest/gtest.h>
#include <QTimer>
#include <QTest>
#include <QPropertyAnimation>
#include <QDBusMessage>
#include <QDBusInterface>

namespace {
void ReaderImageThreadPoolManager_addgetDocImageTask_stub(const ReaderImageParam_t &);
void show_stub();
class TestSlideWidget : public ::testing::Test
{
public:
    TestSlideWidget(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        Stub stub;
        stub.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), ReaderImageThreadPoolManager_addgetDocImageTask_stub);
        stub.set(ADDR(QWidget, show), show_stub);

        QString filePath = QCoreApplication::applicationDirPath() + "/" + "files" + "/" + UT_FILE_PDF;
        if (!QFile(filePath).exists() && QFile(":/files/" + QString(UT_FILE_PDF)).exists()) {
            QDir().mkpath(QCoreApplication::applicationDirPath() + "/" + "files");
            QFile(":/files/" + QString(UT_FILE_PDF)).copy(filePath);
        }
        m_sheet = new DocSheet(Dr::FileType::PDF, filePath, nullptr);
        m_tester = new SlideWidget(m_sheet);
        m_tester->disconnect();

    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_sheet;
    }

protected:
    DocSheet *m_sheet = nullptr;
    SlideWidget *m_tester = nullptr;
};

static QString g_funcname;
void showControl_stub()
{
    g_funcname = __FUNCTION__;
}
int pageCount_stub()
{
    return 2;
}
void playImage_stub()
{
    g_funcname = __FUNCTION__;
}
void closeSlide_stub()
{
    g_funcname = __FUNCTION__;
}
void setPlayStatus_stub(bool)
{
    g_funcname = __FUNCTION__;
}
void onPreBtnClicked_stub()
{
    g_funcname = __FUNCTION__;
}
void onNextBtnClicked_stub()
{
    g_funcname = __FUNCTION__;
}
void ReaderImageThreadPoolManager_addgetDocImageTask_stub(const ReaderImageParam_t &)
{
    g_funcname = __FUNCTION__;
}
QPixmap getImageForDocSheet_stub(void *, DocSheet *, int pageIndex)
{
    g_funcname = QString::number(pageIndex);
    return QPixmap();
}

QDBusMessage QDBusMessage_call_stub(QDBus::CallMode, const QString &,
                                    const QVariant &,
                                    const QVariant &,
                                    const QVariant &,
                                    const QVariant &,
                                    const QVariant &,
                                    const QVariant &,
                                    const QVariant &,
                                    const QVariant &)
{
    g_funcname = __FUNCTION__;
    return QDBusMessage();
}
typedef QDBusMessage (QDBusInterface::*QDBusMessage_call_Ptr)(QDBus::CallMode , const QString &,
                                               const QVariant &,
                                               const QVariant &,
                                               const QVariant &,
                                               const QVariant &,
                                               const QVariant &,
                                               const QVariant &,
                                               const QVariant &,
                                               const QVariant &);

void show_stub()
{

}
}



TEST_F(TestSlideWidget, initTest)
{

}

TEST_F(TestSlideWidget, testonImagevalueChanged)
{
    m_tester->m_offset = 0;
    m_tester->onImagevalueChanged(QVariant(123));
    EXPECT_EQ(m_tester->m_offset, 123);
}

TEST_F(TestSlideWidget, testonParentDestroyed)
{
    m_tester->m_parentIsDestroyed = false;
    m_tester->onParentDestroyed();
    EXPECT_TRUE(m_tester->m_parentIsDestroyed);
}

TEST_F(TestSlideWidget, testmouseMoveEvent)
{
    Stub s;
    s.set(ADDR(SlidePlayWidget, showControl), showControl_stub);
    QMouseEvent *event = new QMouseEvent(QEvent::MouseMove, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mouseMoveEvent(event);
    delete event;
    EXPECT_TRUE(g_funcname == "showControl_stub");
}

TEST_F(TestSlideWidget, testsetWidgetState)
{
    m_tester->setWidgetState(true);
    m_tester->setWidgetState(false);
}

TEST_F(TestSlideWidget, testonPreBtnClicked)
{
    Stub stub;
    stub.set(ADDR(DocSheet, pageCount), pageCount_stub);
    stub.set(ADDR(SlideWidget, playImage), playImage_stub);

    //pageCount:2, index:2
    m_tester->m_curPageIndex = 2;
    m_tester->onPreBtnClicked();
    EXPECT_EQ(m_tester->m_preIndex, 2);
    EXPECT_EQ(m_tester->m_curPageIndex, 1);
    EXPECT_TRUE(g_funcname == "playImage_stub");
}

TEST_F(TestSlideWidget, testonPlayBtnClicked)
{
    //打桩，否则会有线程异常的问题
    Stub stub;
    stub.set(ADDR(SlideWidget, playImage), playImage_stub);

    m_tester->m_canRestart = false;
    m_tester->m_slidePlayWidget->m_autoPlay = true;
    m_tester->onPlayBtnClicked();
    m_tester->m_imageTimer->stop();
    EXPECT_TRUE(m_tester->m_canRestart);
}

TEST_F(TestSlideWidget, testonNextBtnClicked)
{
    Stub stub;
    stub.set(ADDR(DocSheet, pageCount), pageCount_stub);
    stub.set(ADDR(SlideWidget, playImage), playImage_stub);

    //pageCount:2, index:0
    m_tester->m_curPageIndex = 0;
    m_tester->onNextBtnClicked();
    EXPECT_EQ(m_tester->m_preIndex, 0);
    EXPECT_EQ(m_tester->m_curPageIndex, 1);
    EXPECT_TRUE(g_funcname == "playImage_stub");
}

TEST_F(TestSlideWidget, testononExitBtnClicked)
{
    Stub stub;
    stub.set(ADDR(DocSheet, closeSlide), closeSlide_stub);
    m_tester->onExitBtnClicked();
    EXPECT_TRUE(g_funcname == "closeSlide_stub");
}

TEST_F(TestSlideWidget, testplayImage)
{
    Stub stub;
    stub.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), ReaderImageThreadPoolManager_addgetDocImageTask_stub);
    stub.set((QDBusMessage_call_Ptr)ADDR(QDBusInterface, call), QDBusMessage_call_stub);

    m_tester->m_blefttoright = false;
    m_tester->m_imageAnimation->setStartValue(10);
    m_tester->playImage();
    EXPECT_FALSE(m_tester->m_blefttoright);
    EXPECT_EQ(m_tester->m_imageAnimation->startValue(), 0);
}

TEST_F(TestSlideWidget, testonImageShowTimeOut)
{
    Stub stub;
    stub.set(ADDR(SlidePlayWidget, setPlayStatus), setPlayStatus_stub);
    stub.set(ADDR(DocSheet, pageCount), pageCount_stub);
    stub.set(ADDR(SlideWidget, playImage), playImage_stub);

    //pageCount:2, index:2
    m_tester->m_curPageIndex = 2;
    m_tester->m_canRestart = false;
    m_tester->onImageShowTimeOut();
    EXPECT_EQ(m_tester->m_preIndex, 2);
    EXPECT_EQ(m_tester->m_curPageIndex, 1);
    EXPECT_TRUE(g_funcname == "setPlayStatus_stub");
}

TEST_F(TestSlideWidget, testdrawImage)
{
    QPixmap srcImage("1.png");
    QPixmap pixmap(static_cast<int>(m_tester->width() * dApp->devicePixelRatio()), static_cast<int>(m_tester->height() * dApp->devicePixelRatio()));
    pixmap.setDevicePixelRatio(dApp->devicePixelRatio());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHints(QPainter::SmoothPixmapTransform);
    qreal iwidth = srcImage.width();
    qreal iheight = srcImage.height();
    painter.drawPixmap(static_cast<int>((pixmap.width() - iwidth) * 0.5 / dApp->devicePixelRatio()),
                       static_cast<int>((pixmap.height() - iheight) * 0.5 / dApp->devicePixelRatio()), srcImage);

    EXPECT_EQ(m_tester->drawImage(srcImage).width(), pixmap.width());
}

TEST_F(TestSlideWidget, testmousePressEvent)
{
    Stub s;
    s.set(ADDR(SlidePlayWidget, showControl), showControl_stub);

    QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonRelease, QPointF(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    m_tester->mousePressEvent(event);
    delete event;
    EXPECT_TRUE(g_funcname == "showControl_stub");
}

TEST_F(TestSlideWidget, testhandleKeyPressEvent)
{
    Stub stub;
    stub.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), ReaderImageThreadPoolManager_addgetDocImageTask_stub);
    stub.set(ADDR(SlidePlayWidget, setPlayStatus), setPlayStatus_stub);
    stub.set(ADDR(SlideWidget, onPreBtnClicked), onPreBtnClicked_stub);
    stub.set(ADDR(SlideWidget, onNextBtnClicked), onNextBtnClicked_stub);

    m_tester->handleKeyPressEvent(Dr::key_space);
    EXPECT_TRUE(g_funcname == "setPlayStatus_stub");
    m_tester->handleKeyPressEvent(Dr::key_left);
    EXPECT_TRUE(g_funcname == "onPreBtnClicked_stub");
    m_tester->handleKeyPressEvent(Dr::key_right);
    EXPECT_TRUE(g_funcname == "onNextBtnClicked_stub");
}

TEST_F(TestSlideWidget, testonFetchImage)
{
    Stub stub;
    stub.set(ADDR(ReaderImageThreadPoolManager, addgetDocImageTask), ReaderImageThreadPoolManager_addgetDocImageTask_stub);
    m_tester->onFetchImage(0);
    EXPECT_TRUE(g_funcname == "ReaderImageThreadPoolManager_addgetDocImageTask_stub");
}

TEST_F(TestSlideWidget, testonUpdatePageImage)
{
    Stub stub;
    stub.set(ADDR(ReaderImageThreadPoolManager, getImageForDocSheet), getImageForDocSheet_stub);
    m_tester->m_preIndex = 1;
    m_tester->m_curPageIndex = 0;
    m_tester->onUpdatePageImage(0);
    EXPECT_TRUE(g_funcname == "0");
}

