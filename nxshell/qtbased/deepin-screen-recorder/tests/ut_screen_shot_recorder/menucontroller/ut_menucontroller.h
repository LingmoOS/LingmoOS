// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <DMenu>
#include <QAction>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include "stub.h"
#include "addr_pri.h"
#include "../../src/menucontroller/menucontroller.h"


using namespace testing;
class MenuControllerTest: public testing::Test, public QObject
{

public:
    MenuController *m_menuController;
    QString curAct = "";
    Stub stub;
    virtual void SetUp() override
    {
        std::cout << "start MenuControllerTest" << std::endl;
        m_menuController = new MenuController(nullptr);
        connect(m_menuController, &MenuController::saveAction, this, &MenuControllerTest::saveScreenShot);
        connect(m_menuController, &MenuController::closeAction, this, &MenuControllerTest::exitApp);
    }

    virtual void TearDown() override
    {
        delete  m_menuController;
        std::cout << "end MenuControllerTest" << std::endl;
    }
public slots:
    void exitApp();
    void saveScreenShot();
};
void popup_stub(const QPoint &pos, QAction *at = nullptr)
{
    Q_UNUSED(pos);
    Q_UNUSED(at);
}
TEST_F(MenuControllerTest, showMenu)
{
    stub.set(ADDR(DMenu, popup), popup_stub);
    m_menuController->showMenu(QPoint(100, 100));
//    //m_menuController->enterEvent(nullptr);
//    m_menuController->setUndoEnable(true);
//    QEventLoop loop;
//    QTimer::singleShot(3000, &loop, SLOT(quit()));
//    loop.exec();
    stub.reset(ADDR(DMenu, popup));
    qDebug() << curAct;
}
void MenuControllerTest::saveScreenShot()
{
    EXPECT_EQ(1, 1);
}
void MenuControllerTest::exitApp()
{
    EXPECT_EQ(1, 1);
}

