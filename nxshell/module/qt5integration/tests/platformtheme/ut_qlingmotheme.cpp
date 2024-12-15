/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include "qlingmotheme.h"
#include "filedialogmanagerservice.h"
#ifndef DIALOG_SERVICE
#define DIALOG_SERVICE "org.lingmo.fakefilemanager.filedialog"
#endif

class EnvGuard {
public:
    EnvGuard(const char *name, const QByteArray &value) : m_name{name}
    {
        m_prev = qgetenv(name);
        qputenv(name, value);
    }
    ~EnvGuard()
    {
        qputenv(qPrintable(m_name), m_prev);
    }
private:
    QString m_name;
    QByteArray m_prev;
};

class TestQLingmoTheme : public testing::Test
{
public:
    static void SetUpTestSuite()
    {
        qputenv("_d_fileDialogServiceName", DIALOG_SERVICE);
        theme = new QLingmoTheme;
        managerService = new FileDialogManagerService(DIALOG_SERVICE, "/com/lingmo/filemanager/filedialogmanager");
    }

    static void TearDownTestSuite()
    {
        qunsetenv("_d_fileDialogServiceName");
        delete theme;
        delete managerService;
    }

    static QLingmoTheme *theme;
    static FileDialogManagerService *managerService;
};

QLingmoTheme *TestQLingmoTheme::theme = nullptr;
FileDialogManagerService *TestQLingmoTheme::managerService = nullptr;

TEST_F(TestQLingmoTheme, usePlatformNativeDialog)
{
    managerService->m_useFileChooserDialog = true;
    {
        // Test if environment variable take effect
        EnvGuard guard{"_d_disableDBusFileDialog", "false"};
        EXPECT_TRUE(theme->usePlatformNativeDialog(QLingmoTheme::FileDialog));
    }
    {
        EnvGuard guard{"_d_disableDBusFileDialog", "true"};
        EXPECT_FALSE(theme->usePlatformNativeDialog(QLingmoTheme::FileDialog));
    }
    managerService->m_useFileChooserDialog = false;
    {
        EnvGuard guard{"_d_disableDBusFileDialog", "false"};
        EXPECT_FALSE(theme->usePlatformNativeDialog(QLingmoTheme::FileDialog));
    }
    {
        EnvGuard guard{"_d_disableDBusFileDialog", "true"};
        EXPECT_FALSE(theme->usePlatformNativeDialog(QLingmoTheme::FileDialog));
    }
}

TEST_F(TestQLingmoTheme, createPlatformDialogHelper)
{
    managerService->m_useFileChooserDialog = true;
    auto helper = theme->createPlatformDialogHelper(QPlatformTheme::FileDialog);
    EXPECT_NE(nullptr, helper);
}

// Test if right icon engine is choosed
TEST_F(TestQLingmoTheme, createIconEngine)
{
    auto engine = theme->createIconEngine("icon_Layout");
    ASSERT_NE(nullptr, engine);
}

TEST_F(TestQLingmoTheme, settings)
{
    ASSERT_NE(nullptr, theme->settings());
}
