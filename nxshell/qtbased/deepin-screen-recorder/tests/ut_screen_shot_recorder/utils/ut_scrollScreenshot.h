// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/utils/scrollScreenshot.h"

using namespace std;
using namespace testing;


ACCESS_PRIVATE_FIELD(ScrollScreenshot, PixMergeThread *, m_PixMerageThread);
ACCESS_PRIVATE_FIELD(ScrollScreenshot, ScrollScreenshot::ScrollStatus, m_curStatus);
ACCESS_PRIVATE_FIELD(ScrollScreenshot, QTimer *, m_mouseWheelTimer);
class ScrollScreenshotTest: public testing::Test
{
public:
    Stub stub;
    ScrollScreenshot *m_ScrollScreenshot = nullptr;
    virtual void SetUp() override
    {
        if (m_ScrollScreenshot == nullptr)
            m_ScrollScreenshot = new ScrollScreenshot();
        std::cout << "start ScrollScreenshotTest" << std::endl;
    }

    virtual void TearDown() override
    {
        if (nullptr != m_ScrollScreenshot) {
            delete m_ScrollScreenshot;
            m_ScrollScreenshot = nullptr;
        }
        std::cout << "end ScrollScreenshotTest" << std::endl;
    }
};

//添加图片
TEST_F(ScrollScreenshotTest, startAddPixmap)
{
    QPixmap img(":/testImg/addImg1.png");
    m_ScrollScreenshot->setScrollModel(false);
    m_ScrollScreenshot->addPixmap(img);
    QEventLoop loop;
    QTimer::singleShot(500, &loop, SLOT(quit()));
    loop.exec();
    m_ScrollScreenshot->setScrollModel(true);
    m_ScrollScreenshot->addPixmap(img);
    QEventLoop loop2;
    QTimer::singleShot(1000, &loop2, SLOT(quit()));
    loop2.exec();
    PixMergeThread *m_PixMerageThread = access_private_field::ScrollScreenshotm_PixMerageThread(*m_ScrollScreenshot);
    m_PixMerageThread->stopTask();
    m_PixMerageThread->wait();
}
void start_stub2(int msec)
{

    qDebug() << "模拟定时器启动！";
}
void stop_stub()
{
    qDebug() << "模拟定时器停止！";

}
TEST_F(ScrollScreenshotTest, changeState)
{
    ScrollScreenshot::ScrollStatus &m_curStatus = access_private_field::ScrollScreenshotm_curStatus(*m_ScrollScreenshot);
    m_curStatus = ScrollScreenshot::ScrollStatus::Merging;
//    QTimer *&m_mouseWheelTimer = access_private_field::ScrollScreenshotm_mouseWheelTimer(*m_ScrollScreenshot);
    stub.set((void(QTimer::*)(int))ADDR(QTimer, start), start_stub2);
    stub.set(ADDR(QTimer, stop), stop_stub);
    m_ScrollScreenshot->changeState(true);

    QEventLoop loop2;
    QTimer::singleShot(50, &loop2, SLOT(quit()));
    loop2.exec();;

    m_curStatus = ScrollScreenshot::ScrollStatus::Stop;
    m_ScrollScreenshot->changeState(false);

    stub.reset(ADDR(QTimer, stop));
    stub.reset((void(QTimer::*)(int))ADDR(QTimer, start));
}

TEST_F(ScrollScreenshotTest, savePixmap)
{
    ScrollScreenshot::ScrollStatus &m_curStatus = access_private_field::ScrollScreenshotm_curStatus(*m_ScrollScreenshot);
    m_curStatus = ScrollScreenshot::ScrollStatus::Merging;
    m_ScrollScreenshot->savePixmap();
    QEventLoop loop;
    QTimer::singleShot(800, &loop, SLOT(quit()));
    loop.exec();
}

TEST_F(ScrollScreenshotTest, merageImgState)
{
    PixMergeThread::MergeErrorValue state = PixMergeThread::MergeErrorValue::Failed;
    PixMergeThread *m_PixMerageThread = access_private_field::ScrollScreenshotm_PixMerageThread(*m_ScrollScreenshot);
    emit m_PixMerageThread->merageError(state);
    QEventLoop loop;
    QTimer::singleShot(50, &loop, SLOT(quit()));
    loop.exec();

    state = PixMergeThread::MergeErrorValue::MaxHeight;
    emit m_PixMerageThread->merageError(state);
}

TEST_F(ScrollScreenshotTest, merageInvalidArea)
{
    QRect rect(1, 1, 1, 1);
    PixMergeThread::MergeErrorValue state = PixMergeThread::MergeErrorValue::InvalidArea;
    PixMergeThread *m_PixMerageThread = access_private_field::ScrollScreenshotm_PixMerageThread(*m_ScrollScreenshot);
    emit m_PixMerageThread->invalidAreaError(state, rect);
    QEventLoop loop;
    QTimer::singleShot(50, &loop, SLOT(quit()));
    loop.exec();
    state = PixMergeThread::MergeErrorValue::MaxHeight;
    emit m_PixMerageThread->invalidAreaError(state, rect);
}

TEST_F(ScrollScreenshotTest, ScrollScreenshotOthers)
{
    m_ScrollScreenshot->clearPixmap();
    m_ScrollScreenshot->getInvalidArea();
    m_ScrollScreenshot->setTimeAndCalculateTimeDiff(77942255);
}
