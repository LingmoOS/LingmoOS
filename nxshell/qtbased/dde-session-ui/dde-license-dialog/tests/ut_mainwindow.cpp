// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "content.h"
#include "mainwindow.h"
#undef private

#include <DSuggestButton>

#include <gtest/gtest.h>

class UT_MainWindow : public testing::Test
{

};

TEST_F(UT_MainWindow, coverageTest)
{
   MainWindow window;
   window.setTitle("SetTitle");
   EXPECT_EQ(window.m_cnTitle, "SetTitle");
   window.setEnTitle("SetEnTitle");
   EXPECT_EQ(window.m_enTitle, "SetEnTitle");
   window.setAllowCheckBoxText("SetAllowCheckBoxText");
   EXPECT_EQ(window.m_content->m_allow, "SetAllowCheckBoxText");
   window.setEnAllowCheckBoxText("SetEnAllowCheckBoxText");
   EXPECT_EQ(window.m_content->m_enallow, "SetEnAllowCheckBoxText");
   window.setCnSource("/usr/share/deepin-deepinid-client/privacy/deepinid-CN-zh_CN.md");
   EXPECT_EQ(window.m_content->m_cn, "/usr/share/deepin-deepinid-client/privacy/deepinid-CN-zh_CN.md");
   window.setEnSource("/usr/share/deepin-deepinid-client/privacy/deepinid-EN-us_EN.md");
   EXPECT_EQ(window.m_content->m_en, "/usr/share/deepin-deepinid-client/privacy/deepinid-EN-us_EN.md");
   window.m_content->m_hasCn = true;
   window.m_content->m_hasEn = true;
   window.updateLocaleSource();
   Q_EMIT window.m_content->sourceChanged(false);
   if (QLocale::system().language() == QLocale::Chinese) {
        EXPECT_TRUE(window.m_content->m_isCn);
        EXPECT_TRUE(window.m_content->m_languageBtn->button(1)->isChecked());
   } else {
        EXPECT_FALSE(window.m_content->m_isCn);
        EXPECT_TRUE(window.m_content->m_languageBtn->button(0)->isChecked());
   }
   window.setHideBottom("yes");
}
