// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <unistd.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/paths.h"
#include "base/settings.h"
#include "ui/application.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace Lingmo;
using namespace Lingmo::UI;
using namespace std::chrono_literals;

namespace {

class WindowRestoreTest : public testing::Test
{
protected:
    void SetUp() override
    {
        m_previousId = Application::instance()->applicationId();
        // One file per process: ctest runs the tests of this suite side by side.
        const std::string name = "toolkit-restore-test-" + std::to_string(getpid());
        Application::instance()->setApplicationId("lingmo-" + name);
        m_path = base::paths::lingmoConfig(name);
        std::filesystem::remove(m_path);
    }
    void TearDown() override
    {
        std::filesystem::remove(m_path);
        Application::instance()->setApplicationId(m_previousId);
    }

    static void runFor(std::chrono::milliseconds limit)
    {
        base::EventLoop& loop = Application::instance()->loop();
        loop.startTimer(limit, false, [&loop] { loop.quit(); });
        loop.run();
    }

    std::string m_previousId;
    std::string m_path;
};

} // namespace

TEST_F(WindowRestoreTest, KeepsTheSizeTheUserLeftAndMaximized)
{
    {
        Window window;
        window.setMinimumSize({300, 200});
        window.setRestoresSize("Main");
        window.resize({640, 480});
        window.show();
        EXPECT_EQ(window.size(), (SizeF{640, 480}));

        // A size the window was given is not the user's choice.
        TestInput::configure(window, 660, 490, window.states());
        runFor(400ms);
        EXPECT_FALSE(std::filesystem::exists(m_path));

        TestInput::resizeInteractively(window, {700, 500});
        runFor(400ms);
        // The INI format base::Settings writes, which other programs read.
        EXPECT_EQ(base::file::read(m_path), "[WindowSize]\nMain=@Size(700 500)\n");

        window.setMaximized(true);
        EXPECT_TRUE(base::Settings(m_path).boolValue("WindowMaximized/Main"));
    }
    {
        Window window;
        window.setMinimumSize({300, 200});
        window.setRestoresSize("Main");
        window.show();
        EXPECT_EQ(window.size(), (SizeF{700, 500}));
        EXPECT_TRUE(window.states().maximized);
        window.setMaximized(false);
        EXPECT_FALSE(base::Settings(m_path).contains("WindowMaximized/Main"));
    }
    {
        // Another id has its own size; a fixed-size window never maximizes.
        Window window;
        window.setRestoresSize("Other");
        window.resize({400, 300});
        window.show();
        EXPECT_EQ(window.size(), (SizeF{400, 300}));
    }
}

TEST_F(WindowRestoreTest, RestoredSizeStaysInsideTheWorkArea)
{
    {
        base::Settings settings(m_path);
        settings.setValue("WindowSize/Main", base::Settings::Size{2000, 1500});
    }
    Window window;
    window.setMinimumSize({300, 200});
    window.setMaximumSize({1800, 1800});
    window.setRestoresSize("Main");
    window.show();
    EXPECT_EQ(window.size(), (SizeF{1800, 1500}));
    TestInput::configureBounds(window, 1280, 760);
    TestInput::configure(window, 0, 0, window.states());
    EXPECT_EQ(window.size(), (SizeF{1280, 760}));
}

TEST_F(WindowRestoreTest, ClosingRightAfterAResizeStillSaves)
{
    {
        Window window;
        window.setRestoresSize("Main");
        window.show();
        TestInput::resizeInteractively(window, {720, 520});
    }
    EXPECT_EQ(base::Settings(m_path).sizeValue("WindowSize/Main"), (base::Settings::Size{720, 520}));
}
