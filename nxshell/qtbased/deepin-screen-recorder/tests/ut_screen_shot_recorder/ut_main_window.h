// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../../src/utils.h"
#include "../../src/main_window.h"

#include <QTest>
#include <QPoint>
#include <QScreen>
#include <QDir>
#include "stub.h"
#include "addr_pri.h"
#include <gtest/gtest.h>

using namespace testing;


ACCESS_PRIVATE_FIELD(MainWindow, ToolBar *, m_toolBar);
ACCESS_PRIVATE_FIELD(MainWindow, int,  m_screenCount);
ACCESS_PRIVATE_FIELD(MainWindow, ShapesWidget *, m_shapesWidget);

QRect geometry_stub()
{
    return QRect(0, 0, 1920, 1080);
}

void passInputEvent_stub(int wid)
{
    Q_UNUSED(wid);
}

qreal devicePixelRatio_stub_2()
{
    return 1;
}

int width_stub_2()
{
    return 1920;
}

int height_stub_2()
{
    return 1080;
}

int screenCount_stub()
{
    return 1;
}

class MainWindowTest: public testing::Test
{

public:
    Stub stub;
//    MainWindow *m_window = nullptr;
    virtual void SetUp() override
    {
//        m_window = new MainWindow;
        std::cout << "start MainWindowTest" << std::endl;
        ConfigSettings::instance()->setValue("save", "saveCursor", true);
    }

    virtual void TearDown() override
    {
//        if (m_window) {
//            std::cout << "delete  m_window" << std::endl;
//            //m_window->deleteLater();
//            delete  m_window;
//            m_window = nullptr;
//        }
        std::cout << "end MainWindowTest" << std::endl;
        system("killall deepin-shortcut-viewer");
    }
};
/*
TEST_F(MainWindowTest, initAttributes)
{
    stub.set(ADDR(QScreen,geometry),geometry_stub);
    m_window->initAttributes();
    stub.reset(ADDR(QScreen,geometry));
}

TEST_F(MainWindowTest, initResource)
{
    stub.set(ADDR(QScreen,geometry),geometry_stub);
    m_window->initAttributes();
    stub.reset(ADDR(QScreen,geometry));

    stub.set(ADDR(Utils,passInputEvent),passInputEvent_stub);
    m_window->initResource();
    stub.reset(ADDR(Utils,passInputEvent));
}

TEST_F(MainWindowTest, initLaunchMode)
{
    stub.set(ADDR(QScreen,geometry),geometry_stub);
    m_window->initAttributes();
    stub.reset(ADDR(QScreen,geometry));

    stub.set(ADDR(Utils,passInputEvent),passInputEvent_stub);
    m_window->initResource();
    stub.reset(ADDR(Utils,passInputEvent));

    m_window->initLaunchMode("screenShot");
}
*/

TEST_F(MainWindowTest, screenShotShapes)
{
    MainWindow *window = new MainWindow();
    stub.set(ADDR(QScreen, geometry), geometry_stub);
    window->initAttributes();
    stub.reset(ADDR(QScreen, geometry));

    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
    window->initResource();
    stub.reset(ADDR(Utils, passInputEvent));

    window->initLaunchMode("screenShot");

    window->showFullScreen();

    access_private_field::MainWindowm_screenCount(*window) = 1;

    QEventLoop loop;

    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10, 10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(800, 600));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800, 600));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800, 600));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(900, 700));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(900, 700));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(900, 10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(800, 20));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800, 20));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(800, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(900, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(900, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 700));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(400, 900));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 900));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();




    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(10, 10));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(20, 30));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(20, 30));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(20, 50));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(30, 50));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(30, 50));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(window, Qt::Key_R);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    window->changeShotToolEvent("rectangle");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    ShapesWidget *m_shapesWidget = access_private_field::MainWindowm_shapesWidget(*window);


    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Left, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Right, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Up, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Down, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_shapesWidget, Qt::Key_Left,  Qt::ShiftModifier | Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Right, Qt::ShiftModifier | Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Up, Qt::ShiftModifier | Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Down, Qt::ShiftModifier | Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Left, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Right, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Up, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Down, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(m_shapesWidget, Qt::Key_Z, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    ConfigSettings::instance()->setValue("rectangle", "is_blur", true);
    ConfigSettings::instance()->setValue("rectangle", "is_mosaic", false);

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(600, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::keyClick(window, Qt::Key_O);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    ConfigSettings::instance()->setValue("oval", "is_blur", false);
    ConfigSettings::instance()->setValue("oval", "is_mosaic", true);

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(300, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(500, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(500, 180));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(window, Qt::Key_L);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    //window->changeArrowAndLineEvent(0);
    //QTimer::singleShot(1000, &loop, SLOT(quit()));
    //loop.exec();

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(200, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(200, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    //window->changeArrowAndLineEvent(1);
    //QTimer::singleShot(1000, &loop, SLOT(quit()));
    //loop.exec();


    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(120, 210));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(250, 360));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(250, 360));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(window, Qt::Key_P);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(50, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(320, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(m_shapesWidget, QPoint(120, 180));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(280, 280));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_T);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(m_shapesWidget, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(60, 60));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(m_shapesWidget, Qt::Key_Z, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    window->saveScreenShot();
    QTimer::singleShot(2000, &loop, SLOT(quit()));
    loop.exec();

    if (window) {
        delete window;
        window = nullptr;
    }
}

TEST_F(MainWindowTest, screenShot)
{
    MainWindow *window = new MainWindow();
    stub.set(ADDR(QScreen, geometry), geometry_stub);
    window->initAttributes();
    stub.reset(ADDR(QScreen, geometry));

    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
    window->initResource();
    stub.reset(ADDR(Utils, passInputEvent));

    window->initLaunchMode("screenShot");

    window->showFullScreen();


    QEventLoop loop;
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mouseMove(window, QPoint(960, 480));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(100, 100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(100, 100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(250, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(250, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Left, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Right, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Up, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Down, Qt::ControlModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::keyClick(window, Qt::Key_Left, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Right, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Up, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Down, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Left, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Right, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Up, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Down, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    window->changeFunctionButton("record");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_K);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_C);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(200, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(300, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(300, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    window->changeFunctionButton("shot");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_R);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(350, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_O);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(355, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_L);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(350, 300));
    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(360, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_P);

    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(365, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_T);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(370, 300));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();



    QTest::keyClick(window, Qt::Key_Question, Qt::ControlModifier | Qt::ShiftModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_Escape, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    window->sendSavingNotify();
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    window->responseEsc();

//    window->saveScreenShot();


    QTest::mouseClick(window, Qt::MouseButton::RightButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 200));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(400, 250));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTimer::singleShot(3000, &loop, SLOT(quit()));
    loop.exec();

    //退出截图
    QTest::keyClick(window, Qt::Key_Escape, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    if (window) {
        delete window;
        window = nullptr;
    }
}

TEST_F(MainWindowTest, screenRecord)
{
    MainWindow *window = new MainWindow();

    stub.set(ADDR(QScreen, devicePixelRatio), devicePixelRatio_stub_2);
    stub.set(ADDR(QWidget, width), width_stub_2);
    stub.set(ADDR(QWidget, height), height_stub_2);
    stub.set(ADDR(QScreen, geometry), geometry_stub);
    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
    stub.set(ADDR(QDesktopWidget, screenCount), screenCount_stub);

    window->initAttributes();
    window->initResource();
    window->initLaunchMode("screenShot");
    window->showFullScreen();

    //ToolBar *m_toolBar = access_private_field::MainWindowm_toolBar(*window);

    QEventLoop loop;


//    QTest::mouseMove(window, QPoint(0,0));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();

    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(0, 0));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

//    QTest::mouseMove(window, QPoint(1400,1050));
//    QTimer::singleShot(1000, &loop, SLOT(quit()));
//    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1400, 1050));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    //window->changeCameraSelectEvent(true);
    window->changeKeyBoardShowEvent(true);
    window->changeMouseShowEvent(true);

    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::keyClick(window, Qt::Key_R);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::keyClick(window, Qt::Key_W);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::keyClick(window, Qt::Key_W);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    QTest::keyClick(window, Qt::Key_Q);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();


    window->showKeyBoardButtons("F1");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    window->showKeyBoardButtons("F2");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    window->showKeyBoardButtons("F3");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    window->changeSystemAudioSelectEvent(true);
    window->changeCameraSelectEvent(true);

    window->startCountdown();
    QTimer::singleShot(5000, &loop, SLOT(quit()));
    loop.exec();

    window->showKeyBoardButtons("F4");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    window->showKeyBoardButtons("F5");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    window->showKeyBoardButtons("F6");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    window->showKeyBoardButtons("F7");
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    window->showPressFeedback(100, 100);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    window->showDragFeedback(100, 100);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    window->showReleaseFeedback(100, 100);

    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    window->forciblySavingNotify();

    window->stopRecord();
    QTimer::singleShot(1000, &loop, SLOT(quit()));


    stub.reset(ADDR(QScreen, devicePixelRatio));
    stub.reset(ADDR(QWidget, width));
    stub.reset(ADDR(QWidget, height));
    stub.reset(ADDR(QScreen, geometry));
    stub.reset(ADDR(Utils, passInputEvent));
    stub.reset(ADDR(QDesktopWidget, screenCount));

    loop.exec();

    if (window) {
        delete window;
        window = nullptr;
    }
}

TEST_F(MainWindowTest, scrollShot)
{
    MainWindow *window = new MainWindow();
    stub.set(ADDR(QScreen, geometry), geometry_stub);
    window->initAttributes();
    stub.reset(ADDR(QScreen, geometry));

    stub.set(ADDR(Utils, passInputEvent), passInputEvent_stub);
    window->initResource();
    stub.reset(ADDR(Utils, passInputEvent));

    window->initLaunchMode("screenShot");

    window->showFullScreen();

    QEventLoop loop;
    QTest::mouseMove(window, QPoint(700, 100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mousePress(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(700, 100));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(1400, 800));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseRelease(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1400, 800));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    //启动滚动截图
    QTest::keyClick(window, Qt::Key_I, Qt::AltModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseMove(window, QPoint(1000, 400));
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();

    QTest::mouseClick(window, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier, QPoint(1000, 400));

    QTimer::singleShot(10000, &loop, SLOT(quit()));
    loop.exec();
    //退出滚动截图
    QTest::keyClick(window, Qt::Key_Escape, Qt::NoModifier);
    QTimer::singleShot(1000, &loop, SLOT(quit()));
    loop.exec();
    if (window) {
        delete window;
        window = nullptr;
    }
}


static bool hasComposite_stub(void *obj)
{
    Q_UNUSED(obj);
    return false;
}


static QString CpuArchitecture_stub(void *obj)
{
    Q_UNUSED(obj);
    return  "mips";
}

void initMainWindow_stub()
{

    qDebug() << "replace initMainWindow!";
}
TEST_F(MainWindowTest, onHelp)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    window->onHelp();

    stub.set(ADDR(DWindowManagerHelper, hasComposite), hasComposite_stub);
    stub.set(ADDR(QSysInfo, currentCpuArchitecture), CpuArchitecture_stub);
    window->compositeChanged();
    stub.reset(ADDR(DWindowManagerHelper, hasComposite));
    stub.reset(ADDR(QSysInfo, currentCpuArchitecture));
    if (window) {
        delete window;
        window = nullptr;
    }
}


ACCESS_PRIVATE_FIELD(MainWindow, int, m_autoScrollFlagNext);
//
TEST_F(MainWindowTest, onScrollShotCheckScrollType)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    window->onScrollShotCheckScrollType(10);

    int &MainWindow_m_autoScrollFlagNext =  access_private_field::MainWindowm_autoScrollFlagNext(*window);

    EXPECT_EQ(10, MainWindow_m_autoScrollFlagNext);
    delete window;
}

ACCESS_PRIVATE_FIELD(MainWindow, int, recordX);
ACCESS_PRIVATE_FIELD(MainWindow, int, recordY);
ACCESS_PRIVATE_FIELD(MainWindow, int, recordWidth);
ACCESS_PRIVATE_FIELD(MainWindow, int, recordHeight);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_screenWidth);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_screenHeight);
ACCESS_PRIVATE_FIELD(MainWindow, qreal, m_pixelRatio);
ACCESS_PRIVATE_FIELD(MainWindow, ScrollShotTip *, m_scrollShotTip);
ACCESS_PRIVATE_FUN(MainWindow, bool(), isToolBarInShotArea);
ACCESS_PRIVATE_FUN(MainWindow, QPoint(), getScrollShotTipPosition);
static bool isToolBarInShotArea_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}


TEST_F(MainWindowTest, getScrollShotTipPosition)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_m_screenWidth =  access_private_field::MainWindowm_screenWidth(*window);
    MainWindow_m_screenWidth = 1920;
    int &MainWindow_m_screenHeight =  access_private_field::MainWindowm_screenHeight(*window);
    MainWindow_m_screenHeight = 1080;
    int &MainWindow_m_screenCount =  access_private_field::MainWindowm_screenCount(*window);
    MainWindow_m_screenCount = 1;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;
    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->resize(100, 40);
    //MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    auto MainWindow_isToolBarInShotArea = get_private_fun::MainWindowisToolBarInShotArea();
    stub.set(MainWindow_isToolBarInShotArea, isToolBarInShotArea_stub);

    auto point = call_private_fun::MainWindowgetScrollShotTipPosition(*window);
    QPoint tempPoint = QPoint(static_cast<int>((1920  - MainWindow_m_scrollShotTip->width() * MainWindow_m_pixelRatio) / 2),
                              static_cast<int>(100 + 15 * MainWindow_m_pixelRatio));
    EXPECT_EQ(tempPoint, point);

    stub.reset(MainWindow_isToolBarInShotArea);
    stub.reset(ADDR(MainWindow, initMainWindow));
    qDebug() << "delete window! start" ;
    delete window;
    qDebug() << "delete window! end" ;

}

ACCESS_PRIVATE_FIELD(MainWindow, int, m_scrollShotStatus);
ACCESS_PRIVATE_FIELD(MainWindow, ScrollScreenshot *, m_scrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), startAutoScrollShot);
//替换ScrollScreenshot的setScrollModel函数
static bool setScrollModel_stub(void *obj, bool)
{
    Q_UNUSED(obj);
    return true;
}
//替换ScrollScreenshot的addPixmap函数
static bool addPixmap_stub(void *obj, const QPixmap &piximg, int wheelDirection)
{
    Q_UNUSED(obj);
    Q_UNUSED(piximg);
    Q_UNUSED(wheelDirection);
    return true;
}
//开始自动滚动截图测试用例
TEST_F(MainWindowTest, startAutoScrollShot)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*window);
    MainWindow_m_scrollShotStatus = 1;
    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, setScrollModel), setScrollModel_stub);
    stub.set(ADDR(ScrollScreenshot, addPixmap), addPixmap_stub);
    call_private_fun::MainWindowstartAutoScrollShot(*window);
    stub.reset(ADDR(ScrollScreenshot, setScrollModel));
    stub.reset(ADDR(ScrollScreenshot, addPixmap));
    stub.reset(ADDR(MainWindow, initMainWindow));

    delete MainWindow_m_scrollShot;
    delete window;
}


ACCESS_PRIVATE_FUN(MainWindow, void(), pauseAutoScrollShot);
static bool changeState_stub(void *obj, bool wheelDirection)
{
    Q_UNUSED(obj);
    Q_UNUSED(wheelDirection);
    return true;
}
//暂停自动滚动截图测试用例
TEST_F(MainWindowTest, pauseAutoScrollShot)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);

    MainWindow *window = new MainWindow();
    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, changeState), changeState_stub);
    call_private_fun::MainWindowpauseAutoScrollShot(*window);
    stub.reset(ADDR(ScrollScreenshot, changeState));
    delete MainWindow_m_scrollShot;

    stub.reset(ADDR(MainWindow, initMainWindow));
    delete window;

}

ACCESS_PRIVATE_FUN(MainWindow, void(), continueAutoScrollShot);
//继续自动滚动截图测试用例
TEST_F(MainWindowTest, continueAutoScrollShot)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    //MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, setScrollModel), setScrollModel_stub);
    stub.set(ADDR(ScrollScreenshot, changeState), changeState_stub);

    call_private_fun::MainWindowcontinueAutoScrollShot(*window);

    stub.reset(ADDR(ScrollScreenshot, setScrollModel));
    stub.reset(ADDR(ScrollScreenshot, changeState));
    delete MainWindow_m_scrollShot;

    stub.reset(ADDR(MainWindow, initMainWindow));
    delete window;

}

ACCESS_PRIVATE_FUN(MainWindow, void(), startManualScrollShot);
//开始手动滚动截图测试用例
TEST_F(MainWindowTest, startManualScrollShot)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, setScrollModel), setScrollModel_stub);
    stub.set(ADDR(ScrollScreenshot, addPixmap), addPixmap_stub);
    call_private_fun::MainWindowstartManualScrollShot(*window);
    stub.reset(ADDR(ScrollScreenshot, setScrollModel));
    stub.reset(ADDR(ScrollScreenshot, addPixmap));
    delete MainWindow_m_scrollShot;
    stub.reset(ADDR(MainWindow, initMainWindow));
    delete window;

}

ACCESS_PRIVATE_FUN(MainWindow, void(int mouseTime, int direction), handleManualScrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(PreviewWidget::PostionStatus previewPostion, int direction, int mouseTime), scrollShotGrabPixmap);

static bool scrollShotGrabPixmap_stub(void *obj, PreviewWidget::PostionStatus previewPostion, int direction, int mouseTime)
{
    Q_UNUSED(obj);
    Q_UNUSED(previewPostion);
    Q_UNUSED(direction);
    Q_UNUSED(mouseTime);
    return true;
}
//处理手动滚动截图逻辑测试用例
TEST_F(MainWindowTest, handleManualScrollShot)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
//    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    auto MainWindow_scrollShotGrabPixmap = get_private_fun::MainWindowscrollShotGrabPixmap();
    stub.set(MainWindow_scrollShotGrabPixmap, scrollShotGrabPixmap_stub);

    call_private_fun::MainWindowhandleManualScrollShot(*window, 1, 4);

    stub.reset(MainWindow_scrollShotGrabPixmap);
    stub.reset(ADDR(MainWindow, initMainWindow));
    delete window;

}

void getInputEvent_stub(const int wid, const short x, const short y, const unsigned short width, const unsigned short height)
{
    Q_UNUSED(wid);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(width);
    Q_UNUSED(height);
    qDebug() << "getInputEvent_stub!";
}

void setInputEvent_stub()
{
    qDebug() << "setInputEvent_stub!";
}
ACCESS_PRIVATE_FUN(MainWindow, void(), setInputEvent);
//滚动截图设置区域穿透测试用例
TEST_F(MainWindowTest, setInputEvent)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;
    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    auto MainWindow_isToolBarInShotArea = get_private_fun::MainWindowisToolBarInShotArea();
    stub.set(MainWindow_isToolBarInShotArea, isToolBarInShotArea_stub);
    stub.set(ADDR(Utils, getInputEvent), getInputEvent_stub);

    call_private_fun::MainWindowsetInputEvent(*window);

    stub.reset(MainWindow_isToolBarInShotArea);
    stub.reset(ADDR(Utils, getInputEvent));

    stub.reset(ADDR(MainWindow, initMainWindow));
    qDebug() << "delete window start!";
    delete window;
    qDebug() << "delete window end!";

}

ACCESS_PRIVATE_FUN(MainWindow, void(), setCancelInputEvent);
//滚动截图时取消捕捉区域的鼠标穿透测试用例
TEST_F(MainWindowTest, setCancelInputEvent)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;
    call_private_fun::MainWindowsetCancelInputEvent(*window);

    stub.reset(ADDR(MainWindow, initMainWindow));
    delete window;

}

ACCESS_PRIVATE_FUN(MainWindow, void(), showAdjustArea);
static QRect getInvalidArea_stub(void *obj)
{
    Q_UNUSED(obj);
    return QRect(0, 0, 500, 500);
}
//显示可调整的捕捉区域大小及位置测试用例
TEST_F(MainWindowTest, showAdjustArea)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, getInvalidArea), getInvalidArea_stub);


    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;

    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    call_private_fun::MainWindowshowAdjustArea(*window);

    stub.reset(ADDR(ScrollScreenshot, getInvalidArea));
    stub.reset(ADDR(MainWindow, initMainWindow));
    delete MainWindow_m_scrollShot;
    delete window;

}

//判断工具栏是否在在捕捉区域内部测试用例
TEST_F(MainWindowTest, isToolBarInShotArea)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);
    MainWindow_toolBar->show();
    bool flag = call_private_fun::MainWindowisToolBarInShotArea(*window);
    MainWindow_toolBar->hide();

    EXPECT_EQ(flag, true);
    stub.reset(ADDR(MainWindow, initMainWindow));
    delete window;

}

void hideSomeToolBtn_stub()
{

}

void resetCursor_stub()
{

}

void showTip_stub(TipType tipType)
{
    Q_UNUSED(tipType);
    qDebug() << "showTip_stub!!!";
}

bool setBackgroundPixmap_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
QPoint getScrollShotTipPosition_stub(void *obj)
{
    Q_UNUSED(obj);
    return QPoint(500, 500);
}
void showScrollShot_stub()
{

}
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_initScroll);
ACCESS_PRIVATE_FIELD(MainWindow, TopTips *, m_sizeTips);
ACCESS_PRIVATE_FUN(MainWindow, void(), initScrollShot);
ACCESS_PRIVATE_FUN(MainWindow, void(), resetCursor);
ACCESS_PRIVATE_FUN(MainWindow, void(), showScrollShot);
TEST_F(MainWindowTest, initScrollShot)
{
    /*
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_initScroll(*window) = false;

    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();
    access_private_field::MainWindowm_sizeTips(*window) = new TopTips() ;
    stub.set(ADDR(ToolBar, hideSomeToolBtn), hideSomeToolBtn_stub);

    auto MainWindow_resetCursor = get_private_fun::MainWindowresetCursor();
    stub.set(MainWindow_resetCursor, resetCursor_stub);

    auto MainWindow_setInputEvent = get_private_fun::MainWindowsetInputEvent();
    stub.set(MainWindow_setInputEvent, setInputEvent_stub);

    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->move(100, 100);
    MainWindow_m_scrollShotTip->resize(100, 40);
    stub.set(ADDR(ScrollShotTip, showTip), showTip_stub);
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);
    stub.set(ADDR(ScrollShotTip, setBackgroundPixmap), setBackgroundPixmap_stub);

    auto MainWindow_getScrollShotTipPosition = get_private_fun::MainWindowgetScrollShotTipPosition();
    stub.set(MainWindow_getScrollShotTipPosition, getScrollShotTipPosition_stub);

    auto MainWindow_isToolBarInShotArea = get_private_fun::MainWindowisToolBarInShotArea();
    stub.set(MainWindow_isToolBarInShotArea, isToolBarInShotArea_stub);


    auto MainWindow_showScrollShot = get_private_fun::MainWindowshowScrollShot();
    stub.set(MainWindow_showScrollShot, showScrollShot_stub);


    call_private_fun::MainWindowinitScrollShot(*window);

    stub.reset(MainWindow_showScrollShot);
    stub.reset(MainWindow_isToolBarInShotArea);
    stub.reset(MainWindow_getScrollShotTipPosition);
    stub.reset(ADDR(ScrollShotTip, setBackgroundPixmap));
    stub.reset(ADDR(ScrollShotTip, showTip));
    stub.reset(MainWindow_resetCursor);
    stub.reset(ADDR(ToolBar, hideSomeToolBtn));
    delete window;
    */
}

ACCESS_PRIVATE_FIELD(MainWindow, QPixmap, m_firstScrollShotImg);
//ACCESS_PRIVATE_FIELD(MainWindow, DPushButton *, m_shotButton);
ACCESS_PRIVATE_FIELD(MainWindow, TopTips *, m_scrollShotSizeTips);
ACCESS_PRIVATE_FIELD(MainWindow, PreviewWidget *, m_previewWidget);
ACCESS_PRIVATE_FUN(MainWindow, void(), updateToolBarPos);
//ACCESS_PRIVATE_FUN(MainWindow, void(), updateShotButtonPos);
static void updateToolBarPos_stub(void *obj)
{
    Q_UNUSED(obj);
    qDebug() << "调整工具栏位置";

}
static void updateShotButtonPos_stub(void *obj)
{
    Q_UNUSED(obj);
    qDebug() << "调整截图保存按钮位置";
}
//初始化滚动截图，显示滚动截图中的一些公共部件、例如工具栏、提示、图片大小、第一张预览图，单元测试用例
TEST_F(MainWindowTest, showScrollShot)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    QRect previewRecordRect {
        static_cast<int>(0),
        static_cast<int>(0),
        static_cast<int>(1920),
        static_cast<int>(1080)
    };
    PreviewWidget *&MainWindow_previewWidget = access_private_field::MainWindowm_previewWidget(*window);
    MainWindow_previewWidget = new PreviewWidget(previewRecordRect);
    MainWindow_previewWidget->setScreenInfo(MainWindow_screenWidth, MainWindow_m_pixelRatio);
    MainWindow_previewWidget->initPreviewWidget();

    //DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*window);
    //MainWindow_shotButton = new DPushButton();

    TopTips *&MainWindow_scrollShotSizeTips = access_private_field::MainWindowm_scrollShotSizeTips(*window);
    MainWindow_scrollShotSizeTips = new TopTips();

    ScrollShotTip *&MainWindow_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_scrollShotTip = new ScrollShotTip();

    auto MainWindow_updateToolBarPos = get_private_fun::MainWindowupdateToolBarPos();
    stub.set(MainWindow_updateToolBarPos, updateToolBarPos_stub);

    //auto MainWindow_updateShotButtonPos = get_private_fun::MainWindowupdateShotButtonPos();
    //stub.set(MainWindow_updateShotButtonPos, updateShotButtonPos_stub);

    //调用滚动
    call_private_fun::MainWindowshowScrollShot(*window);

    stub.reset(MainWindow_updateToolBarPos);
    //stub.reset(MainWindow_updateShotButtonPos);
    stub.reset(ADDR(MainWindow, initMainWindow));


//    delete MainWindow_toolBar;
//    delete MainWindow_previewWidget;
    //delete MainWindow_shotButton;
    delete MainWindow_scrollShotSizeTips;
//    delete MainWindow_scrollShotTip;
    delete window;

}

ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isZhaoxin);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_scrollShotType);
//滚动截图模式，抓取当前捕捉区域的图片，传递给滚动截图处理类进行图片的拼接，单元测试
TEST_F(MainWindowTest, scrollShotGrabPixmap)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    bool &MainWindow_isZhaoxin =  access_private_field::MainWindowm_isZhaoxin(*window);
    MainWindow_isZhaoxin = false;

    int &MainWindow_scrollShotType =  access_private_field::MainWindowm_scrollShotType(*window);
    MainWindow_scrollShotType = 0;

    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, setScrollModel), setScrollModel_stub);
    stub.set(ADDR(ScrollScreenshot, addPixmap), addPixmap_stub);

    auto MainWindow_isToolBarInShotArea = get_private_fun::MainWindowisToolBarInShotArea();
    stub.set(MainWindow_isToolBarInShotArea, isToolBarInShotArea_stub);

    //DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*window);
    //MainWindow_shotButton = new DPushButton();

    TopTips *&MainWindow_scrollShotSizeTips = access_private_field::MainWindowm_scrollShotSizeTips(*window);
    MainWindow_scrollShotSizeTips = new TopTips();

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;
    QRect previewRecordRect {
        static_cast<int>(0),
        static_cast<int>(0),
        static_cast<int>(1920),
        static_cast<int>(1080)
    };
    PreviewWidget *&MainWindow_previewWidget = access_private_field::MainWindowm_previewWidget(*window);
    MainWindow_previewWidget = new PreviewWidget(previewRecordRect);
    MainWindow_previewWidget->setScreenInfo(MainWindow_screenWidth, MainWindow_m_pixelRatio);
    MainWindow_previewWidget->initPreviewWidget();

    call_private_fun::MainWindowscrollShotGrabPixmap(*window, PreviewWidget::PostionStatus::INSIDE, 5, 1);
    call_private_fun::MainWindowscrollShotGrabPixmap(*window, PreviewWidget::PostionStatus::RIGHT, 5, 1);

    stub.reset(ADDR(ScrollScreenshot, setScrollModel));
    stub.reset(ADDR(ScrollScreenshot, addPixmap));
    stub.reset(MainWindow_isToolBarInShotArea);
    stub.reset(ADDR(MainWindow, initMainWindow));

    delete MainWindow_m_scrollShot;
    //delete MainWindow_shotButton;
    delete MainWindow_scrollShotSizeTips;
//    delete MainWindow_toolBar;
//    delete MainWindow_previewWidget;
    delete window;

}

static void updateImage_stub(void *obj, QImage img)
{
    Q_UNUSED(obj);
    Q_UNUSED(img);
    qDebug() << "预览窗口更新图片";
}
ACCESS_PRIVATE_FIELD(MainWindow, ScreenGrabber, m_screenGrabber);
ACCESS_PRIVATE_FUN(MainWindow, void(QImage img), showPreviewWidgetImage);
//显示预览窗口和图片单元测试用例
TEST_F(MainWindowTest, showPreviewWidgetImage)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_screenWidth = 1920;

    QRect previewRecordRect {
        static_cast<int>(0),
        static_cast<int>(0),
        static_cast<int>(1920),
        static_cast<int>(1080)
    };
    PreviewWidget *&MainWindow_previewWidget = access_private_field::MainWindowm_previewWidget(*window);
    MainWindow_previewWidget = new PreviewWidget(previewRecordRect);
    MainWindow_previewWidget->setScreenInfo(MainWindow_screenWidth, MainWindow_m_pixelRatio);
    MainWindow_previewWidget->initPreviewWidget();
    stub.set(ADDR(PreviewWidget, updateImage), updateImage_stub);

    TopTips *&MainWindow_scrollShotSizeTips = access_private_field::MainWindowm_scrollShotSizeTips(*window);
    MainWindow_scrollShotSizeTips = new TopTips();

    ScreenGrabber &MainWindow_m_screenGrabber = access_private_field::MainWindowm_screenGrabber(*window);
    bool ok;
    QPixmap pixmap = MainWindow_m_screenGrabber.grabEntireDesktop(ok, previewRecordRect, MainWindow_m_pixelRatio);

    QImage img = pixmap.toImage();

    call_private_fun::MainWindowshowPreviewWidgetImage(*window, img);
    stub.reset(ADDR(PreviewWidget, updateImage));
    stub.reset(ADDR(MainWindow, initMainWindow));

    delete MainWindow_scrollShotSizeTips;
//    delete MainWindow_previewWidget;
    delete window;

}

ACCESS_PRIVATE_FIELD(MainWindow, int, m_scrollShotMouseClick);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isErrorWithScrollShot);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isAutoScrollShotStart);
ACCESS_PRIVATE_FUN(MainWindow, void(int x, int y), scrollShotMouseClickEvent);
bool startAutoScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
bool pauseAutoScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
bool setCancelInputEvent_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
bool continueAutoScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
//滚动截图鼠标按钮事件单元测试用例
TEST_F(MainWindowTest, scrollShotMouseClickEvent)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
    MainWindow_m_scrollShotTip->move(100, 100);
    MainWindow_m_scrollShotTip->resize(100, 40);
    stub.set(ADDR(ScrollShotTip, showTip), showTip_stub);
    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    //DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*window);
    //MainWindow_shotButton = new DPushButton();

    bool &MainWindow_m_isErrorWithScrollShot = access_private_field::MainWindowm_isErrorWithScrollShot(*window);
    MainWindow_m_isErrorWithScrollShot = false;

    bool &MainWindow_m_isAutoScrollShotStart = access_private_field::MainWindowm_isAutoScrollShotStart(*window);
    MainWindow_m_isAutoScrollShotStart = false;

    int &MainWindow_scrollShotType =  access_private_field::MainWindowm_scrollShotType(*window);
    MainWindow_scrollShotType = 0;

    int &MainWindow_m_scrollShotMouseClick =  access_private_field::MainWindowm_scrollShotMouseClick(*window);
    MainWindow_m_scrollShotMouseClick = 0;

    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*window);
    MainWindow_m_scrollShotStatus = 1;

    auto MainWindow_startAutoScrollShot = get_private_fun::MainWindowstartAutoScrollShot();
    stub.set(MainWindow_startAutoScrollShot, startAutoScrollShot_stub);

    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    auto MainWindow_setCancelInputEvent = get_private_fun::MainWindowsetCancelInputEvent();
    stub.set(MainWindow_setCancelInputEvent, setCancelInputEvent_stub);

    auto MainWindow_setInputEvent = get_private_fun::MainWindowsetInputEvent();
    stub.set(MainWindow_setInputEvent, setInputEvent_stub);

    auto MainWindow_continueAutoScrollShot = get_private_fun::MainWindowcontinueAutoScrollShot();
    stub.set(MainWindow_continueAutoScrollShot, continueAutoScrollShot_stub);

    call_private_fun::MainWindowscrollShotMouseClickEvent(*window, 500, 500);

    call_private_fun::MainWindowscrollShotMouseClickEvent(*window, 500, 500);

    stub.reset(ADDR(ScrollShotTip, showTip));
    stub.reset(MainWindow_startAutoScrollShot);
    stub.reset(MainWindow_pauseAutoScrollShot);
    stub.reset(MainWindow_setCancelInputEvent);
    stub.reset(MainWindow_setInputEvent);
    stub.reset(MainWindow_continueAutoScrollShot);
    stub.reset(ADDR(MainWindow, initMainWindow));

    //delete MainWindow_m_scrollShotTip;
    //delete MainWindow_shotButton;
//    delete MainWindow_toolBar;

    delete window;

}

ACCESS_PRIVATE_FUN(MainWindow, void(int x, int y), scrollShotMouseMoveEvent);
void disableXGrabButton_stub()
{

}
void enableXGrabButton_stub()
{

}
//滚动截图鼠标移动事件处理,单元测试用例
TEST_F(MainWindowTest, scrollShotMouseMoveEvent)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
//    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);
    MainWindow_m_scrollShotTip->move(100, 100);
    MainWindow_m_scrollShotTip->resize(100, 40);
    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    //DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*window);
    //MainWindow_shotButton = new DPushButton();

    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*window);
    MainWindow_m_scrollShotStatus = 1;

    bool &MainWindow_m_isErrorWithScrollShot = access_private_field::MainWindowm_isErrorWithScrollShot(*window);
    MainWindow_m_isErrorWithScrollShot = false;

    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    auto MainWindow_setCancelInputEvent = get_private_fun::MainWindowsetCancelInputEvent();
    stub.set(MainWindow_setCancelInputEvent, setCancelInputEvent_stub);

    stub.set(ADDR(Utils, disableXGrabButton), disableXGrabButton_stub);
    stub.set(ADDR(Utils, enableXGrabButton), enableXGrabButton_stub);


    call_private_fun::MainWindowscrollShotMouseMoveEvent(*window, 500, 500);

    stub.reset(MainWindow_pauseAutoScrollShot);
    stub.reset(MainWindow_setCancelInputEvent);
    stub.reset(ADDR(Utils, disableXGrabButton));
    stub.reset(ADDR(Utils, enableXGrabButton));
    stub.reset(ADDR(MainWindow, initMainWindow));

    delete window;
    //delete MainWindow_m_scrollShotTip;
    //delete MainWindow_shotButton;
//    delete MainWindow_toolBar;
}

ACCESS_PRIVATE_FUN(MainWindow, void(int mouseTime, int direction, int x, int y), scrollShotMouseScrollEvent);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_autoScrollFlagLast);
bool handleManualScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
bool startManualScrollShot_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
//滚动截图时处理鼠标滚轮滚动,单元测试用例
TEST_F(MainWindowTest, scrollShotMouseScrollEvent)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();

    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    int &MainWindow_m_autoScrollFlagNext =  access_private_field::MainWindowm_autoScrollFlagNext(*window);
    MainWindow_m_autoScrollFlagNext = 1;

    int &MainWindow_m_autoScrollFlagLast =  access_private_field::MainWindowm_autoScrollFlagLast(*window);
    MainWindow_m_autoScrollFlagLast = 1;

    int &MainWindow_m_scrollShotType =  access_private_field::MainWindowm_scrollShotType(*window);
    MainWindow_m_scrollShotType = 1;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
//    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    //DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*window);
    //MainWindow_shotButton = new DPushButton();

    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    auto MainWindow_startManualScrollShot = get_private_fun::MainWindowstartManualScrollShot();
    stub.set(MainWindow_startManualScrollShot, startManualScrollShot_stub);

    auto MainWindow_setInputEvent = get_private_fun::MainWindowsetInputEvent();
    stub.set(MainWindow_setInputEvent, setInputEvent_stub);

    auto MainWindow_handleManualScrollShot = get_private_fun::MainWindowhandleManualScrollShot();
    stub.set(MainWindow_handleManualScrollShot, handleManualScrollShot_stub);

    call_private_fun::MainWindowscrollShotMouseScrollEvent(*window, 1, 5, 500, 500);

    stub.reset(MainWindow_pauseAutoScrollShot);
    stub.reset(MainWindow_startManualScrollShot);
    stub.reset(MainWindow_setInputEvent);
    stub.reset(MainWindow_handleManualScrollShot);
    stub.reset(ADDR(MainWindow, initMainWindow));

    //delete MainWindow_m_scrollShotTip;
    //delete MainWindow_shotButton;
//    delete MainWindow_toolBar;
    delete window;

}

ACCESS_PRIVATE_FUN(MainWindow, void(QDBusMessage msg), onLockScreenEvent);
//滚动截图时，锁屏处理事件，单元测试用例
TEST_F(MainWindowTest, onLockScreenEvent)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*window);
    MainWindow_m_scrollShotStatus = 1;

    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    QDBusMessage msg ;
    QList<QVariant> arguments;
    arguments.append("test");
    msg.setArguments(arguments);
    call_private_fun::MainWindowonLockScreenEvent(*window, msg);
    stub.reset(MainWindow_pauseAutoScrollShot);
    stub.reset(ADDR(MainWindow, initMainWindow));
    delete window;


}

ACCESS_PRIVATE_FUN(MainWindow, void(), onOpenScrollShotHelp);
//打开截图录屏帮助文档并定位到滚动截图 单元测试
TEST_F(MainWindowTest, onOpenScrollShotHelp)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    call_private_fun::MainWindowonOpenScrollShotHelp(*window);
    stub.reset(ADDR(MainWindow, initMainWindow));
    delete window;

}

ACCESS_PRIVATE_FUN(MainWindow, void(PixMergeThread::MergeErrorValue state), onScrollShotMerageImgState);
ACCESS_PRIVATE_FUN(MainWindow, QPixmap(const QRect &rect), getPixmapofRect);
ACCESS_PRIVATE_FIELD(MainWindow, QTimer *, m_tipShowtimer);
bool showAdjustArea_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}

QPixmap getPixmapofRect_stub(void *obj, const QRect &rect)
{
    Q_UNUSED(obj);
    Q_UNUSED(rect);
    QPixmap pixmap;
    return pixmap;
}



//滚动截图时，获取拼接时的状态  单元测试用例
TEST_F(MainWindowTest, onScrollShotMerageImgState)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    auto MainWindow_pauseAutoScrollShot = get_private_fun::MainWindowpauseAutoScrollShot();
    stub.set(MainWindow_pauseAutoScrollShot, pauseAutoScrollShot_stub);

    int &MainWindow_m_scrollShotStatus =  access_private_field::MainWindowm_scrollShotStatus(*window);
    MainWindow_m_scrollShotStatus = 1;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
//    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);
    stub.set(ADDR(ScrollShotTip, showTip), showTip_stub);
    stub.set(ADDR(ScrollShotTip, setBackgroundPixmap), setBackgroundPixmap_stub);

    auto MainWindow_setCancelInputEvent = get_private_fun::MainWindowsetCancelInputEvent();
    stub.set(MainWindow_setCancelInputEvent, setCancelInputEvent_stub);

    auto MainWindow_showAdjustArea = get_private_fun::MainWindowshowAdjustArea();
    stub.set(MainWindow_showAdjustArea, showAdjustArea_stub);

    auto MainWindow_getScrollShotTipPosition = get_private_fun::MainWindowgetScrollShotTipPosition();
    stub.set(MainWindow_getScrollShotTipPosition, getScrollShotTipPosition_stub);


    auto MainWindow_getPixmapofRect = get_private_fun::MainWindowgetPixmapofRect();
    stub.set(MainWindow_getPixmapofRect, getPixmapofRect_stub);

    QTimer *&MainWindow_m_tipShowtimer = access_private_field::MainWindowm_tipShowtimer(*window);
    MainWindow_m_tipShowtimer = new QTimer();

    call_private_fun::MainWindowonScrollShotMerageImgState(*window, PixMergeThread::MergeErrorValue::Failed);

    stub.reset(MainWindow_pauseAutoScrollShot);
    stub.reset(ADDR(ScrollShotTip, showTip));
    stub.reset(ADDR(ScrollShotTip, setBackgroundPixmap));
    stub.reset(MainWindow_setCancelInputEvent);
    stub.reset(MainWindow_showAdjustArea);
    stub.reset(MainWindow_getScrollShotTipPosition);
    stub.reset(MainWindow_getPixmapofRect);
    stub.reset(ADDR(MainWindow, initMainWindow));

    //delete MainWindow_m_scrollShotTip;
    delete MainWindow_m_tipShowtimer;
    delete window;

}

ACCESS_PRIVATE_FIELD(MainWindow, QRect, m_adjustArea);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isAdjustArea);
ACCESS_PRIVATE_FUN(MainWindow, void(), onAdjustCaptureArea);
bool clearPixmap_stub(void *obj)
{
    Q_UNUSED(obj);
    return true;
}
//自动调整捕捉区域的大小及位置 单元测试用例
TEST_F(MainWindowTest, onAdjustCaptureArea)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_screenWidth =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_screenWidth = 1920;
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    ScrollShotTip *&MainWindow_m_scrollShotTip = access_private_field::MainWindowm_scrollShotTip(*window);
    MainWindow_m_scrollShotTip = new ScrollShotTip();
//    MainWindow_m_scrollShotTip->showTip(TipType::StartScrollShotTip);

    QRect &MainWindow_m_adjustArea = access_private_field::MainWindowm_adjustArea(*window);
    MainWindow_m_adjustArea = QRect(0, 0, 1920, 1080);

    bool &MainWindow_m_isAdjustArea = access_private_field::MainWindowm_isAdjustArea(*window);
    MainWindow_m_isAdjustArea = false;

    TopTips *&MainWindow_scrollShotSizeTips = access_private_field::MainWindowm_scrollShotSizeTips(*window);
    MainWindow_scrollShotSizeTips = new TopTips();

    ToolBar *&MainWindow_toolBar = access_private_field::MainWindowm_toolBar(*window);
    MainWindow_toolBar = new ToolBar();
    MainWindow_toolBar->resize(500, 100);
    MainWindow_toolBar->move(0, 0);

    //DPushButton *&MainWindow_shotButton = access_private_field::MainWindowm_shotButton(*window);
    //MainWindow_shotButton = new DPushButton();

    QRect previewRecordRect {
        static_cast<int>(0),
        static_cast<int>(0),
        static_cast<int>(1920),
        static_cast<int>(1080)
    };
    PreviewWidget *&MainWindow_previewWidget = access_private_field::MainWindowm_previewWidget(*window);
    MainWindow_previewWidget = new PreviewWidget(previewRecordRect);
    MainWindow_previewWidget->setScreenInfo(MainWindow_screenWidth, MainWindow_m_pixelRatio);
    MainWindow_previewWidget->initPreviewWidget();

    bool &MainWindow_m_isAutoScrollShotStart = access_private_field::MainWindowm_isAutoScrollShotStart(*window);
    MainWindow_m_isAutoScrollShotStart = false;

    bool &MainWindow_isZhaoxin =  access_private_field::MainWindowm_isZhaoxin(*window);
    MainWindow_isZhaoxin = false;

    auto MainWindow_updateToolBarPos = get_private_fun::MainWindowupdateToolBarPos();
    stub.set(MainWindow_updateToolBarPos, updateToolBarPos_stub);

    //auto MainWindow_updateShotButtonPos = get_private_fun::MainWindowupdateShotButtonPos();
    //stub.set(MainWindow_updateShotButtonPos, updateShotButtonPos_stub);

    ScrollScreenshot *&MainWindow_m_scrollShot =  access_private_field::MainWindowm_scrollShot(*window);
    MainWindow_m_scrollShot = new ScrollScreenshot;
    stub.set(ADDR(ScrollScreenshot, clearPixmap), clearPixmap_stub);

    call_private_fun::MainWindowonAdjustCaptureArea(*window);

    stub.reset(MainWindow_updateToolBarPos);
    //stub.reset(MainWindow_updateShotButtonPos);
    stub.reset(ADDR(ScrollScreenshot, clearPixmap));
    stub.reset(ADDR(MainWindow, initMainWindow));

    //delete MainWindow_m_scrollShotTip;
    //delete MainWindow_shotButton;
    delete MainWindow_scrollShotSizeTips;
//    delete MainWindow_toolBar;
//    delete MainWindow_previewWidget;
    delete MainWindow_m_scrollShot;

    delete window;


}

void initAttributes_stub()
{

}
void initLaunchMode_stub(const QString &launchMode)
{
    Q_UNUSED(launchMode);
}
void showFullScreen_stub()
{

}
void initResource_stub()
{

}
void shotFullScreen_stub(bool flag)
{
    Q_UNUSED(flag);
}
bool saveAction_stub(const QPixmap &pix)
{
    Q_UNUSED(pix);
    return true;
}
void sendNotify_stub(SaveAction saveAction, QString saveFilePath, const bool succeed)
{
    Q_UNUSED(saveAction);
    Q_UNUSED(saveFilePath);
    Q_UNUSED(succeed);

}
TEST_F(MainWindowTest, fullScreenshot)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, initAttributes), initAttributes_stub);
    stub.set(ADDR(MainWindow, initLaunchMode), initLaunchMode_stub);
    stub.set(ADDR(MainWindow, showFullScreen), showFullScreen_stub);
    stub.set(ADDR(MainWindow, initResource), initResource_stub);
    stub.set(ADDR(MainWindow, shotFullScreen), shotFullScreen_stub);
    stub.set(ADDR(MainWindow, saveAction), saveAction_stub);
    stub.set(ADDR(MainWindow, sendNotify), sendNotify_stub);
    MainWindow *window = new MainWindow();

    window->fullScreenshot();


    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, initAttributes));
    stub.reset(ADDR(MainWindow, initLaunchMode));
    stub.reset(ADDR(MainWindow, showFullScreen));
    stub.reset(ADDR(MainWindow, initResource));
    stub.reset(ADDR(MainWindow, shotFullScreen));
    stub.reset(ADDR(MainWindow, saveAction));
    stub.reset(ADDR(MainWindow, sendNotify));
    delete window;

}

ACCESS_PRIVATE_FIELD(MainWindow, QPixmap, m_backgroundPixmap);
TEST_F(MainWindowTest, topWindow)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, initAttributes), initAttributes_stub);
    stub.set(ADDR(MainWindow, initLaunchMode), initLaunchMode_stub);
    stub.set(ADDR(MainWindow, showFullScreen), showFullScreen_stub);
    stub.set(ADDR(MainWindow, initResource), initResource_stub);
    stub.set(ADDR(MainWindow, saveAction), saveAction_stub);
    stub.set(ADDR(MainWindow, sendNotify), sendNotify_stub);
    MainWindow *window = new MainWindow();
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    int &MainWindow_m_screenCount =  access_private_field::MainWindowm_screenCount(*window);
    MainWindow_m_screenCount = 1;

    QPixmap MainWindow_m_backgroundPixmap = access_private_field::MainWindowm_backgroundPixmap(*window);

    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 在多屏模式下, winId 不是0
    MainWindow_m_backgroundPixmap = t_primaryScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1920, 1080);

    window->topWindow();

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, initAttributes));
    stub.reset(ADDR(MainWindow, initLaunchMode));
    stub.reset(ADDR(MainWindow, showFullScreen));
    stub.reset(ADDR(MainWindow, initResource));
    stub.reset(ADDR(MainWindow, saveAction));
    stub.reset(ADDR(MainWindow, sendNotify));
    delete window;

}

ACCESS_PRIVATE_FIELD(MainWindow, QList<QRect>, windowRects);
ACCESS_PRIVATE_FIELD(MainWindow, QList<QString>, windowNames);
TEST_F(MainWindowTest, saveTopWindow)
{

    //stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, initAttributes), initAttributes_stub);
    stub.set(ADDR(MainWindow, initLaunchMode), initLaunchMode_stub);
    stub.set(ADDR(MainWindow, showFullScreen), showFullScreen_stub);
    stub.set(ADDR(MainWindow, initResource), initResource_stub);
    stub.set(ADDR(MainWindow, saveAction), saveAction_stub);
    stub.set(ADDR(MainWindow, sendNotify), sendNotify_stub);
    MainWindow *window = new MainWindow();
    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    int &MainWindow_m_screenCount =  access_private_field::MainWindowm_screenCount(*window);
    MainWindow_m_screenCount = 1;

    QPixmap MainWindow_m_backgroundPixmap = access_private_field::MainWindowm_backgroundPixmap(*window);

    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    // 在多屏模式下, winId 不是0
    MainWindow_m_backgroundPixmap = t_primaryScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1920, 1080);

    access_private_field::MainWindowwindowRects(*window).append(QRect(0, 0, 1000, 1000));
    access_private_field::MainWindowwindowRects(*window).append(QRect(0, 0, 500, 500));
    access_private_field::MainWindowwindowNames(*window).append("111");
    access_private_field::MainWindowwindowNames(*window).append("222");

    window->saveTopWindow();

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, initAttributes));
    stub.reset(ADDR(MainWindow, initLaunchMode));
    stub.reset(ADDR(MainWindow, showFullScreen));
    stub.reset(ADDR(MainWindow, initResource));
    stub.reset(ADDR(MainWindow, saveAction));
    stub.reset(ADDR(MainWindow, sendNotify));
    delete window;

}
//TEST_F(MainWindowTest, noNotify)
//{

//    MainWindow *window = new MainWindow();
//    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
//    stub.set(ADDR(MainWindow, initAttributes), initAttributes_stub);
//    stub.set(ADDR(MainWindow, initLaunchMode), initLaunchMode_stub);
//    stub.set(ADDR(MainWindow, showFullScreen), showFullScreen_stub);
//    stub.set(ADDR(MainWindow, initResource), initResource_stub);

//    window->noNotify();

//    stub.reset(ADDR(MainWindow, initMainWindow));
//    stub.reset(ADDR(MainWindow, initAttributes));
//    stub.reset(ADDR(MainWindow, initLaunchMode));
//    stub.reset(ADDR(MainWindow, showFullScreen));
//    stub.reset(ADDR(MainWindow, initResource));
//    delete window;

//}
bool isWaylandMode_stub()
{
    return true;
}
ACCESS_PRIVATE_FUN(MainWindow, void(QWheelEvent *), wheelEvent);
TEST_F(MainWindowTest, wheelEvent)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_scrollShot(*window) = nullptr;
    QWheelEvent *event ;
    Utils::isWaylandMode = true;
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    int &MainWindow_m_screenWidth =  access_private_field::MainWindowm_screenWidth(*window);
    MainWindow_m_screenWidth = 1920;
    int &MainWindow_m_screenHeight =  access_private_field::MainWindowm_screenHeight(*window);
    MainWindow_m_screenHeight = 1080;

    qreal &MainWindow_m_pixelRatio =  access_private_field::MainWindowm_pixelRatio(*window);
    MainWindow_m_pixelRatio = 1.0;

    call_private_fun::MainWindowwheelEvent(*window, event);

    Utils::isWaylandMode = false;
    delete window;
    stub.reset(ADDR(MainWindow, initMainWindow));

}

ACCESS_PRIVATE_FIELD(MainWindow, CameraWidget *, m_cameraWidget);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_selectedCamera);
ACCESS_PRIVATE_FIELD(MainWindow, int, m_shotflag);
int getRecordWidth_stub()
{
    return 500;
}
int getRecordHeight_stub()
{
    return 500;
}
CameraWidget::Position postion_stub()
{
    return CameraWidget::Position::leftBottom;
}
TEST_F(MainWindowTest, updateCameraWidgetPos)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, initAttributes), initAttributes_stub);
    stub.set(ADDR(MainWindow, initLaunchMode), initLaunchMode_stub);
    stub.set(ADDR(MainWindow, showFullScreen), showFullScreen_stub);
    stub.set(ADDR(MainWindow, initResource), initResource_stub);

    stub.set(ADDR(CameraWidget, getRecordWidth), getRecordWidth_stub);
    stub.set(ADDR(CameraWidget, getRecordHeight), getRecordHeight_stub);
    stub.set(ADDR(CameraWidget, postion), postion_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_cameraWidget(*window) = new CameraWidget();
    access_private_field::MainWindowm_selectedCamera(*window) = true;
    access_private_field::MainWindowm_shotflag(*window) = 2;
    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;
    window->updateCameraWidgetPos();
    stub.reset(ADDR(CameraWidget, getRecordWidth));
    stub.reset(ADDR(CameraWidget, getRecordHeight));
    stub.reset(ADDR(CameraWidget, postion));
    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, initAttributes));
    stub.reset(ADDR(MainWindow, initLaunchMode));
    stub.reset(ADDR(MainWindow, showFullScreen));
    stub.reset(ADDR(MainWindow, initResource));
    delete window;

}

void startCountdown_stub()
{

}
ACCESS_PRIVATE_FIELD(MainWindow, QSize, m_screenSize);
TEST_F(MainWindowTest, tableRecordSet)
{
    //stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, initAttributes), initAttributes_stub);
    stub.set(ADDR(MainWindow, initLaunchMode), initLaunchMode_stub);
    stub.set(ADDR(MainWindow, showFullScreen), showFullScreen_stub);
    stub.set(ADDR(MainWindow, initResource), initResource_stub);
    stub.set(ADDR(MainWindow, startCountdown), startCountdown_stub);

    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_screenSize(*window) = QSize(1920, 1080);

    window->tableRecordSet();

    stub.reset(ADDR(MainWindow, startCountdown));

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, initAttributes));
    stub.reset(ADDR(MainWindow, initLaunchMode));
    stub.reset(ADDR(MainWindow, showFullScreen));
    stub.reset(ADDR(MainWindow, initResource));
    delete window;

}
void showDragFeedback_stub(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}
ACCESS_PRIVATE_FIELD(MainWindow, unsigned int, m_functionType);
TEST_F(MainWindowTest, onMouseDrag)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, showDragFeedback), showDragFeedback_stub);

    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_functionType(*window) = MainWindow::status::record  ;

    window->onMouseDrag(10, 10);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, showDragFeedback));
    delete window;

}
void showPressFeedback_stub(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}
TEST_F(MainWindowTest, onMousePress1)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, showPressFeedback), showPressFeedback_stub);

    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_functionType(*window) = MainWindow::status::record  ;

    window->onMousePress(10, 10);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, showPressFeedback));
    delete window;

}
void scrollShotMouseClickEvent_stub(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}
TEST_F(MainWindowTest, onMousePress2)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, scrollShotMouseClickEvent), scrollShotMouseClickEvent_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_initScroll(*window) = true;
    access_private_field::MainWindowm_functionType(*window) = MainWindow::status::scrollshot  ;

    window->onMousePress(10, 10);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, scrollShotMouseClickEvent));
    delete window;

}
void showReleaseFeedback_stub(int x, int y)
{
    Q_UNUSED(x);
    Q_UNUSED(y);
}
void showContentButtons_stub(int s)
{
}
TEST_F(MainWindowTest, onMouseRelease)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, showReleaseFeedback), showReleaseFeedback_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_functionType(*window) = MainWindow::status::record  ;

    window->onMousePress(10, 10);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, showReleaseFeedback));
    delete window;

}
void responseEsc_stub()
{
}
ACCESS_PRIVATE_FIELD(MainWindow, int, recordButtonStatus);
ACCESS_PRIVATE_FIELD(MainWindow, ShowButtons *, m_showButtons);
TEST_F(MainWindowTest, onKeyboardPressWayland1)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, responseEsc), responseEsc_stub);
    stub.set((void(ShowButtons::*)(const int))ADDR(ShowButtons, showContentButtons), showContentButtons_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_functionType(*window) = MainWindow::status::record;
    access_private_field::MainWindowm_showButtons(*window) = new ShowButtons();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    window->onKeyboardPressWayland(Qt::Key_Escape);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, responseEsc));
    stub.reset((void(ShowButtons::*)(const int))ADDR(ShowButtons, showContentButtons));
    delete window;

}

void shapeClickedFromMain_stub(QString s)
{
}
TEST_F(MainWindowTest, onKeyboardPressWayland2)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(ToolBar, shapeClickedFromMain), shapeClickedFromMain_stub);
    stub.set((void(ShowButtons::*)(const int))ADDR(ShowButtons, showContentButtons), showContentButtons_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_functionType(*window) = MainWindow::status::record;
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;
    access_private_field::MainWindowm_showButtons(*window) = new ShowButtons();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();
    window->onKeyboardPressWayland(Qt::Key_S);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(ToolBar, shapeClickedFromMain));
    stub.reset((void(ShowButtons::*)(const int))ADDR(ShowButtons, showContentButtons));
    delete window;

}

TEST_F(MainWindowTest, onKeyboardPressWayland3)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(ToolBar, shapeClickedFromMain), shapeClickedFromMain_stub);
    stub.set((void(ShowButtons::*)(const int))ADDR(ShowButtons, showContentButtons), showContentButtons_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_functionType(*window) = MainWindow::status::record;
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;
    access_private_field::MainWindowm_showButtons(*window) = new ShowButtons();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();
    window->onKeyboardPressWayland(Qt::Key_M);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(ToolBar, shapeClickedFromMain));
    stub.reset((void(ShowButtons::*)(const int))ADDR(ShowButtons, showContentButtons));
    delete window;

}

TEST_F(MainWindowTest, onKeyboardPressWayland4)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(ToolBar, shapeClickedFromMain), shapeClickedFromMain_stub);
    stub.set((void(ShowButtons::*)(const int))ADDR(ShowButtons, showContentButtons), showContentButtons_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_functionType(*window) = MainWindow::status::record;
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;
    access_private_field::MainWindowm_showButtons(*window) = new ShowButtons();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();
    window->onKeyboardPressWayland(Qt::Key_F3);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(ToolBar, shapeClickedFromMain));
    stub.reset((void(ShowButtons::*)(const int))ADDR(ShowButtons, showContentButtons));
    delete window;

}
TEST_F(MainWindowTest, initPadShot)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, initAttributes), initAttributes_stub);
    stub.set(ADDR(MainWindow, initLaunchMode), initLaunchMode_stub);
    stub.set(ADDR(MainWindow, showFullScreen), showFullScreen_stub);
    stub.set(ADDR(MainWindow, initResource), initResource_stub);
    stub.set(ADDR(MainWindow, updateToolBarPos), updateToolBarPos_stub);

    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();

    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;

    window->initPadShot();

    stub.reset(ADDR(MainWindow, updateToolBarPos));
    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, initAttributes));
    stub.reset(ADDR(MainWindow, initLaunchMode));
    stub.reset(ADDR(MainWindow, showFullScreen));
    stub.reset(ADDR(MainWindow, initResource));
    delete window;

}


ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isShapesWidgetExist);
TEST_F(MainWindowTest, onViewShortcut)
{
    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(MainWindow, initAttributes), initAttributes_stub);
    stub.set(ADDR(MainWindow, initLaunchMode), initLaunchMode_stub);
    stub.set(ADDR(MainWindow, showFullScreen), showFullScreen_stub);
    stub.set(ADDR(MainWindow, initResource), initResource_stub);

    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();

    int &MainWindow_recordX =  access_private_field::MainWindowrecordX(*window);
    MainWindow_recordX = 0;
    int &MainWindow_recordY =  access_private_field::MainWindowrecordY(*window);
    MainWindow_recordY = 0;
    int &MainWindow_recordWidth =  access_private_field::MainWindowrecordWidth(*window);
    MainWindow_recordWidth = 1920;
    int &MainWindow_recordHeight =  access_private_field::MainWindowrecordHeight(*window);
    MainWindow_recordHeight = 1080;

    access_private_field::MainWindowm_isShapesWidgetExist(*window) = true;
    access_private_field::MainWindowm_shapesWidget(*window) = new ShapesWidget();

    window->onViewShortcut();

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(MainWindow, initAttributes));
    stub.reset(ADDR(MainWindow, initLaunchMode));
    stub.reset(ADDR(MainWindow, showFullScreen));
    stub.reset(ADDR(MainWindow, initResource));
    delete window;
}

TEST_F(MainWindowTest, shotKeyPressEvent)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(ToolBar, shapeClickedFromMain), shapeClickedFromMain_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();

    window->shotKeyPressEvent(KEY_F3);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(ToolBar, shapeClickedFromMain));
    delete window;

}


TEST_F(MainWindowTest, recordKeyPressEvent_Key_s)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(ToolBar, shapeClickedFromMain), shapeClickedFromMain_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    window->recordKeyPressEvent(KEY_S);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(ToolBar, shapeClickedFromMain));
    delete window;

}

TEST_F(MainWindowTest, recordKeyPressEvent_Key_m)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(ToolBar, shapeClickedFromMain), shapeClickedFromMain_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    window->recordKeyPressEvent(KEY_M);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(ToolBar, shapeClickedFromMain));
    delete window;

}

TEST_F(MainWindowTest, recordKeyPressEvent_Key_f3)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(ToolBar, shapeClickedFromMain), shapeClickedFromMain_stub);
    MainWindow *window = new MainWindow();
    access_private_field::MainWindowm_toolBar(*window) = new ToolBar();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    window->recordKeyPressEvent(KEY_F3);

    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(ToolBar, shapeClickedFromMain));
    delete window;

}

bool isSysHighVersion1040_stub()
{

    return true;
}
void startRecord_stub()
{
}
void m_pRecorderRegion_stub()
{
}
void whileCheckTempFileArm_stub()
{
}
ACCESS_PRIVATE_FIELD(MainWindow, RecorderRegionShow *, m_pRecorderRegion);
ACCESS_PRIVATE_FIELD(MainWindow, QThread *, m_connectionThread);
ACCESS_PRIVATE_FUN(MainWindow, void(), whileCheckTempFileArm);
TEST_F(MainWindowTest, startRecord)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    stub.set(ADDR(ToolBar, shapeClickedFromMain), shapeClickedFromMain_stub);
    stub.set(ADDR(Utils, isSysHighVersion1040), isSysHighVersion1040_stub);
    stub.set(ADDR(RecordProcess, startRecord), startRecord_stub);
    stub.set(ADDR(RecorderRegionShow, setCameraShow), m_pRecorderRegion_stub);
    auto MainWindow_whileCheckTempFileArm = get_private_fun::MainWindowwhileCheckTempFileArm();
    stub.set(MainWindow_whileCheckTempFileArm, whileCheckTempFileArm_stub);
    MainWindow *window = new MainWindow();

    Utils::isTabletEnvironment = false;

    access_private_field::MainWindowm_pRecorderRegion(*window) = new RecorderRegionShow();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    window->startRecord();

    stub.reset(MainWindow_whileCheckTempFileArm);
    stub.reset(ADDR(RecorderRegionShow, setCameraShow));
    stub.reset(ADDR(RecordProcess, startRecord));
    stub.reset(ADDR(Utils, isSysHighVersion1040));
    stub.reset(ADDR(MainWindow, initMainWindow));
    stub.reset(ADDR(ToolBar, shapeClickedFromMain));
    delete window;

}

ACCESS_PRIVATE_FIELD(MainWindow, ConnectionThread *, m_connectionThreadObject);
ACCESS_PRIVATE_FIELD(MainWindow, QList<MainWindow::ScreenInfo>, m_screenInfo);
ACCESS_PRIVATE_FIELD(MainWindow, bool, m_isVertical);
ACCESS_PRIVATE_FUN(MainWindow, void(const QVector<ClientManagement::WindowState> &), waylandwindowinfo);
TEST_F(MainWindowTest, waylandwindowinfo1)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();

    Utils::isTabletEnvironment = false;

    access_private_field::MainWindowm_pRecorderRegion(*window) = new RecorderRegionShow();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    access_private_field::MainWindowm_isVertical(*window) = false;
    MainWindow::ScreenInfo screenInfo1, screenInfo2;
    screenInfo1.x = 0;
    screenInfo1.y = 0;
    screenInfo1.width = 1920;
    screenInfo1.height = 1080;
    screenInfo1.name = "test1";
    screenInfo2.x = 10;
    screenInfo2.y = 0;
    screenInfo2.width = 1920;
    screenInfo2.height = 1080;
    screenInfo2.name = "test2";
    access_private_field::MainWindowm_screenInfo(*window).append(screenInfo1) ;
    access_private_field::MainWindowm_screenInfo(*window).append(screenInfo2) ;
    QVector<ClientManagement::WindowState> windowStates;
    ClientManagement::WindowState windowState1;
    windowState1.pid = 1;
    windowState1.windowId = 1;
    windowState1.resourceName[0] = 't';
    windowState1.resourceName[1] = 'e';
    windowState1.resourceName[2] = 's';
    windowState1.resourceName[3] = 't';
    windowState1.geometry.x = 0;
    windowState1.geometry.y = 0;
    windowState1.geometry.width = 500;
    windowState1.geometry.height = 600;
    windowState1.isMinimized = false;
    windowState1.isFullScreen = false;
    windowState1.isActive = false;
    windowStates.push_back(windowState1);
    access_private_field::MainWindowm_connectionThread(*window) = new QThread();
    access_private_field::MainWindowm_connectionThreadObject(*window) = new ConnectionThread();
    access_private_field::MainWindowm_screenSize(*window) = QSize(1920, 1080);
    call_private_fun::MainWindowwaylandwindowinfo(*window, windowStates);

    stub.reset(ADDR(MainWindow, initMainWindow));
    delete access_private_field::MainWindowm_connectionThread(*window);
    delete window;

}

TEST_F(MainWindowTest, waylandwindowinfo2)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();

    access_private_field::MainWindowm_pRecorderRegion(*window) = new RecorderRegionShow();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    access_private_field::MainWindowm_isVertical(*window) = false;
    MainWindow::ScreenInfo screenInfo1, screenInfo2;
    screenInfo1.x = 0;
    screenInfo1.y = 0;
    screenInfo1.width = 1920;
    screenInfo1.height = 1080;
    screenInfo1.name = "test1";
    screenInfo2.x = 0;
    screenInfo2.y = 0;
    screenInfo2.width = 1920;
    screenInfo2.height = 1080;
    screenInfo2.name = "test2";
    access_private_field::MainWindowm_screenInfo(*window).append(screenInfo1) ;
    access_private_field::MainWindowm_screenInfo(*window).append(screenInfo2) ;
    QVector<ClientManagement::WindowState> windowStates;
    ClientManagement::WindowState windowState1;
    windowState1.pid = 1;
    windowState1.windowId = 1;
    windowState1.resourceName[0] = 't';
    windowState1.resourceName[1] = 'e';
    windowState1.resourceName[2] = 's';
    windowState1.resourceName[3] = 't';
    windowState1.geometry.x = 0;
    windowState1.geometry.y = 0;
    windowState1.geometry.width = 500;
    windowState1.geometry.height = 600;
    windowState1.isMinimized = false;
    windowState1.isFullScreen = false;
    windowState1.isActive = false;
    windowStates.push_back(windowState1);
    access_private_field::MainWindowm_connectionThread(*window) = new QThread();
    access_private_field::MainWindowm_connectionThreadObject(*window) = new ConnectionThread();
    access_private_field::MainWindowm_screenSize(*window) = QSize(1920, 1080);
    call_private_fun::MainWindowwaylandwindowinfo(*window, windowStates);

    stub.reset(ADDR(MainWindow, initMainWindow));
    delete access_private_field::MainWindowm_connectionThread(*window);
    delete window;

}


TEST_F(MainWindowTest, waylandwindowinfo3)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();

    access_private_field::MainWindowm_pRecorderRegion(*window) = new RecorderRegionShow();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    access_private_field::MainWindowm_isVertical(*window) = true;
    MainWindow::ScreenInfo screenInfo1, screenInfo2;
    screenInfo1.x = 0;
    screenInfo1.y = 0;
    screenInfo1.width = 1920;
    screenInfo1.height = 1080;
    screenInfo1.name = "test1";
    screenInfo2.x = 10;
    screenInfo2.y = 0;
    screenInfo2.width = 1920;
    screenInfo2.height = 1080;
    screenInfo2.name = "test2";
    access_private_field::MainWindowm_screenInfo(*window).append(screenInfo1) ;
    access_private_field::MainWindowm_screenInfo(*window).append(screenInfo2) ;
    QVector<ClientManagement::WindowState> windowStates;
    ClientManagement::WindowState windowState1;
    windowState1.pid = 1;
    windowState1.windowId = 1;
    windowState1.resourceName[0] = 't';
    windowState1.resourceName[1] = 'e';
    windowState1.resourceName[2] = 's';
    windowState1.resourceName[3] = 't';
    windowState1.geometry.x = 0;
    windowState1.geometry.y = 0;
    windowState1.geometry.width = 500;
    windowState1.geometry.height = 600;
    windowState1.isMinimized = false;
    windowState1.isFullScreen = false;
    windowState1.isActive = false;
    windowStates.push_back(windowState1);
    access_private_field::MainWindowm_connectionThread(*window) = new QThread();
    access_private_field::MainWindowm_connectionThreadObject(*window) = new ConnectionThread();
    access_private_field::MainWindowm_screenSize(*window) = QSize(1920, 1080);
    call_private_fun::MainWindowwaylandwindowinfo(*window, windowStates);

    stub.reset(ADDR(MainWindow, initMainWindow));
    delete access_private_field::MainWindowm_connectionThread(*window);
    delete window;

}

TEST_F(MainWindowTest, waylandwindowinfo4)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();


    access_private_field::MainWindowm_pRecorderRegion(*window) = new RecorderRegionShow();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    access_private_field::MainWindowm_isVertical(*window) = true;
    MainWindow::ScreenInfo screenInfo1, screenInfo2;
    screenInfo1.x = 0;
    screenInfo1.y = 10;
    screenInfo1.width = 1920;
    screenInfo1.height = 1080;
    screenInfo1.name = "test1";
    screenInfo2.x = 10;
    screenInfo2.y = 0;
    screenInfo2.width = 1920;
    screenInfo2.height = 1080;
    screenInfo2.name = "test2";
    access_private_field::MainWindowm_screenInfo(*window).append(screenInfo1) ;
    access_private_field::MainWindowm_screenInfo(*window).append(screenInfo2) ;
    QVector<ClientManagement::WindowState> windowStates;
    ClientManagement::WindowState windowState1;
    windowState1.pid = 1;
    windowState1.windowId = 1;
    windowState1.resourceName[0] = 't';
    windowState1.resourceName[1] = 'e';
    windowState1.resourceName[2] = 's';
    windowState1.resourceName[3] = 't';
    windowState1.geometry.x = 0;
    windowState1.geometry.y = 0;
    windowState1.geometry.width = 500;
    windowState1.geometry.height = 600;
    windowState1.isMinimized = false;
    windowState1.isFullScreen = false;
    windowState1.isActive = false;
    windowStates.push_back(windowState1);
    access_private_field::MainWindowm_connectionThread(*window) = new QThread();
    access_private_field::MainWindowm_connectionThreadObject(*window) = new ConnectionThread();
    access_private_field::MainWindowm_screenSize(*window) = QSize(1920, 1080);
    call_private_fun::MainWindowwaylandwindowinfo(*window, windowStates);

    stub.reset(ADDR(MainWindow, initMainWindow));
    delete access_private_field::MainWindowm_connectionThread(*window);
    delete window;

}

TEST_F(MainWindowTest, waylandwindowinfo5)
{

    stub.set(ADDR(MainWindow, initMainWindow), initMainWindow_stub);
    MainWindow *window = new MainWindow();


    access_private_field::MainWindowm_pRecorderRegion(*window) = new RecorderRegionShow();
    access_private_field::MainWindowrecordButtonStatus(*window) = 0;

    access_private_field::MainWindowm_isVertical(*window) = true;
    MainWindow::ScreenInfo screenInfo1, screenInfo2;
    screenInfo1.x = 0;
    screenInfo1.y = 10;
    screenInfo1.width = 1920;
    screenInfo1.height = 1080;
    screenInfo1.name = "test1";
    access_private_field::MainWindowm_screenInfo(*window).append(screenInfo1) ;
    QVector<ClientManagement::WindowState> windowStates;
    ClientManagement::WindowState windowState1;
    windowState1.pid = 1;
    windowState1.windowId = 1;
    windowState1.resourceName[0] = 't';
    windowState1.resourceName[1] = 'e';
    windowState1.resourceName[2] = 's';
    windowState1.resourceName[3] = 't';
    windowState1.geometry.x = 0;
    windowState1.geometry.y = 0;
    windowState1.geometry.width = 500;
    windowState1.geometry.height = 600;
    windowState1.isMinimized = false;
    windowState1.isFullScreen = false;
    windowState1.isActive = false;
    windowStates.push_back(windowState1);
    access_private_field::MainWindowm_connectionThread(*window) = new QThread();
    access_private_field::MainWindowm_connectionThreadObject(*window) = new ConnectionThread();
    access_private_field::MainWindowm_screenSize(*window) = QSize(1920, 1080);
    call_private_fun::MainWindowwaylandwindowinfo(*window, windowStates);

    stub.reset(ADDR(MainWindow, initMainWindow));
    delete access_private_field::MainWindowm_connectionThread(*window);
    delete window;

}
