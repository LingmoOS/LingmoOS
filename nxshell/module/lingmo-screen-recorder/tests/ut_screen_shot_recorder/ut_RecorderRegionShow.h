// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QPoint>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/RecorderRegionShow.h"
#include "../../src/utils.h"

using namespace testing;

class RecorderRegionShowTest: public testing::Test
{

public:
    Stub stub;
//    RecorderRegionShow *m_pRecorderRegion;
    virtual void SetUp() override
    {
        std::cout << "start RecorderRegionShowTest" << std::endl;
//        RecorderRegionShow *m_pRecorderRegion =  new RecorderRegionShow();
//        m_pRecorderRegion->resize(800, 600);
//        m_pRecorderRegion->move(100, 100);
//        m_pRecorderRegion->show();
    }

    virtual void TearDown() override
    {
        std::cout << "end RecorderRegionShowTest" << std::endl;
//        m_pRecorderRegion->hide();
//        delete m_pRecorderRegion;
    }
};

//TEST_F(RecorderRegionShowTest, showKeyBoardButtons)
//{
//    for(int i = 0; i < 6; ++i) {
//        QString key = QString("%1").arg(i);
//        m_pRecorderRegion->showKeyBoardButtons(key);
//        m_pRecorderRegion->update();
//        QEventLoop loop;
//        QTimer::singleShot(1000, &loop, SLOT(quit()));
//        loop.exec();
//    }
//    QEventLoop loop;
//    QTimer::singleShot(3000, &loop, SLOT(quit()));
//    loop.exec();
//}

void initCamera_stub()
{

}

bool cameraStart_stub()
{
    return true;
}
void passInputEvent_stub1(int wid)
{
    Q_UNUSED(wid);
}
bool getcameraStatus_stub()
{
    return false;
}
TEST_F(RecorderRegionShowTest, initCameraInfoRightBottom)
{
    RecorderRegionShow *pRecorderRegion =  new RecorderRegionShow();
    pRecorderRegion->resize(800, 600);
    pRecorderRegion->move(100, 100);
    pRecorderRegion->show();
    stub.set(ADDR(CameraWidget, initCamera), initCamera_stub);
    stub.set(ADDR(CameraWidget, cameraStart), cameraStart_stub);
    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub1);
    stub.set(ADDR(CameraWidget, getcameraStatus), getcameraStatus_stub);
    //执行case函数
    pRecorderRegion->initCameraInfo(CameraWidget::Position::rightBottom, QSize(420, 250));
    stub.reset(ADDR(CameraWidget, initCamera));
    stub.reset(ADDR(CameraWidget, cameraStart));
    stub.reset(ADDR(Utils, passInputEvent));
    delete pRecorderRegion;
    stub.reset(ADDR(CameraWidget, getcameraStatus));

}

TEST_F(RecorderRegionShowTest, initCameraInfoRightTop)
{
    RecorderRegionShow *pRecorderRegion =  new RecorderRegionShow();
    pRecorderRegion->resize(800, 600);
    pRecorderRegion->move(100, 100);
    pRecorderRegion->show();
    stub.set(ADDR(CameraWidget, initCamera), initCamera_stub);
    stub.set(ADDR(CameraWidget, cameraStart), cameraStart_stub);
    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub1);
    stub.set(ADDR(CameraWidget, getcameraStatus), getcameraStatus_stub);
    //执行case函数
    pRecorderRegion->initCameraInfo(CameraWidget::Position::rightTop, QSize(420, 250));
    stub.reset(ADDR(CameraWidget, initCamera));
    stub.reset(ADDR(CameraWidget, cameraStart));
    stub.reset(ADDR(Utils, passInputEvent));
    delete pRecorderRegion;
    stub.reset(ADDR(CameraWidget, getcameraStatus));

}

TEST_F(RecorderRegionShowTest, initCameraInfoLeftBottom)
{
    RecorderRegionShow *pRecorderRegion =  new RecorderRegionShow();
    pRecorderRegion->resize(800, 600);
    pRecorderRegion->move(100, 100);
    pRecorderRegion->show();
    stub.set(ADDR(CameraWidget, initCamera), initCamera_stub);
    stub.set(ADDR(CameraWidget, cameraStart), cameraStart_stub);
    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub1);
    stub.set(ADDR(CameraWidget, getcameraStatus), getcameraStatus_stub);
    //执行case函数
    pRecorderRegion->initCameraInfo(CameraWidget::Position::leftBottom, QSize(420, 250));
    stub.reset(ADDR(CameraWidget, initCamera));
    stub.reset(ADDR(CameraWidget, cameraStart));
    stub.reset(ADDR(Utils, passInputEvent));
    delete pRecorderRegion;
    stub.reset(ADDR(CameraWidget, getcameraStatus));

}

TEST_F(RecorderRegionShowTest, initCameraInfoLeftTop)
{
    RecorderRegionShow *pRecorderRegion =  new RecorderRegionShow();
    pRecorderRegion->resize(800, 600);
    pRecorderRegion->move(100, 100);
    pRecorderRegion->show();

    stub.set(ADDR(CameraWidget, initCamera), initCamera_stub);
    stub.set(ADDR(CameraWidget, cameraStart), cameraStart_stub);
    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub1);
    stub.set(ADDR(CameraWidget, getcameraStatus), getcameraStatus_stub);
    //执行case函数
    pRecorderRegion->initCameraInfo(CameraWidget::Position::leftTop, QSize(420, 250));
    stub.reset(ADDR(CameraWidget, initCamera));
    stub.reset(ADDR(CameraWidget, cameraStart));
    stub.reset(ADDR(Utils, passInputEvent));

    delete pRecorderRegion;
    stub.reset(ADDR(CameraWidget, getcameraStatus));

}
ACCESS_PRIVATE_FUN(RecorderRegionShow, void(), updateMultiKeyBoardPos);
void updateMultiKeyBoardPos_stub()
{

}
TEST_F(RecorderRegionShowTest, showKeyBoardButtons)
{
    RecorderRegionShow *pRecorderRegion =  new RecorderRegionShow();
    pRecorderRegion->resize(800, 600);
    pRecorderRegion->move(100, 100);
    pRecorderRegion->show();
    auto RecorderRegionShow_updateMultiKeyBoardPos = get_private_fun::RecorderRegionShowupdateMultiKeyBoardPos();
    stub.set(RecorderRegionShow_updateMultiKeyBoardPos, updateMultiKeyBoardPos_stub);
    //执行case函数
    pRecorderRegion->showKeyBoardButtons("K");
    stub.reset(RecorderRegionShow_updateMultiKeyBoardPos);
    delete pRecorderRegion;
}

ACCESS_PRIVATE_FIELD(RecorderRegionShow, QList<KeyButtonWidget *>, m_keyButtonList);
TEST_F(RecorderRegionShowTest, updateKeyBoardButtonStyle)
{
    RecorderRegionShow *pRecorderRegion =  new RecorderRegionShow();
    pRecorderRegion->resize(800, 600);
    pRecorderRegion->move(100, 100);
    pRecorderRegion->show();
    for (int i = 0; i < 6; i++) {
        KeyButtonWidget *t_keyWidget = new KeyButtonWidget();
        t_keyWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
        t_keyWidget->setKeyLabelWord(QString::number(i));
        access_private_field::RecorderRegionShowm_keyButtonList(*pRecorderRegion).append(t_keyWidget);
    }
    //执行case函数
    pRecorderRegion->updateKeyBoardButtonStyle();
    delete pRecorderRegion;
}

ACCESS_PRIVATE_FIELD(RecorderRegionShow, CameraWidget *, m_cameraWidget);
TEST_F(RecorderRegionShowTest, setCameraShow)
{
    RecorderRegionShow *pRecorderRegion =  new RecorderRegionShow();
    pRecorderRegion->resize(800, 600);
    pRecorderRegion->move(100, 100);
    pRecorderRegion->show();
    access_private_field::RecorderRegionShowm_cameraWidget(*pRecorderRegion) = new CameraWidget();
    stub.set(ADDR(CameraWidget, getcameraStatus), getcameraStatus_stub);
    //执行case函数
    pRecorderRegion->setCameraShow();
    delete pRecorderRegion;
    stub.reset(ADDR(CameraWidget, getcameraStatus));
}
ACCESS_PRIVATE_FIELD(RecorderRegionShow, QPainter *, m_painter);
ACCESS_PRIVATE_FUN(RecorderRegionShow, void(QPaintEvent *), paintEvent);
TEST_F(RecorderRegionShowTest, paintEvent)
{
    if (QSysInfo::currentCpuArchitecture().startsWith("arm"))
        return;
    RecorderRegionShow *pRecorderRegion =  new RecorderRegionShow();
    pRecorderRegion->resize(800, 600);
    pRecorderRegion->move(100, 100);
    pRecorderRegion->show();
    //access_private_field::RecorderRegionShowm_painter(*pRecorderRegion) = new QPainter();
    QPaintEvent *event;
    //执行case函数
    call_private_fun::RecorderRegionShowpaintEvent(*pRecorderRegion, event);
    delete pRecorderRegion;
}
TEST_F(RecorderRegionShowTest, updateMultiKeyBoardPos)
{
    RecorderRegionShow *pRecorderRegion =  new RecorderRegionShow();
    pRecorderRegion->resize(800, 600);
    pRecorderRegion->move(100, 100);
    pRecorderRegion->show();
//    for (int i = 0; i < 6; i++) {
//        KeyButtonWidget *t_keyWidget = new KeyButtonWidget();
//        t_keyWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::ToolTip);
//        t_keyWidget->setKeyLabelWord(QString::number(i));
//        access_private_field::RecorderRegionShowm_keyButtonList(*pRecorderRegion).append(t_keyWidget);
//    }
    //执行case函数
    call_private_fun::RecorderRegionShowupdateMultiKeyBoardPos(*pRecorderRegion);
    delete pRecorderRegion;
}
