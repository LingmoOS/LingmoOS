/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <gtest/gtest.h>
#include "qdeepintheme.h"
#include "filedialogmanagerservice.h"
#ifndef DIALOG_SERVICE
#define DIALOG_SERVICE "org.deepin.fakefilemanager.filedialog"
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

class TestQDeepinTheme : public testing::Test
{
public:
    static void SetUpTestSuite()
    {
        qputenv("_d_fileDialogServiceName", DIALOG_SERVICE);
        theme = new QDeepinTheme;
        managerService = new FileDialogManagerService(DIALOG_SERVICE, "/com/deepin/filemanager/filedialogmanager");
    }

    static void TearDownTestSuite()
    {
        qunsetenv("_d_fileDialogServiceName");
        delete theme;
        delete managerService;
    }

    static QDeepinTheme *theme;
    static FileDialogManagerService *managerService;
};

QDeepinTheme *TestQDeepinTheme::theme = nullptr;
FileDialogManagerService *TestQDeepinTheme::managerService = nullptr;

TEST_F(TestQDeepinTheme, usePlatformNativeDialog)
{
    managerService->m_useFileChooserDialog = true;
    {
        // Test if environment variable take effect
        EnvGuard guard{"_d_disableDBusFileDialog", "false"};
        EXPECT_TRUE(theme->usePlatformNativeDialog(QDeepinTheme::FileDialog));
    }
    {
        EnvGuard guard{"_d_disableDBusFileDialog", "true"};
        EXPECT_FALSE(theme->usePlatformNativeDialog(QDeepinTheme::FileDialog));
    }
    managerService->m_useFileChooserDialog = false;
    {
        EnvGuard guard{"_d_disableDBusFileDialog", "false"};
        EXPECT_FALSE(theme->usePlatformNativeDialog(QDeepinTheme::FileDialog));
    }
    {
        EnvGuard guard{"_d_disableDBusFileDialog", "true"};
        EXPECT_FALSE(theme->usePlatformNativeDialog(QDeepinTheme::FileDialog));
    }
}

TEST_F(TestQDeepinTheme, createPlatformDialogHelper)
{
    managerService->m_useFileChooserDialog = true;
    auto helper = theme->createPlatformDialogHelper(QPlatformTheme::FileDialog);
    EXPECT_NE(nullptr, helper);
}

// Test if right icon engine is choosed
TEST_F(TestQDeepinTheme, createIconEngine)
{
    auto engine = theme->createIconEngine("icon_Layout");
    ASSERT_NE(nullptr, engine);
}

TEST_F(TestQDeepinTheme, settings)
{
    ASSERT_NE(nullptr, theme->settings());
}
