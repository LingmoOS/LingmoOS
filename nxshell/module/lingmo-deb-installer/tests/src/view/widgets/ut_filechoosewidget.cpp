// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/widgets/filechoosewidget.h"
#include <stub.h>

#include <DFileDialog>

#include <gtest/gtest.h>
typedef int (*DFileDialogfptr)(QDialog *);
DFileDialogfptr DDialog_exec = (DFileDialogfptr)(&QDialog::exec);

int stub_exec()
{
    return 0;
}

TEST(FileChooseWidget_TEST, FileChooseWidget_UT_themeChanged)
{
    FileChooseWidget *fchooseWidget = new FileChooseWidget;
    fchooseWidget->themeChanged();
    ASSERT_EQ(QSize(160, 160), fchooseWidget->m_iconImage->size());
    delete fchooseWidget;
}

TEST(FileChooseWidget_TEST, FileChooseWidget_UT_chooseFiles)
{
    FileChooseWidget *fchooseWidget = new FileChooseWidget;
    fchooseWidget->m_settings.setValue("history_dir", "/");
    Stub stub;
    stub.set(DDialog_exec, stub_exec);
    fchooseWidget->chooseFiles();
    ASSERT_EQ("/", fchooseWidget->m_settings.value("history_dir"));
    delete fchooseWidget;
}
