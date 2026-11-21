// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common/detached_process.h"

#include "base/event_loop.h"
#include "base/file.h"
#include "tests/support/wait.h"

#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

using namespace Lingmo;

namespace {

class DetachedProcessTest : public ::testing::Test
{
protected:
    std::string read(const std::string &name) const
    {
        return base::file::read(directory + '/' + name).value_or(std::string());
    }

    void write(const std::string &name, const std::string &text) const
    {
        std::ofstream(directory + '/' + name) << text;
    }

    // The trimmed output `script` wrote to `name`, once it has.
    std::string outputOf(const std::string &script, const std::string &name,
                         const core::Environment &environment = {{"PATH", "/usr/bin:/bin"}})
    {
        const std::string path = directory + '/' + name;
        const std::string written = path + ".done";
        EXPECT_GT(core::startDetached({"sh", "-c", "{ " + script + "; } > " + path + "; mv " + path + ' ' + written},
                                      directory, environment),
                  0);
        EXPECT_TRUE(testsupport::waitFor(loop, [&] { return std::filesystem::exists(written); }));
        std::string text = read(name + ".done");
        while (!text.empty() && text.back() == '\n')
            text.pop_back();
        return text;
    }

    base::EventLoop loop;
    base::file::TemporaryDirectory temporary {std::filesystem::temp_directory_path().string()};
    const std::string directory = temporary.path();
};

} // namespace

TEST_F(DetachedProcessTest, protectionIsDropped)
{
    write("score", "-900\n");
    core::dropOomProtection((directory + "/score").c_str());
    EXPECT_EQ(read("score"), "0");
}

TEST_F(DetachedProcessTest, unprotectedScoreStays)
{
    write("zero", "0\n");
    write("raised", "300\n");
    core::dropOomProtection((directory + "/zero").c_str());
    core::dropOomProtection((directory + "/raised").c_str());
    EXPECT_EQ(read("zero"), "0\n");
    EXPECT_EQ(read("raised"), "300\n");

    core::dropOomProtection((directory + "/missing").c_str());
    EXPECT_FALSE(std::filesystem::exists(directory + "/missing"));
}

TEST_F(DetachedProcessTest, raisedScoreIsInheritedUnchanged)
{
    // Raising it needs no privilege; the test cannot go below 0 without one.
    std::ofstream("/proc/self/oom_score_adj") << "250";
    ASSERT_EQ(base::file::read("/proc/self/oom_score_adj"), "250\n");
    EXPECT_EQ(outputOf("cat /proc/self/oom_score_adj", "score"), "250");
}

TEST_F(DetachedProcessTest, programRunsWhereAndAsAsked)
{
    EXPECT_EQ(outputOf("pwd", "directory"), std::filesystem::canonical(directory).string());
    EXPECT_EQ(outputOf("echo \"$ONLY\"; env | grep -c ^HOME=", "environment",
                       {{"PATH", "/usr/bin:/bin"}, {"ONLY", "value"}}),
              "value\n0");
}

TEST_F(DetachedProcessTest, failedStartIsReported)
{
    EXPECT_EQ(core::startDetached({"/nonexistent/program"}, directory, {}), -1);
    EXPECT_EQ(core::startDetached({"sh", "-c", "true"}, directory + "/missing", {{"PATH", "/usr/bin:/bin"}}), -1);
    EXPECT_EQ(core::startDetached({}, directory, {}), -1);
}
