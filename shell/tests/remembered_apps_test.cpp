// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <filesystem>
#include <optional>
#include <string>

#include "computer_control/remembered_apps.h"

using namespace Lingmo::Computer_control;

namespace {

class RememberedAppsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        directory = std::filesystem::temp_directory_path()
                / ("remembered-apps-" + std::to_string(getpid()));
        std::filesystem::create_directories(directory);
        file = (directory / "compositor.conf").string();
    }

    void TearDown() override { std::filesystem::remove_all(directory); }

    std::filesystem::path directory;
    std::string file;
};

TEST_F(RememberedAppsTest, ApplicationIsTheParentsExecutable)
{
    const pid_t child = fork();
    ASSERT_GE(child, 0);
    if (child == 0) {
        pause();
        _exit(0);
    }
    const std::optional<AgentApplication> application = RememberedApps::applicationOf(child);
    kill(child, SIGKILL);
    waitpid(child, nullptr, 0);

    ASSERT_TRUE(application);
    EXPECT_EQ(application->path, std::filesystem::read_symlink("/proc/self/exe").string());
    EXPECT_FALSE(application->file.empty());
}

TEST_F(RememberedAppsTest, NoApplicationForAnUnknownProcess)
{
    EXPECT_FALSE(RememberedApps::applicationOf(0));
    EXPECT_FALSE(RememberedApps::applicationOf(1));
}

TEST_F(RememberedAppsTest, RememberedAcrossInstances)
{
    const AgentApplication application {"/usr/lib/agent/agent", "1:2:3.4"};
    {
        RememberedApps apps(file);
        EXPECT_FALSE(apps.allows(application));
        apps.remember(application, "Agent", "agent");
        EXPECT_TRUE(apps.allows(application));
    }
    RememberedApps apps(file);
    EXPECT_TRUE(apps.allows(application));
}

TEST_F(RememberedAppsTest, AnotherFileAtThePathIsAnotherApplication)
{
    RememberedApps apps(file);
    apps.remember({"/usr/lib/agent/agent", "1:2:3.4"}, "Agent", "agent");
    EXPECT_FALSE(apps.allows({"/usr/lib/agent/agent", "1:5:3.4"}));
    EXPECT_FALSE(apps.allows({"/usr/lib/other/agent", "1:2:3.4"}));
}

TEST_F(RememberedAppsTest, ForgetRemovesTheApplication)
{
    const AgentApplication application {"/usr/lib/agent/agent", "1:2:3.4"};
    int changes = 0;
    {
        RememberedApps apps(file);
        const auto subscription = apps.changed.connect([&changes] { ++changes; });
        apps.remember(application, "Agent", "agent");
        apps.forget(application.path);
        apps.forget(application.path);
        EXPECT_FALSE(apps.allows(application));
    }
    EXPECT_EQ(changes, 2);
    RememberedApps apps(file);
    EXPECT_FALSE(apps.allows(application));
}

} // namespace
