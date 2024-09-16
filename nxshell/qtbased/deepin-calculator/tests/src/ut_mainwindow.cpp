// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_mainwindow.h"

#include "../../src/mainwindow.h"

#include <QPalette>
#include <DTitlebar>

TestCala::TestCala()
{

}

TEST_F(TestCala, initTheme)
{
    MainWindow *m_mainwindow = new MainWindow;
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::UnknownType);
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_mainwindow->initTheme();
    EXPECT_EQ(m_mainwindow->titlebar()->palette().color(DPalette::Light), QColor(240, 240, 240));
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_mainwindow->initTheme();
    QColor color(0, 0, 0);
    color.setAlphaF(0.1);
    EXPECT_EQ(m_mainwindow->titlebar()->palette().color(DPalette::Light), color);
    m_mainwindow->deleteLater();
}

TEST_F(TestCala, initModule)
{
    MainWindow *m_mainwindow = new MainWindow;
    m_mainwindow->m_settings->setOption("mode", 2);
    m_mainwindow->initModule();
    EXPECT_EQ(m_mainwindow->m_mainLayout->currentWidget(), m_mainwindow->m_programmerModule);
    m_mainwindow->m_settings->setOption("mode", 0);
    m_mainwindow->initModule();
    EXPECT_EQ(m_mainwindow->m_mainLayout->currentWidget(), m_mainwindow->m_basicModule);
    m_mainwindow->m_settings->setOption("mode", 1);
    m_mainwindow->initModule();
    EXPECT_EQ(m_mainwindow->m_mainLayout->currentWidget(), m_mainwindow->m_scientificModule);
    EXPECT_FALSE(m_mainwindow->m_isinit);
    m_mainwindow->deleteLater();
}

TEST_F(TestCala, switchToSimpleMode)
{
    MainWindow *m_mainwindow = new MainWindow;
    m_mainwindow->m_settings->setOption("mode", 1);
    m_mainwindow->switchToSimpleMode();
    EXPECT_EQ(m_mainwindow->m_settings->getOption("mode"), 0);
    EXPECT_EQ(m_mainwindow->m_mainLayout->currentWidget(), m_mainwindow->m_basicModule);
    m_mainwindow->deleteLater();
}

TEST_F(TestCala, switchToScientificMode)
{
    MainWindow *m_mainwindow = new MainWindow;
    m_mainwindow->m_settings->setOption("mode", 0);
    m_mainwindow->switchToScientificMode();
    EXPECT_EQ(m_mainwindow->m_settings->getOption("mode"), 1);
    EXPECT_EQ(m_mainwindow->m_mainLayout->currentWidget(), m_mainwindow->m_scientificModule);
    m_mainwindow->deleteLater();
}

TEST_F(TestCala, keyPressEvent)
{
    MainWindow *m_mainwindow = new MainWindow;
    m_mainwindow->m_settings->setOption("mode", 0);
    m_mainwindow->initModule();
    QKeyEvent *k = new QKeyEvent(QEvent::KeyPress, Qt::Key_1, Qt::NoModifier);
    m_mainwindow->keyPressEvent(k);
    EXPECT_EQ(m_mainwindow->m_basicModule->m_expressionBar->getInputEdit()->text(), "1");
    delete k;
    m_mainwindow->deleteLater();
}

TEST_F(TestCala, moveEvent)
{
    MainWindow *m_mainwindow = new MainWindow;
    QMoveEvent *m = new QMoveEvent(QPoint(1, 1), QPoint(0, 0));
    m_mainwindow->moveEvent(m);
    EXPECT_EQ(m_mainwindow->m_settings->getOption("windowX"), 1);
    delete m;
    m_mainwindow->deleteLater();
}

TEST_F(TestCala, resizeEvent)
{
    MainWindow *m_mainwindow = new MainWindow;
    m_mainwindow->switchToScientificMode();
    QResizeEvent *r = new QResizeEvent(QSize(811, 542), QSize(451, 542));
    m_mainwindow->resizeEvent(r);
    EXPECT_EQ(m_mainwindow->m_settings->getOption("windowWidth"), 811);
    delete r;
    m_mainwindow->deleteLater();
}

