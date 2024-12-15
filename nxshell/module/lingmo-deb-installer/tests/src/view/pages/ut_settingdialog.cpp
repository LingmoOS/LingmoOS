// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/pages/settingdialog.h"
#include "../deb-installer/utils/hierarchicalverify.h"

#include <DLabel>

#include <stub.h>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE

class UT_SettingDialog : public ::testing::Test
{
protected:
    void SetUp()
    {
        dialog = new SettingDialog;
        dialog->show();
    }
    void TearDown() { delete dialog; }

private:
    SettingDialog *dialog;
};

TEST_F(UT_SettingDialog, createProceedDefenderSafetyLabel_Pass)
{
    QWidget *w = SettingDialog::createProceedDefenderSafetyLabel(nullptr);
    DLabel *label = qobject_cast<DLabel *>(w);
    ASSERT_NE(label, nullptr);

    EXPECT_TRUE(label->text().contains("href='localtest.com'"));
    delete w;
}

static bool g_trigger = false;
void stub_proceedDefenderSafetyPage()
{
    g_trigger = true;
}

TEST_F(UT_SettingDialog, linkActivated_Trigger_Pass)
{
    QWidget *w = SettingDialog::createProceedDefenderSafetyLabel(nullptr);
    DLabel *label = qobject_cast<DLabel *>(w);
    ASSERT_NE(label, nullptr);

    Stub stub;
    stub.set(ADDR(HierarchicalVerify, proceedDefenderSafetyPage), stub_proceedDefenderSafetyPage);

    g_trigger = false;
    Q_EMIT label->linkActivated("");
    EXPECT_TRUE(g_trigger);
}

TEST_F(UT_SettingDialog, switchHierarchicalNotify_Enable_Pass)
{
    dialog->switchHierarchicalNotify(true);

    DLabel *label = dialog->findChild<DLabel *>("OptionProceedLabel");
    ASSERT_NE(label, nullptr);
    EXPECT_TRUE(label->isVisible());
    EXPECT_TRUE(label->text().contains("Security Center"));
}

TEST_F(UT_SettingDialog, switchHierarchicalNotify_Disable_Pass)
{
    dialog->switchHierarchicalNotify(false);

    DLabel *label = dialog->findChild<DLabel *>("OptionProceedLabel");
    ASSERT_NE(label, nullptr);
    EXPECT_FALSE(label->isVisible());
    EXPECT_TRUE(label->text().contains("Security Center"));
}
