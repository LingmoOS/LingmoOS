// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The compositor's command line, logging categories, preference paths and
// child environment.

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <vector>

#include <unistd.h>

#include <gtest/gtest.h>

#include "base/paths.h"
#include "core/compositor_options.h"
#include "core/logging.h"
#include "core/preferences.h"
#include "core/process_environment.h"

namespace Lingmo {
namespace {

std::optional<core::CompositorOptions> parse(std::vector<std::string> arguments)
{
    arguments.insert(arguments.begin(), "lingmo-aurora");
    std::vector<char *> argv;
    for (std::string &argument : arguments)
        argv.push_back(argument.data());
    argv.push_back(nullptr);
    return core::CompositorOptions::parse(int(arguments.size()), argv.data());
}

TEST(CompositorOptionsTest, Defaults)
{
    const auto options = parse({});
    ASSERT_TRUE(options);
    EXPECT_EQ(options->socketName, "");
    EXPECT_FALSE(options->noDrm);
    EXPECT_EQ(options->headlessOutputs, 1);
    EXPECT_EQ(options->readyFd, -1);
    EXPECT_EQ(options->scale.numerator(), 120u);
    EXPECT_EQ(options->displayConfigService, "org.lingmo.DisplayConfig");
    EXPECT_TRUE(options->clipboardClients.empty());
    EXPECT_TRUE(options->logRules.empty());
}

TEST(CompositorOptionsTest, ReadsValuesAndRepeats)
{
    const auto options = parse({"--socket", "wayland-9", "--no-drm", "--headless-outputs=3",
                                "--scale", "1.5", "--capture-client", "/a", "--capture-client=/b",
                                "--login", "--input-method", "fcitx5", "--log-rules",
                                "lingmo.drm.debug=true"});
    ASSERT_TRUE(options);
    EXPECT_EQ(options->socketName, "wayland-9");
    EXPECT_TRUE(options->noDrm);
    EXPECT_EQ(options->headlessOutputs, 3);
    EXPECT_EQ(options->scale.numerator(), 180u);
    EXPECT_EQ(options->captureClients, (std::vector<std::string> {"/a", "/b"}));
    EXPECT_TRUE(options->login);
    EXPECT_EQ(options->inputMethodId, "fcitx5");
    EXPECT_EQ(options->logRules, "lingmo.drm.debug=true");
}

// Anyone can run the screenshot tool or the recorder with arguments of their
// choosing, so their paths grant nothing.
TEST(CompositorOptionsTest, OnlyThePortalBackendCapturesByItsPath)
{
    const auto defaults = parse({});
    ASSERT_TRUE(defaults);
    EXPECT_EQ(core::captureExecutables(*defaults), (std::vector<std::string> {LINGMO_PORTAL_BACKEND}));
    const auto added = parse({"--capture-client", "/a"});
    ASSERT_TRUE(added);
    const std::vector<std::string> executables = core::captureExecutables(*added);
    EXPECT_EQ(executables, (std::vector<std::string> {LINGMO_PORTAL_BACKEND, "/a"}));
    for (const std::string &tool : {std::string(LINGMO_BINDIR "/lingmo-screenshot"),
                                    std::string(LINGMO_SCREEN_RECORDER)})
        EXPECT_EQ(std::find(executables.begin(), executables.end(), tool), executables.end()) << tool;
}

TEST(CompositorOptionsTest, RejectsInvalidValues)
{
    EXPECT_FALSE(parse({"--scale", "5"}));
    EXPECT_FALSE(parse({"--scale", "1,5"}));
    EXPECT_FALSE(parse({"--no-drm", "--headless-outputs", "0"}));
    EXPECT_FALSE(parse({"--no-drm", "--headless-outputs", "17"}));
    EXPECT_FALSE(parse({"--ready-fd", "2"}));
    EXPECT_FALSE(parse({"--ready-fd", "x"}));
    // A count is checked only for the headless backend, as before.
    EXPECT_TRUE(parse({"--headless-outputs", "0"}));
}

TEST(CompositorOptionsTest, TakesTheReadyDescriptor)
{
    int fds[2];
    ASSERT_EQ(pipe(fds), 0);
    const int fd = fds[1] >= 3 ? fds[1] : -1;
    ASSERT_GE(fd, 3);
    const auto options = parse({"--ready-fd", std::to_string(fd)});
    ASSERT_TRUE(options);
    EXPECT_EQ(options->readyFd, fd);
    close(fds[0]);
    close(fds[1]);
}

TEST(LoggingTest, RulesReachTheCategories)
{
    EXPECT_TRUE(lingmoDrm.isEnabled(base::LogLevel::Info));
    EXPECT_FALSE(lingmoDrm.isEnabled(base::LogLevel::Debug));

    core::setLoggingRules("lingmo.drm.debug=true;lingmo.scene=false");
    EXPECT_TRUE(lingmoDrm.isEnabled(base::LogLevel::Debug));
    EXPECT_FALSE(lingmoScene.isEnabled(base::LogLevel::Warning));

    core::setLoggingRules("");
    EXPECT_FALSE(lingmoDrm.isEnabled(base::LogLevel::Debug));
    EXPECT_TRUE(lingmoScene.isEnabled(base::LogLevel::Warning));
}

TEST(LoggingTest, TheMacroWritesOneLine)
{
    std::vector<std::string> lines;
    base::logging::setHandler([&](const base::LogCategory &category, base::LogLevel level,
                                  std::string_view message) {
        lines.push_back(base::logging::formatLine(category, level, message));
    });
    LM_INFO(lingmoWayland) << "started" << 3;
    base::logging::setHandler(nullptr);
    ASSERT_EQ(lines.size(), 1u);
    EXPECT_EQ(lines.front(), "lingmo.wayland: started 3");
}

TEST(PreferencesTest, TestsWriteUnderTheirOwnConfigHome)
{
    // The test main points every test binary at a scratch directory.
    const std::string scratch = std::filesystem::path(preferencesDirectory()).parent_path();
    EXPECT_NE(scratch, base::paths::configHome());

    setConfigHomeForTesting("/tmp/lingmo-core-test");
    EXPECT_EQ(preferencesDirectory(), "/tmp/lingmo-core-test/lingmo");
    EXPECT_EQ(preferencesFile("compositor"), "/tmp/lingmo-core-test/lingmo/compositor.conf");

    // Without an override, the user's configuration directory.
    setConfigHomeForTesting({});
    EXPECT_EQ(preferencesFile("shell"), base::paths::configHome() + "/lingmo/shell.conf");
    setConfigHomeForTesting(scratch);
}

TEST(ProcessEnvironmentTest, ChildrenGetTheCurrentEnvironment)
{
    setenv("LINGMO_CORE_TEST_SET", "outer", 1);
    unsetenv("LINGMO_CORE_TEST_UNSET");

    const std::vector<std::string> child = childEnvironment();
    EXPECT_EQ(std::count(child.begin(), child.end(), "LINGMO_CORE_TEST_SET=outer"), 1);
    EXPECT_EQ(std::count_if(child.begin(), child.end(), [](const std::string &entry) {
                  return entry.starts_with("LINGMO_CORE_TEST_UNSET=");
              }),
              0);
    EXPECT_GT(child.size(), 1u);
}

} // namespace
} // namespace Lingmo
