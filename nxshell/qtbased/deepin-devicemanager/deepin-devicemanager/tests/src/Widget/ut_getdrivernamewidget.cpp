// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "GetDriverNameWidget.h"
#include "GetDriverNameModel.h"
#include "ut_Head.h"
#include "stub.h"

#include <DStackedWidget>
#include <DLabel>

#include <QDir>
#include <QFileIconProvider>

#include <gtest/gtest.h>


class UT_GetDriverNameWidget : public UT_HEAD
{
public:
    void SetUp()
    {
        m_GetDriverNameWidget = new GetDriverNameWidget;
    }
    void TearDown()
    {
        delete m_GetDriverNameWidget;
    }

    GetDriverNameWidget *m_GetDriverNameWidget;
};

TEST_F(UT_GetDriverNameWidget, UT_GetDriverNameWidget_loadAllDrivers)
{
    m_GetDriverNameWidget->loadAllDrivers(true, "/home");
    EXPECT_EQ(0, m_GetDriverNameWidget->mp_StackWidget->currentIndex());
}

TEST_F(UT_GetDriverNameWidget, UT_GetDriverNameWidget_selectName_001)
{
    m_GetDriverNameWidget->mp_selectedRow = -1;
    QString ret = m_GetDriverNameWidget->selectName();
    EXPECT_STREQ("", ret.toStdString().c_str());
}

TEST_F(UT_GetDriverNameWidget, UT_GetDriverNameWidget_selectName_002)
{
    m_GetDriverNameWidget->mp_selectedRow = 0;

    QStandardItem *icomItem = new QStandardItem;
    QString path = "/data/home/jixiaomei/.local/share/Trash/files/deepin-devicemanager_5.6.12.13-1_arm64.deb";
    QFileInfo info(path);
    QFileIconProvider icon_provider;
    QIcon icon = icon_provider.icon(info);
    icomItem->setData(icon, Qt::DecorationRole);
    icomItem->setData(QVariant::fromValue(path), Qt::UserRole);
    QStandardItem *textItem = new QStandardItem("deepin-devicemanager_5.6.12.13-1_arm64.deb");
    m_GetDriverNameWidget->mp_model = new QStandardItemModel();
    m_GetDriverNameWidget->mp_model->setItem(0, 0, icomItem);
    m_GetDriverNameWidget->mp_model->setItem(0, 1, textItem);

    QString ret = m_GetDriverNameWidget->selectName();
    EXPECT_STREQ(path.toStdString().c_str(), ret.toStdString().c_str());

    delete icomItem;
    delete textItem;
    delete m_GetDriverNameWidget->mp_model;
}

TEST_F(UT_GetDriverNameWidget, UT_GetDriverNameWidget_updateTipLabelText)
{
    m_GetDriverNameWidget->updateTipLabelText("/home");
    EXPECT_STREQ("/home", m_GetDriverNameWidget->mp_tipLabel->text().toStdString().c_str());
    EXPECT_STREQ("/home", m_GetDriverNameWidget->mp_tipLabel->toolTip().toStdString().c_str());
}

TEST_F(UT_GetDriverNameWidget, UT_GetDriverNameWidget_stopLoadingDrivers)
{
    m_GetDriverNameWidget->stopLoadingDrivers();
    EXPECT_TRUE(m_GetDriverNameWidget->mp_GetModel->m_Stop);
}

TEST_F(UT_GetDriverNameWidget, UT_GetDriverNameWidget_reloadDriversListPages_001)
{
    m_GetDriverNameWidget->mp_model = new QStandardItemModel();
    m_GetDriverNameWidget->reloadDriversListPages();

    EXPECT_EQ(-1, m_GetDriverNameWidget->mp_selectedRow);

    delete m_GetDriverNameWidget->mp_model;
}

TEST_F(UT_GetDriverNameWidget, UT_GetDriverNameWidget_reloadDriversListPages_002)
{
    QStandardItem *icomItem = new QStandardItem;
    QString path = "/data/home/jixiaomei/.local/share/Trash/files/deepin-devicemanager_5.6.12.13-1_arm64.deb";
    QFileInfo info(path);
    QFileIconProvider icon_provider;
    QIcon icon = icon_provider.icon(info);
    icomItem->setData(icon, Qt::DecorationRole);
    icomItem->setData(QVariant::fromValue(path), Qt::UserRole);
    QStandardItem *textItem = new QStandardItem("deepin-devicemanager_5.6.12.13-1_arm64.deb");
    m_GetDriverNameWidget->mp_model = new QStandardItemModel();
    m_GetDriverNameWidget->mp_model->setItem(0, 0, icomItem);
    m_GetDriverNameWidget->mp_model->setItem(0, 1, textItem);
    m_GetDriverNameWidget->reloadDriversListPages();

    EXPECT_EQ(-1, m_GetDriverNameWidget->mp_selectedRow);

    delete icomItem;
    delete textItem;
    delete m_GetDriverNameWidget->mp_model;
}

TEST_F(UT_GetDriverNameWidget, UT_GetDriverNameWidget_slotSelectedDriver)
{
    QModelIndex index;
    QStandardItem *icomItem = new QStandardItem;
    QString path = "/data/home/jixiaomei/.local/share/Trash/files/deepin-devicemanager_5.6.12.13-1_arm64.deb";
    QFileInfo info(path);
    QFileIconProvider icon_provider;
    QIcon icon = icon_provider.icon(info);
    icomItem->setData(icon, Qt::DecorationRole);
    icomItem->setData(QVariant::fromValue(path), Qt::UserRole);
    QStandardItem *textItem = new QStandardItem("deepin-devicemanager_5.6.12.13-1_arm64.deb");
    m_GetDriverNameWidget->mp_model = new QStandardItemModel();
    m_GetDriverNameWidget->mp_model->setItem(0, 0, icomItem);
    m_GetDriverNameWidget->mp_model->setItem(0, 1, textItem);

    m_GetDriverNameWidget->slotSelectedDriver(index);
    EXPECT_EQ(-1, m_GetDriverNameWidget->mp_selectedRow);

    delete icomItem;
    delete textItem;
    delete m_GetDriverNameWidget->mp_model;
}

TEST_F(UT_GetDriverNameWidget, UT_GetDriverNameWidget_slotFinishLoadDrivers)
{
    m_GetDriverNameWidget->mp_model = new QStandardItemModel();

    m_GetDriverNameWidget->slotFinishLoadDrivers();
    EXPECT_EQ(1, m_GetDriverNameWidget->mp_StackWidget->currentIndex());

    delete m_GetDriverNameWidget->mp_model;
}
