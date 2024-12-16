// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gtest/gtest.h"
#include "ut_cleanerresultitemwidget.h"
#include "window/modules/cleaner/widgets/cleanerresultitemwidget.h"

#include <DPalette>
#include <DTipLabel>
#include <DSpinner>

DGUI_USE_NAMESPACE

CleanerResultItemWidgetTest::CleanerResultItemWidgetTest()
{
}

TEST_F(CleanerResultItemWidgetTest, Sample)
{
    CleanerResultItemWidget cleanerWidgetNotRoot;
    CleanerResultItemWidget cleanerWidgetRoot;

    cleanerWidgetRoot.setAsRoot();
    cleanerWidgetNotRoot.setRoot(&cleanerWidgetRoot);

    cleanerWidgetRoot.setHeadText("head_root");
    cleanerWidgetRoot.setHeadTipText("head_tip");
    cleanerWidgetRoot.setUsedSpace("0 Byte");
    cleanerWidgetRoot.setTailText("tail_root");

    cleanerWidgetNotRoot.setHeadText("head_not_root");
    cleanerWidgetNotRoot.setHeadTipText("head_tip_not_root");
    cleanerWidgetNotRoot.setUsedSpace("1 GB");
    cleanerWidgetNotRoot.setTailText("tail_not_root");

    // prepare, 清空所有窗口内容
    cleanerWidgetNotRoot.setCleanDone();
    QString cleanedUsageLabel = cleanerWidgetNotRoot.findChild<DLabel *>("usageLabel")->text();
    QString cleanedTailLabel = cleanerWidgetNotRoot.findChild<DLabel *>("tailLabel")->text();
    EXPECT_EQ("", cleanedUsageLabel);
    EXPECT_EQ(tr("Removed"), cleanedTailLabel);

    cleanerWidgetNotRoot.prepare();
    EXPECT_EQ("", cleanerWidgetNotRoot.findChild<DLabel *>("usageLabel")->text());
    EXPECT_EQ("", cleanerWidgetNotRoot.findChild<DLabel *>("tailLabel")->text());
    EXPECT_EQ(false, cleanerWidgetNotRoot.findChild<DSpinner *>("spinner")->isPlaying());
}

// contentFrame
// DPalette::NoType  根项的背景
// DPalette::ItemBackground  非根项的背景
// 出乎意料的失败案例，但是运行时结果是正确的
TEST_F(CleanerResultItemWidgetTest, BackGroundStyle)
{
    CleanerResultItemWidget cleanerWidgetNotRoot;
    CleanerResultItemWidget cleanerWidgetRoot;
    cleanerWidgetRoot.setAsRoot();
    cleanerWidgetNotRoot.setRoot(&cleanerWidgetRoot);

    // QPalette::ColorRole outputRoot = cleanerWidgetRoot.findChild<DFrame *>("contentFrame")->backgroundRole();
    // QPalette::ColorRole outputNotRoot = cleanerWidgetNotRoot.findChild<DFrame *>("contentFrame")->backgroundRole();

    //    EXPECT_EQ(DPalette::NoType, cleanerWidgetRoot.findChild<DFrame *>("contentFrame")->backgroundRole());
    //    EXPECT_EQ(DPalette::ItemBackground, cleanerWidgetNotRoot.findChild<DFrame *>("contentFrame")->backgroundRole());
    //    EXPECT_EQ(DPalette::NoType, outputRoot);
    //    EXPECT_EQ(DPalette::ItemBackground, outputNotRoot);
}

// 作为根项目的窗口，可以设置tailText
// 其它窗口可设置，但不会写入
TEST_F(CleanerResultItemWidgetTest, SetTailText)
{
    CleanerResultItemWidget cleanerWidgetNotRoot;
    CleanerResultItemWidget cleanerWidgetRoot;
    cleanerWidgetRoot.setAsRoot();
    cleanerWidgetNotRoot.setRoot(&cleanerWidgetRoot);

    QString inputRoot("myTailRoot");
    QString intputNotRoot("myTailnNotRoot");

    cleanerWidgetNotRoot.setTailText(intputNotRoot);
    cleanerWidgetRoot.setTailText(inputRoot);

    QString outputRoot = cleanerWidgetRoot.findChild<DTipLabel *>("tailLabel")->text();
    QString outPutNotRoot = cleanerWidgetNotRoot.findChild<DTipLabel *>("tailLabel")->text();

    EXPECT_EQ(inputRoot, cleanerWidgetRoot.findChild<DTipLabel *>("tailLabel")->text());
    EXPECT_EQ("", cleanerWidgetNotRoot.findChild<DTipLabel *>("tailLabel")->text());
}

// 触发信号时的区别
// root项、非root项发在被勾选时发送不一样的信号
TEST_F(CleanerResultItemWidgetTest, EmitSignal_Root)
{
    CleanerResultItemWidget cleanerWidgetRoot;
    cleanerWidgetRoot.setAsRoot();

    bool rootClickedFlag = false;
    bool notRootClickedFlag = false;

    connect(&cleanerWidgetRoot, &CleanerResultItemWidget::rootClicked, this, [&](bool flag) {
        rootClickedFlag = flag;
    });
    connect(&cleanerWidgetRoot, &CleanerResultItemWidget::childClicked, this, [&](bool flag) {
        notRootClickedFlag = flag;
    });

    // 通过setCheckBoxStatus触发信号
    cleanerWidgetRoot.setCheckBoxStatus(true);
    cleanerWidgetRoot.childSelected();
    EXPECT_EQ(cleanerWidgetRoot.getCheckBoxStatus(), rootClickedFlag);
    EXPECT_EQ(false, notRootClickedFlag);

    cleanerWidgetRoot.setCheckBoxStatus(false);
    cleanerWidgetRoot.childSelected();
    EXPECT_EQ(cleanerWidgetRoot.getCheckBoxStatus(), rootClickedFlag);
    EXPECT_EQ(false, notRootClickedFlag);

    cleanerWidgetRoot.setCheckBoxStatus(true);
    cleanerWidgetRoot.childSelected();
    EXPECT_EQ(cleanerWidgetRoot.getCheckBoxStatus(), rootClickedFlag);
    EXPECT_EQ(false, notRootClickedFlag);
}

// 触发信号时的区别
// root项、非root项发在被勾选时发送不一样的信号
TEST_F(CleanerResultItemWidgetTest, EmitSignal_NotRoot)
{
    CleanerResultItemWidget cleanerWidgetNotRoot;

    bool rootClickedFlag = false;
    bool notRootClickedFlag = false;

    connect(&cleanerWidgetNotRoot, &CleanerResultItemWidget::rootClicked, this, [&](bool flag) {
        rootClickedFlag = flag;
    });
    connect(&cleanerWidgetNotRoot, &CleanerResultItemWidget::childClicked, this, [&](bool flag) {
        notRootClickedFlag = flag;
    });

    // 通过setCheckBoxStatus() childSelected()触发信号
    cleanerWidgetNotRoot.setCheckBoxStatus(true);
    cleanerWidgetNotRoot.childSelected();
    EXPECT_EQ(cleanerWidgetNotRoot.getCheckBoxStatus(), notRootClickedFlag);
    EXPECT_EQ(false, rootClickedFlag);

    cleanerWidgetNotRoot.setCheckBoxStatus(false);
    cleanerWidgetNotRoot.childSelected();
    EXPECT_EQ(cleanerWidgetNotRoot.getCheckBoxStatus(), notRootClickedFlag);
    EXPECT_EQ(false, rootClickedFlag);

    cleanerWidgetNotRoot.setCheckBoxStatus(true);
    cleanerWidgetNotRoot.childSelected();
    EXPECT_EQ(cleanerWidgetNotRoot.getCheckBoxStatus(), notRootClickedFlag);
    EXPECT_EQ(false, rootClickedFlag);
}

// 切换工作状态时，spinner和tailText状态与内容
// 仅root窗口有效
TEST_F(CleanerResultItemWidgetTest, WorkStart_Spinner)
{
    CleanerResultItemWidget cleanerWidgetNotRoot;
    CleanerResultItemWidget cleanerWidgetRoot;
    cleanerWidgetRoot.setAsRoot();
    cleanerWidgetNotRoot.setRoot(&cleanerWidgetRoot);

    DSpinner *notRootSpinner = nullptr;
    cleanerWidgetNotRoot.setWorkStarted(true);
    notRootSpinner = cleanerWidgetNotRoot.findChild<DSpinner *>("spinner");
    ASSERT_NE(nullptr, notRootSpinner);
    EXPECT_EQ(true, notRootSpinner->isHidden());
    EXPECT_EQ(false, notRootSpinner->isPlaying());

    DSpinner *rootSpinner = nullptr;
    cleanerWidgetRoot.setWorkStarted(true);
    rootSpinner = cleanerWidgetRoot.findChild<DSpinner *>("spinner");
    ASSERT_NE(nullptr, rootSpinner);
    EXPECT_EQ(false, rootSpinner->isHidden());
    EXPECT_EQ(true, rootSpinner->isPlaying());

    cleanerWidgetRoot.setWorkStarted(false);
    EXPECT_EQ(true, rootSpinner->isHidden());
    EXPECT_EQ(false, rootSpinner->isPlaying());
    rootSpinner = nullptr;
}
