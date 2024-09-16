// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>
#include <QAction>

#define private public
#define project public
#include "stub.h"
#include "addr_pri.h"
#include "../../src/widgets/subtoolwidget.h"
#include "../../src/main_window.h"

using namespace testing;
void click_stub()
{

}

bool isChecked_true_stub_ut()
{
    return true;
}

bool isChecked_false_stub()
{
    return false;
}

QString toString_true_stub()
{
    return QString("true");
}

QString toString_false_stub()
{
    return QString("false");
}

class SubToolWidgetTest: public testing::Test
{

public:
    Stub stub;
    SubToolWidget *m_subToolWidget;
    MainWindow *m_mainWindow;
    virtual void SetUp() override
    {
        m_mainWindow = new MainWindow;
        m_subToolWidget = new SubToolWidget(m_mainWindow);
        m_subToolWidget->initWidget();
//        m_subToolWidget->initRecordLabel();
//        m_subToolWidget->initShotLabel();
    }

    virtual void TearDown() override
    {
        if (nullptr != m_subToolWidget)
            delete m_subToolWidget;
        if (nullptr != m_mainWindow)
            delete m_mainWindow;
    }
};

TEST_F(SubToolWidgetTest, installTipHint)
{
    QWidget *w1 = new QWidget();
    m_subToolWidget->installTipHint(w1, QString("矩形"));

    QWidget *w2 = new QWidget(w1);
    QWidget *w3 = new QWidget(w2);
    m_subToolWidget->installTipHint(w3, QString("画笔"));
    if (nullptr != w3)
        delete w3;
    if (nullptr != w2)
        delete w2;
    if (nullptr != w1)
        delete w1;
}

/*
TEST_F(SubToolWidgetTest, installHint)
{
    QWidget *w = new QWidget();
    QWidget *hint = new QWidget();
    m_subToolWidget->installHint(w,hint);
    if(nullptr != w)
        w->deleteLater();
    if(nullptr != hint)
        hint->deleteLater();
}

TEST_F(SubToolWidgetTest, switchContent)
{
    m_subToolWidget->switchContent(QString("record"));
    m_subToolWidget->switchContent(QString("shot"));
}
*/
TEST_F(SubToolWidgetTest, changeArrowAndLineFromSideBar)
{
    m_subToolWidget->changeArrowAndLineFromSideBar(0);
    stub.set(ADDR(QAbstractButton, isChecked), isChecked_true_stub_ut);
    m_subToolWidget->changeArrowAndLineFromSideBar(1);
    stub.reset(ADDR(QAbstractButton, isChecked));

    stub.set(ADDR(QAbstractButton, isChecked), isChecked_false_stub);
    m_subToolWidget->changeArrowAndLineFromSideBar(1);
    stub.reset(ADDR(QAbstractButton, isChecked));

    stub.set(ADDR(QAbstractButton, isChecked), isChecked_true_stub_ut);
    m_subToolWidget->changeArrowAndLineFromSideBar(1);
    stub.reset(ADDR(QAbstractButton, isChecked));

    stub.set(ADDR(QAbstractButton, isChecked), isChecked_false_stub);
    m_subToolWidget->changeArrowAndLineFromSideBar(1);
    stub.reset(ADDR(QAbstractButton, isChecked));
}

TEST_F(SubToolWidgetTest, setRecordLaunchMode)
{
    m_subToolWidget->setRecordLaunchMode(true);
}

TEST_F(SubToolWidgetTest, setVideoButtonInitFromSub)
{
    m_subToolWidget->m_cameraButton->setChecked(true);
    m_subToolWidget->setVideoButtonInitFromSub();
}

TEST_F(SubToolWidgetTest, shapeClickedFromWidget)
{
    m_subToolWidget->shapeClickedFromWidget(QString("rect"));
    m_subToolWidget->shapeClickedFromWidget(QString("circ"));
    m_subToolWidget->shapeClickedFromWidget(QString("line"));
    m_subToolWidget->shapeClickedFromWidget(QString("pen"));
    m_subToolWidget->shapeClickedFromWidget(QString("text"));
    m_subToolWidget->shapeClickedFromWidget(QString("keyBoard"));
    m_subToolWidget->shapeClickedFromWidget(QString("mouse"));
    m_subToolWidget->shapeClickedFromWidget(QString("camera"));
    m_subToolWidget->shapeClickedFromWidget(QString("audio"));
    stub.set(ADDR(QAbstractButton, click), click_stub);
    m_subToolWidget->shapeClickedFromWidget(QString("option"));
    stub.reset(ADDR(QAbstractButton, click));
}

TEST_F(SubToolWidgetTest, setMicroPhoneEnable)
{
    m_subToolWidget->setMicroPhoneEnable(false);
    m_subToolWidget->setMicroPhoneEnable(true);
}

TEST_F(SubToolWidgetTest, setCameraDeviceEnable)
{
    m_subToolWidget->setCameraDeviceEnable(false);
    m_subToolWidget->setCameraDeviceEnable(true);
}

TEST_F(SubToolWidgetTest, setSystemAudioEnable)
{
    m_subToolWidget->setSystemAudioEnable(false);
    m_subToolWidget->setSystemAudioEnable(true);
}

TEST_F(SubToolWidgetTest, initRecordLabel)
{
    m_subToolWidget->m_haveMicroPhone = true;
    stub.set(ADDR(QVariant, toString), toString_true_stub);
    m_subToolWidget->initRecordLabel();
    stub.reset(ADDR(QVariant, toString));

    stub.set(ADDR(QVariant, toString), toString_false_stub);
    m_subToolWidget->initRecordLabel();
    stub.reset(ADDR(QVariant, toString));
    m_subToolWidget->m_systemAudioAction->triggered(true);

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
}



//TEST_F(SubToolWidgetTest, installHint)
//{
//    QWidget *w = new QWidget();
//    QWidget *hint = new QWidget();
//    m_subToolWidget->installHint(w,hint);
//}

//TEST_F(SubToolWidgetTest, switchContent)
//{
//    m_subToolWidget->switchContent(QString("record"));
//    m_subToolWidget->switchContent(QString("shot"));
//}




