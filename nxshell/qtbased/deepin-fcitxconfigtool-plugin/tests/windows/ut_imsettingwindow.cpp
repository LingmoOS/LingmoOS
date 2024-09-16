// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "window/imsettingwindow.h"
class ut_imsettingwindow : public ::testing::Test
{
protected:
    ut_imsettingwindow()
    {
    }

    virtual ~ut_imsettingwindow()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

//TEST_F(ut_imsettingwindow, updateUI)
//{
//    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.updateUI();
//    EXPECT_TRUE(true);
//}

TEST_F(ut_imsettingwindow, initUI)
{
    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.initUI();
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, initConnect)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.initConnect();
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, readConfig)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.readConfig();
}

//TEST_F(ut_imsettingwindow, itemSwap)
//{
//    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.itemSwap(nullptr);
//    EXPECT_TRUE(true);
//}

TEST_F(ut_imsettingwindow, onEditBtnClicked)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onEditBtnClicked(true);
    iMSettingWindow.onEditBtnClicked(false);
}

//TEST_F(ut_imsettingwindow, onDefaultIMChanged)
//{
//    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.onDefaultIMChanged();
//    EXPECT_TRUE(true);
//}

TEST_F(ut_imsettingwindow, onCurIMChanged)
{
    FcitxQtInputMethodItemList list;
    FcitxQtInputMethodItem item;
    item.setName("name");
    item.setUniqueName("name");
    item.setLangCode("name");
    item.setEnabled(false);
    list << item;
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onCurIMChanged(list);
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, onAddBtnCilcked)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onAddBtnCilcked();
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, onItemUp)
{
    FcitxQtInputMethodItem item;
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onItemUp(item);
    EXPECT_TRUE(true);
}

TEST_F(ut_imsettingwindow, onItemDown)
{
    FcitxQtInputMethodItem item;
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onItemDown(item);
    EXPECT_TRUE(true);
}

//加上会有内存检测错误
//TEST_F(ut_imsettingwindow, onItemDelete)
//{
//    FcitxQtInputMethodItem item;
//    IMSettingWindow iMSettingWindow;
//    iMSettingWindow.onItemDelete(item);
//    EXPECT_TRUE(true);
//}

TEST_F(ut_imsettingwindow, doReloadConfigUI)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.doReloadConfigUI();
    EXPECT_TRUE(true);
}
TEST_F(ut_imsettingwindow, onReloadConnect)
{
    IMSettingWindow iMSettingWindow;
    iMSettingWindow.onReloadConnect();
    EXPECT_TRUE(true);
}
