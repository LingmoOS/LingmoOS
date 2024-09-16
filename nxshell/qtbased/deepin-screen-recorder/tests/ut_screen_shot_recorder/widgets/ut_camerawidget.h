// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include "stub.h"
#include "addr_pri.h"

#include "../../src/widgets/camerawidget.h"

using namespace testing;

bool _isNull_stub()
{
    return false;
}

ACCESS_PRIVATE_FUN(CameraWidget, void(QEvent *e), enterEvent);
ACCESS_PRIVATE_FUN(CameraWidget, void(QMouseEvent *event), mousePressEvent);
ACCESS_PRIVATE_FUN(CameraWidget, void(QMouseEvent *event), mouseMoveEvent);
ACCESS_PRIVATE_FUN(CameraWidget, void(QMouseEvent *event), mouseReleaseEvent);
ACCESS_PRIVATE_FUN(CameraWidget, void(QPaintEvent *e), paintEvent);

class CameraWidgetTest:public testing::Test{

public:
    Stub stub;
    CameraWidget *m_cameraWidget;
    virtual void SetUp() override{
        m_cameraWidget = new CameraWidget();
        stub.set(ADDR(QCameraInfo,isNull),_isNull_stub);
        m_cameraWidget->initCamera();
    }

    virtual void TearDown() override{
        stub.reset(ADDR(QCameraInfo,isNull));
        if(nullptr != m_cameraWidget)
            delete m_cameraWidget;
    }
};

TEST_F(CameraWidgetTest, enterEvent)
{
    QEvent *e = new QEvent(QEvent::Enter);
    call_private_fun::CameraWidgetenterEvent(*m_cameraWidget,e);

    delete e;
}

TEST_F(CameraWidgetTest, mousePressEvent)
{
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::CameraWidgetmousePressEvent(*m_cameraWidget,ev);

    delete ev;
}

TEST_F(CameraWidgetTest, mouseMoveEvent)
{
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::CameraWidgetmouseMoveEvent(*m_cameraWidget,ev);
    delete ev;
}

TEST_F(CameraWidgetTest, mouseReleaseEvent)
{
    QMouseEvent *ev = new QMouseEvent(QEvent::MouseButtonPress, QPoint(10,10), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    call_private_fun::CameraWidgetmouseReleaseEvent(*m_cameraWidget,ev);

    delete ev;
}

TEST_F(CameraWidgetTest, paintEvent)
{
    QPaintEvent *e = new QPaintEvent(QRect());
    call_private_fun::CameraWidgetpaintEvent(*m_cameraWidget,e);

    delete e;
}

TEST_F(CameraWidgetTest, setRecordRect)
{
    m_cameraWidget->setRecordRect(10,10,100,100);
}

TEST_F(CameraWidgetTest, showAt)
{
    m_cameraWidget->showAt(QPoint(10,10));
}

TEST_F(CameraWidgetTest, postion)
{
    qDebug() << m_cameraWidget->postion();
}

TEST_F(CameraWidgetTest, getRecordX)
{
    m_cameraWidget->setRecordRect(10,10,50,50);
    EXPECT_LT(0,m_cameraWidget->getRecordX());
}

TEST_F(CameraWidgetTest, getRecordY)
{
    m_cameraWidget->setRecordRect(10,10,50,50);
    EXPECT_LT(0,m_cameraWidget->getRecordY());
}

TEST_F(CameraWidgetTest, getRecordWidth)
{
    m_cameraWidget->setRecordRect(10,10,50,50);
    EXPECT_LT(0,m_cameraWidget->getRecordWidth());
}

TEST_F(CameraWidgetTest, getRecordHeight)
{
    m_cameraWidget->setRecordRect(10,10,50,50);
    EXPECT_LT(0,m_cameraWidget->getRecordHeight());
}

TEST_F(CameraWidgetTest, cameraStart)
{
    m_cameraWidget->cameraStart();
}

TEST_F(CameraWidgetTest, cameraStop)
{
    m_cameraWidget->cameraStop();
}

TEST_F(CameraWidgetTest, cameraResume)
{
    m_cameraWidget->cameraResume();
}

TEST_F(CameraWidgetTest, captureImage)
{
    m_cameraWidget->captureImage();
}

TEST_F(CameraWidgetTest, processCapturedImage)
{
    QImage img;
    m_cameraWidget->processCapturedImage(0,img);
}

TEST_F(CameraWidgetTest, deleteCapturedImage)
{
    m_cameraWidget->deleteCapturedImage(0,QString("ut"));
}

TEST_F(CameraWidgetTest, setCameraStop)
{
    m_cameraWidget->setCameraStop(true);
}

TEST_F(CameraWidgetTest, getcameraStatus)
{
    m_cameraWidget->getcameraStatus();
}

TEST_F(CameraWidgetTest, cameraInitError)
{
    m_cameraWidget->cameraInitError(QCamera::Error::NoError);
}

