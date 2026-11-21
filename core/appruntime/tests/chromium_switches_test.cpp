// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "chromiumswitches.h"

#include "base/file.h"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

namespace fs = std::filesystem;
using Command = std::vector<std::string>;

static void touch(const std::string &path)
{
    std::ofstream file(path);
    ASSERT_TRUE(file.good());
}

class ChromiumSwitchesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_root.isValid());
        fs::create_directories(path("electron-app"));
        fs::create_directories(path("bin"));
        fs::create_directories(path("plain"));

        touch(path("electron-app/app"));
        touch(path("electron-app/icudtl.dat"));
        touch(path("electron-app/chrome_100_percent.pak"));
        fs::create_symlink(path("electron-app/app"), path("bin/app"));

        touch(path("plain/tool"));
        touch(path("plain/icudtl.dat"));
    }

    std::string path(const std::string &name) const { return m_root.path() + '/' + name; }

    Lingmo::Base::file::TemporaryDirectory m_root {fs::temp_directory_path().string()};
};

TEST_F(ChromiumSwitchesTest, switchFollowsTheProgram)
{
    const std::string app = path("electron-app/app");
    EXPECT_EQ(withChromiumSwitches({app, "claude://new"}),
              (Command {app, "--disable-font-subpixel-positioning", "claude://new"}));
}

TEST_F(ChromiumSwitchesTest, symlinkResolvesToTheResources)
{
    EXPECT_TRUE(isChromiumProgram(path("bin/app")));
}

TEST_F(ChromiumSwitchesTest, otherProgramsAreLeftAlone)
{
    // icudtl.dat alone is not a Chromium build.
    const Command tool {path("plain/tool"), "--flag"};
    EXPECT_EQ(withChromiumSwitches(tool), tool);

    const Command missing {path("plain/missing")};
    EXPECT_EQ(withChromiumSwitches(missing), missing);
    EXPECT_EQ(withChromiumSwitches({}), Command());
}
