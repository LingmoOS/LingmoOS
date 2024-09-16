// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTimer>
#include <QTest>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/utils/pixmergethread.h"

using namespace std;
using namespace testing;

QRect getScrollChangeRectArea_stub(cv::Mat &img1, const cv::Mat &img2)
{
    int x = 1;
    int y = 1;
    int width = 60;
    int hight = 60;
    return QRect(x, y, width, hight);
}

QRect getScrollChangeRectArea_stub2(cv::Mat &img1, const cv::Mat &img2)
{
    int x = 1;
    int y = 1;
    int width = -60;
    int hight = -60;
    return QRect(x, y, width, hight);
}

ACCESS_PRIVATE_FIELD(PixMergeThread, int, m_upCount);
ACCESS_PRIVATE_FIELD(PixMergeThread, int, m_downCount);
ACCESS_PRIVATE_FIELD(PixMergeThread, cv::Mat, m_curImg);
ACCESS_PRIVATE_FIELD(PixMergeThread, unsigned int, m_MeragerCount);
ACCESS_PRIVATE_FIELD(PixMergeThread, int, m_curTimeDiff);

ACCESS_PRIVATE_FUN(PixMergeThread, QRect(cv::Mat &, const cv::Mat &), getScrollChangeRectArea);
ACCESS_PRIVATE_FUN(PixMergeThread, cv::Mat(const QPixmap &), qPixmapToCvMat);
ACCESS_PRIVATE_FUN(PixMergeThread, bool(const cv::Mat &), splicePictureDown);
ACCESS_PRIVATE_FUN(PixMergeThread, bool(const cv::Mat &), splicePictureUp);

class PixMergeThreadTest: public testing::Test
{
public:
    Stub stub;
    PixMergeThread *m_pixMergeThread = nullptr;
    virtual void SetUp() override
    {
        if (m_pixMergeThread == nullptr)
            m_pixMergeThread = new PixMergeThread();
        std::cout << "start PixMergeThreadTest" << std::endl;
    }

    virtual void TearDown() override
    {
        if (nullptr != m_pixMergeThread) {
            delete m_pixMergeThread;
            m_pixMergeThread = nullptr;
        }
        std::cout << "end PixMergeThreadTest" << std::endl;
    }
};

//向下拼接
TEST_F(PixMergeThreadTest, startPixMergeThreadScrollDown)
{
    m_pixMergeThread->clearCurImg();
    QPixmap img1(":/testImg/addImg1.png");
    QPixmap img2(":/testImg/addImg2.png");
    QPixmap img3(":/testImg/addImg3.png");
    m_pixMergeThread->setScrollModel(true);
    m_pixMergeThread->start();
    m_pixMergeThread->addShotImg(img1);
    m_pixMergeThread->addShotImg(img2);
    m_pixMergeThread->addShotImg(img3);

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_pixMergeThread->stopTask();
    m_pixMergeThread->wait();
}

//向上拼接
TEST_F(PixMergeThreadTest, startPixMergeThreadScrollUp)
{
    m_pixMergeThread->clearCurImg();
    QPixmap img1(":/testImg/addImg1.png");
    QPixmap img2(":/testImg/addImg2.png");
    QPixmap img3(":/testImg/addImg3.png");
    m_pixMergeThread->setScrollModel(true);
    m_pixMergeThread->start();
    m_pixMergeThread->addShotImg(img3, PixMergeThread::PictureDirection::ScrollUp);
    m_pixMergeThread->addShotImg(img2, PixMergeThread::PictureDirection::ScrollUp);
    m_pixMergeThread->addShotImg(img1, PixMergeThread::PictureDirection::ScrollUp);

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    m_pixMergeThread->stopTask();
    m_pixMergeThread->wait();
}


//向上拼接上半部分的异常处理
TEST_F(PixMergeThreadTest, splicePictureUpUpperHalfError)
{
    QPixmap invalidArea1(":/testImg/upUpperHalfError1.png");
    QPixmap invalidArea2(":/testImg/upUpperHalfError2.png");
    cv::Mat &m_curImg = access_private_field::PixMergeThreadm_curImg(*m_pixMergeThread);
    m_curImg = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, invalidArea1);
    cv::Mat invalidAreaData = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, invalidArea2);
    unsigned int &m_MeragerCount = access_private_field::PixMergeThreadm_MeragerCount(*m_pixMergeThread);
    auto pixfun = get_private_fun::PixMergeThreadgetScrollChangeRectArea();
    stub.set(pixfun, getScrollChangeRectArea_stub);

    m_MeragerCount = 0;
    call_private_fun::PixMergeThreadsplicePictureUp(*m_pixMergeThread, invalidAreaData);

    m_MeragerCount = 1;
    int &m_curTimeDiff = access_private_field::PixMergeThreadm_curTimeDiff(*m_pixMergeThread);
    m_curTimeDiff = 1;
    call_private_fun::PixMergeThreadsplicePictureUp(*m_pixMergeThread, invalidAreaData);

    stub.reset(pixfun);
    stub.set(pixfun, getScrollChangeRectArea_stub2);
    m_MeragerCount = 0;
    call_private_fun::PixMergeThreadsplicePictureUp(*m_pixMergeThread, invalidAreaData);
    stub.reset(pixfun);
}

//向上拼接下半部分的异常处理
TEST_F(PixMergeThreadTest, splicePictureUpLowerHalfError)
{
    QPixmap invalidArea1(":/testImg/upLowerHalfError1.png");
    QPixmap invalidArea2(":/testImg/upLowerHalfError2.png");
    cv::Mat &m_curImg = access_private_field::PixMergeThreadm_curImg(*m_pixMergeThread);
    m_curImg = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, invalidArea1);
    cv::Mat invalidAreaData = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, invalidArea2);
    unsigned int &m_MeragerCount = access_private_field::PixMergeThreadm_MeragerCount(*m_pixMergeThread);
    auto pixfun = get_private_fun::PixMergeThreadgetScrollChangeRectArea();
    stub.set(pixfun, getScrollChangeRectArea_stub);

    m_MeragerCount = 0;
    call_private_fun::PixMergeThreadsplicePictureUp(*m_pixMergeThread, invalidAreaData);

    m_MeragerCount = 1;
    call_private_fun::PixMergeThreadsplicePictureUp(*m_pixMergeThread, invalidAreaData);

    m_MeragerCount = 1;
    int &m_curTimeDiff = access_private_field::PixMergeThreadm_curTimeDiff(*m_pixMergeThread);
    m_curTimeDiff = 1;
    call_private_fun::PixMergeThreadsplicePictureUp(*m_pixMergeThread, invalidAreaData);

    stub.reset(pixfun);
    stub.set(pixfun, getScrollChangeRectArea_stub2);
    m_MeragerCount = 0;
    call_private_fun::PixMergeThreadsplicePictureUp(*m_pixMergeThread, invalidAreaData);
    stub.reset(pixfun);
}

//向下拼接上半部分的异常处理
TEST_F(PixMergeThreadTest, splicePictureDownUpperHalfError)
{
    QPixmap invalidArea1(":/testImg/downUpperHalfError1.png");
    QPixmap invalidArea2(":/testImg/downUpperHalfError2.png");
    cv::Mat &m_curImg = access_private_field::PixMergeThreadm_curImg(*m_pixMergeThread);
    m_curImg = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, invalidArea1);
    cv::Mat invalidAreaData = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, invalidArea2);
    unsigned int &m_MeragerCount = access_private_field::PixMergeThreadm_MeragerCount(*m_pixMergeThread);
    auto pixfun = get_private_fun::PixMergeThreadgetScrollChangeRectArea();
    stub.set(pixfun, getScrollChangeRectArea_stub);

    m_MeragerCount = 0;
    m_pixMergeThread->setScrollModel(false);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, invalidAreaData);

    m_MeragerCount = 0;
    m_pixMergeThread->setScrollModel(true);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, invalidAreaData);

    m_MeragerCount = 1;
    m_pixMergeThread->setScrollModel(false);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, invalidAreaData);

    m_MeragerCount = 1;
    m_pixMergeThread->setScrollModel(true);
    int &m_curTimeDiff = access_private_field::PixMergeThreadm_curTimeDiff(*m_pixMergeThread);
    m_curTimeDiff = 1;
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, invalidAreaData);

    stub.reset(pixfun);
    stub.set(pixfun, getScrollChangeRectArea_stub2);
    m_MeragerCount = 0;
    m_pixMergeThread->setScrollModel(false);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, invalidAreaData);

    m_MeragerCount = 0;
    m_pixMergeThread->setScrollModel(true);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, invalidAreaData);
    stub.reset(pixfun);
}

//向下拼接下半部分的异常处理
TEST_F(PixMergeThreadTest, splicePictureDownLowerHalfError)
{
    QPixmap img1(":/testImg/addImg1.png");
    cv::Mat imgData = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, img1);
    cv::Mat &m_curImg = access_private_field::PixMergeThreadm_curImg(*m_pixMergeThread);
    m_curImg = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, img1);
    unsigned int &m_MeragerCount = access_private_field::PixMergeThreadm_MeragerCount(*m_pixMergeThread);

    m_MeragerCount = 0;
    m_pixMergeThread->setScrollModel(false);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, imgData);

    m_MeragerCount = 0;
    m_pixMergeThread->setScrollModel(true);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, imgData);

    m_MeragerCount = 1;
    m_pixMergeThread->setScrollModel(false);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, imgData);

    m_MeragerCount = 1;
    m_pixMergeThread->setScrollModel(true);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, imgData);

    m_MeragerCount = 1;
    m_pixMergeThread->setScrollModel(true);
    int &m_curTimeDiff = access_private_field::PixMergeThreadm_curTimeDiff(*m_pixMergeThread);
    m_curTimeDiff = 1;
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, imgData);

    QPixmap invalidArea1(":/testImg/invalidArea3.png");
    QPixmap invalidArea2(":/testImg/invalidArea4.png");
    m_curImg = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, invalidArea1);
    cv::Mat invalidAreaData = call_private_fun::PixMergeThreadqPixmapToCvMat(*m_pixMergeThread, invalidArea2);

    m_MeragerCount = 0;
    m_pixMergeThread->setScrollModel(false);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, invalidAreaData);

    m_MeragerCount = 0;
    m_pixMergeThread->setScrollModel(true);
    call_private_fun::PixMergeThreadsplicePictureDown(*m_pixMergeThread, invalidAreaData);
}

//其他
TEST_F(PixMergeThreadTest, PixMergeThreadOthers)
{
    int &m_upCount  = access_private_field::PixMergeThreadm_upCount(*m_pixMergeThread);
    m_upCount = 1;
    int &m_downCount  = access_private_field::PixMergeThreadm_downCount(*m_pixMergeThread);
    m_downCount = 1;
    m_pixMergeThread->isOneWay();
    m_downCount = 0;
    m_pixMergeThread->isOneWay();

    m_pixMergeThread->calculateTimeDiff(77942255);
    m_pixMergeThread->getMerageResult();
}








