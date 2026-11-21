// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "ui/application.h"
#include "widgets/picture.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    Lingmo::ui::Application application(argc, argv, Lingmo::ui::Application::Platform::Headless);
    settings::setDataDirectory(SETTINGS_SOURCE_DIR);
    return RUN_ALL_TESTS();
}
