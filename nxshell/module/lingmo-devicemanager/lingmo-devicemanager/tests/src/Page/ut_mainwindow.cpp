// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "PageInfo.h"
#include "MainWindow.h"
#include "DeviceInput.h"
#include "DeviceInfo.h"
#include "DeviceWidget.h"
#include "PageListView.h"
#include "DeviceListView.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QFileDialog>
#include <QProcess>
#include <QJsonArray>
#include <QJsonDocument>
#include <QIODevice>
#include <QJsonObject>

#include <gtest/gtest.h>

void ut_refreshDataBase()
{
    return;
}

class MainWindow_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        stub.set(ADDR(MainWindow, refreshDataBase), ut_refreshDataBase);
        m_mainWindow = new MainWindow;
    }
    void TearDown()
    {
        delete m_mainWindow;
    }
    MainWindow *m_mainWindow = nullptr;
    Stub stub;
};

TEST_F(MainWindow_UT, MainWindow_UT_refresh)
{
    m_mainWindow->m_refreshing = false;
    m_mainWindow->refresh();
    EXPECT_TRUE(m_mainWindow->m_refreshing);
    m_mainWindow->slotRefreshInfo();
    EXPECT_TRUE(m_mainWindow->m_refreshing);
}

QString UT_getSaveFileName()
{
    return "1.txt";
}

TEST_F(MainWindow_UT, MainWindow_UT_exportTo)
{
    stub.set(ADDR(QFileDialog, getSaveFileName), UT_getSaveFileName);

    EXPECT_FALSE(m_mainWindow->exportTo());
    m_mainWindow->slotExportInfo();
}

TEST_F(MainWindow_UT, MainWindow_UT_addJsonArrayItem)
{
    QJsonArray array;
    array.insert(0, QJsonValue("/"));
    m_mainWindow->addJsonArrayItem(array, "test", "/");
    EXPECT_EQ(2, array.size());
}

TEST_F(MainWindow_UT, MainWindow_UT_getJsonDoc)
{
    QJsonDocument doc;
    QJsonArray array;
    array.insert(0, QJsonValue("/"));
    doc.setArray(array);
    m_mainWindow->getJsonDoc(doc);
    EXPECT_TRUE(doc.object().keys().size() == 1);
    m_mainWindow->windowMaximizing();
    EXPECT_TRUE(m_mainWindow->isMaximized());
}

TEST_F(MainWindow_UT, MainWindow_UT_resizeEvent)
{
    QResizeEvent resizeevent(QSize(10, 10), QSize(10, 10));
    m_mainWindow->resizeEvent(&resizeevent);
}

TEST_F(MainWindow_UT, MainWindow_UT_slotListItemClicked)
{
    m_mainWindow->slotListItemClicked("Monitor");
    m_mainWindow->slotChangeUI();
}

TEST_F(MainWindow_UT, MainWindow_UT_keyPressEvent)
{
    QKeyEvent keyPressEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    QCoreApplication::sendEvent(m_mainWindow, &keyPressEvent);
}

TEST_F(MainWindow_UT, MainWindow_UT_event)
{
    QEvent event(QEvent::ApplicationFontChange);
    QCoreApplication::sendEvent(m_mainWindow, &event);
}

TEST_F(MainWindow_UT, MainWindow_UT_initWindow)
{
    m_mainWindow->initWindow();
    EXPECT_FALSE(m_mainWindow->mp_MainStackWidget->isVisible());
}

TEST_F(MainWindow_UT, MainWindow_UT_loadingFinishSlot)
{
    m_mainWindow->slotLoadingFinish("finish");
    EXPECT_TRUE(m_mainWindow->mp_DeviceWidget->mp_ListView->mp_ListView->mp_ItemModel->rowCount() > 0);
}
