// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "core/config.h"
#include "testutil.h"

#include "ui/application.h"

#include <gtest/gtest.h>

using namespace Lingmo;

// The headless application every test of the binary shares.
ui::Application* s_app = nullptr;

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    // Nothing a test changes reaches the user's settings.
    fmtest::TemporaryDir config;
    fm::setSettingsFile(config.filePath("filemanager.conf"));
    ui::Application app(argc, argv, ui::Application::Platform::Headless);
    s_app = &app;
    return RUN_ALL_TESTS();
}
