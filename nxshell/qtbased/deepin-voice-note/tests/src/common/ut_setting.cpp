// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_setting.h"
#include "setting.h"

UT_Setting::UT_Setting()
{
}

TEST_F(UT_Setting, UT_Setting_keys_001)
{
    CustemBackend custembackend("/home/zhangteng/.config/deepin/deepin-voice-note/config.conf");
    QStringList keyList;
    EXPECT_EQ(keyList.size(), 0);
    keyList = custembackend.keys();
    EXPECT_GE(keyList.size(), 0);
}

TEST_F(UT_Setting, UT_Setting_setOption_001)
{
    QString key = "old._app_main_wnd_sz_key_";
    QString value = "/home/zhangteng/test";
    setting::instance()->setOption(key, value);
    EXPECT_EQ("/home/zhangteng/test", setting::instance()->getOption(key));
    EXPECT_FALSE(key.isEmpty());
    QStringList keyList = setting::instance()->m_setting->keys();
    EXPECT_TRUE(keyList.contains(key));
}

TEST_F(UT_Setting, UT_Setting_doSetOption_001)
{
    CustemBackend custembackend("/home/zhangteng/.config/deepin/deepin-voice-note/config.conf");
    QString key = "old._app_main_wnd_sz_key_";
    QString value = "/home/zhangteng/test";
    custembackend.doSetOption(key, value);
    EXPECT_EQ("/home/zhangteng/test", custembackend.getOption("_app_main_wnd_sz_key_"));
    key = "_app_main_wnd_sz_key_";
    custembackend.doSetOption(key, value);
    EXPECT_EQ("/home/zhangteng/test", custembackend.getOption("_app_main_wnd_sz_key_"));
}

TEST_F(UT_Setting, UT_Setting_getSetting_001)
{
    DSettings *tmpset = setting::instance()->getSetting();
    EXPECT_TRUE(tmpset != nullptr);
}

TEST_F(UT_Setting, UT_Setting_GenerateSettingTranslate_001)
{
    extern void GenerateSettingTranslate();
    GenerateSettingTranslate();
}

TEST_F(UT_Setting, UT_Setting_doSync_001)
{
    CustemBackend custembackend("/home/zhangteng/.config/deepin/deepin-voice-note/config.conf");
    custembackend.doSync();
}
