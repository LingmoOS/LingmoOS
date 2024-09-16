// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "window/availwidget.h"
#include "widgets/settingsgroup.h"
#include "widgets/settingsitem.h"
class ut_availwidget : public ::testing::Test
{
protected:
    ut_availwidget()
    {
    }

    virtual ~ut_availwidget()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_availwidget, clearItemStatus)
{
    AvailWidget availWidget;
    availWidget.clearItemStatus();
}

TEST_F(ut_availwidget, languageName)
{
    languageName("");
    languageName("*");
    languageName("111");
    languageName("zh_CH");
    EXPECT_TRUE(true);
}

TEST_F(ut_availwidget, getSeleteIm)
{
//    AvailWidget availWidget;
//    FcitxQtInputMethodItemList newItemList;
//    FcitxQtInputMethodItem item;
//    item.setName("name");
//    item.setUniqueName("name");
//    item.setLangCode("name");
//    item.setEnabled(false);
//    newItemList << item;
//    availWidget.onUpdateUI(newItemList);
//    item = availWidget.getSeleteIm();
}

TEST_F(ut_availwidget, onSearchIM)
{
    AvailWidget availWidget;
    availWidget.onSearchIM("fcitx-pinyin");
    availWidget.onSearchIM("fcitx-keyboard-us");
    availWidget.onSearchIM("fcitx-keyboard-cn");
}

TEST_F(ut_availwidget, onUpdateUI)
{
    AvailWidget availWidget;
    FcitxQtInputMethodItemList oldItemList;
    availWidget.onUpdateUI(oldItemList);

    FcitxQtInputMethodItemList newItemList;
    FcitxQtInputMethodItem item;
    item.setName("name");
    item.setUniqueName("name");
    item.setLangCode("name");
    item.setEnabled(false);
    newItemList << item;
    availWidget.onUpdateUI(newItemList);
    availWidget.onUpdateUI();
}

TEST_F(ut_availwidget, removeSeleteIm)
{
    AvailWidget availWidget;

    FcitxQtInputMethodItem item;
    item.setName("name");
    item.setUniqueName("name");
    item.setLangCode("name");
    item.setEnabled(false);
    availWidget.removeSeleteIm(item);
}
TEST_F(ut_availwidget, addSeleteIm)
{
    AvailWidget availWidget;

    FcitxQtInputMethodItem item;
    item.setName("name");
    item.setUniqueName("name");
    item.setLangCode("name");
    item.setEnabled(false);
    availWidget.addSeleteIm(item);
}
TEST_F(ut_availwidget, seleteIM)
{
    AvailWidget availWidget;
    availWidget.seleteIM(true);
    availWidget.seleteIM(false);
}
TEST_F(ut_availwidget, initUI)
{
    AvailWidget availWidget;
}

TEST_F(ut_availwidget, initConnect)
{
    AvailWidget availWidget;
    availWidget.initConnect();
}

TEST_F(ut_availwidget, clearItemStatusAndFilter)
{
    AvailWidget availWidget;

    dcc_fcitx_configtool::widgets::FcitxSettingsGroup group;
    dcc_fcitx_configtool::widgets::FcitxSettingsItem item;
    group.appendItem(&item);
    availWidget.clearItemStatusAndFilter(&group);
}


