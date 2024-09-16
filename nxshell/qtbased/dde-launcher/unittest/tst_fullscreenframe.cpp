// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "fullscreenframe.h"
#include "../src/model/appslistmodel.h"

class Tst_Fullscreenframe : public testing::Test
{
public:
    void SetUp() override
    {
        m_fullScreenFrame = new FullScreenFrame(nullptr);
    }

    void TearDown() override
    {
        delete m_fullScreenFrame;
        m_fullScreenFrame = nullptr;
    }

public:
    FullScreenFrame* m_fullScreenFrame;
};

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollCurrent_test)
{
    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::Chat, AppsListModel::AppCategory::Chat);
}

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollPrev_test)
{
    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::Chat, AppsListModel::AppCategory::Internet);
}

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollPrev1_test)
{
    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::Video, AppsListModel::AppCategory::Internet);
}

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollNext_test)
{
    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::Internet, AppsListModel::AppCategory::Chat);
}

TEST_F(Tst_Fullscreenframe, scrollToCategory_scrollNext1_test)
{
    m_fullScreenFrame->scrollToCategory(AppsListModel::AppCategory::All, AppsListModel::AppCategory::Chat);
}

TEST_F(Tst_Fullscreenframe, scrollBlurBoxWidget_test)
{
    BlurBoxWidget* blueBox = new BlurBoxWidget(AppsListModel::AppCategory::Internet, nullptr);
    ScrollWidgetAgent* scrollWidgetAgent = new ScrollWidgetAgent;
    scrollWidgetAgent->setPosType(Pos_M);
    scrollWidgetAgent->setBlurBoxWidget(blueBox);

    m_fullScreenFrame->scrollBlurBoxWidget(scrollWidgetAgent);
}

TEST_F(Tst_Fullscreenframe, tips_test)
{
    for (int type = 0; type < 2; type++) {
         m_fullScreenFrame->updateDisplayMode(type);
         m_fullScreenFrame->showTips("");
         m_fullScreenFrame->hideTips();
    }
}

TEST_F(Tst_Fullscreenframe, backtab_nextTabWidget_test)
{
    m_fullScreenFrame->updateDisplayMode(GROUP_BY_CATEGORY);
    m_fullScreenFrame->nextTabWidget(Qt::Key_Backtab);

    m_fullScreenFrame->updateDisplayMode(SEARCH);
    m_fullScreenFrame->nextTabWidget(Qt::Key_Backtab);
}

TEST_F(Tst_Fullscreenframe, tab_nextTabWidget_test)
{
    m_fullScreenFrame->updateDisplayMode(GROUP_BY_CATEGORY);
    m_fullScreenFrame->nextTabWidget(Qt::Key_Tab);

    m_fullScreenFrame->updateDisplayMode(SEARCH);
    m_fullScreenFrame->nextTabWidget(Qt::Key_Tab);
}

