// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The shell's view of the window manager's application windows.

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "shell/common/wm_task_windows.h"
#include "shm_test_util.h"
#include "wm/activation.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

using namespace Lingmo;

namespace {

const base::Rect kOutput(0, 0, 1000, 800);

class WmTaskWindowsTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer = scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    compositor::Compositor compositor{surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat{&compositor};
    wm::WindowManager manager{&compositor, &seat};
    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 20);

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        manager.setOutputBounds(kOutput);
    }

    void TearDown() override { seat.shutdown(); }

    compositor::Surface *makeSurface()
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        const base::Size size(200, 100);
        auto buffer = pool->createBuffer(size, 0, size.width() * 4, render::PixelFormat::Bgrx8888, nullptr);
        surface->attach(buffer, {});
        surface->damage(base::Rect(base::Point(), size));
        surface->commit();
        compositor.updateTextures(renderer);
        return surface;
    }

    wm::Window *open(const std::string &appId, const std::string &title, int64_t pid = 10)
    {
        return manager.createWindow(makeSurface(), {.requestedSize = {200, 100},
                                                    .title = title,
                                                    .applicationId = appId,
                                                    .processId = pid});
    }
};

} // namespace

TEST_F(WmTaskWindowsTest, AnnouncesApplicationWindowsOnly)
{
    wm::Window *before = open("org.example.Before", "Before");
    shell::WmTaskWindows windows(manager);
    ASSERT_EQ(windows.windows(), std::vector<uint64_t>{before->id()});

    std::vector<uint64_t> added;
    auto subscription = windows.windowAdded.connect([&](uint64_t id) { added.push_back(id); });
    wm::Window *editor = open("org.example.Editor", "Draft", 42);

    // A shell surface is no application window.
    compositor::Surface *dockSurface = makeSurface();
    wm::ShellWindowMetadata metadata;
    metadata.role = wm::WindowRole::Dock;
    manager.setShellMetadata(dockSurface, metadata);
    wm::Window *dock = manager.createWindow(dockSurface, {.requestedSize = {200, 100}});

    EXPECT_EQ(added, std::vector<uint64_t>{editor->id()});
    const shell::TaskWindow *info = windows.window(editor->id());
    ASSERT_NE(info, nullptr);
    EXPECT_EQ(info->appId, "org.example.Editor");
    EXPECT_EQ(info->title, "Draft");
    EXPECT_EQ(info->pid, 42);
    EXPECT_TRUE(info->isTaskListWindow());
    EXPECT_EQ(windows.window(dock->id()), nullptr);
}

TEST_F(WmTaskWindowsTest, ReportsChangesAndTheActiveApplicationWindow)
{
    shell::WmTaskWindows windows(manager);
    wm::Window *editor = open("org.example.Editor", "Draft");
    wm::Window *viewer = open("org.example.Viewer", "Picture");

    std::vector<std::pair<uint64_t, uint32_t>> changes;
    auto changed = windows.windowChanged.connect(
            [&](uint64_t id, uint32_t what) { changes.emplace_back(id, what); });
    std::vector<uint64_t> active;
    auto activeChanged = windows.activeWindowChanged.connect([&](uint64_t id) { active.push_back(id); });

    editor->setTitle("Draft 2");
    ASSERT_FALSE(changes.empty());
    EXPECT_EQ(changes.back(), std::make_pair(editor->id(), uint32_t(shell::kTitleChange)));
    EXPECT_EQ(windows.window(editor->id())->title, "Draft 2");

    manager.activation()->activate(viewer);
    EXPECT_EQ(windows.activeWindow(), viewer->id());

    changes.clear();
    windows.minimize(viewer->id());
    EXPECT_TRUE(windows.window(viewer->id())->minimized);
    EXPECT_TRUE(std::any_of(changes.begin(), changes.end(), [&](const auto &change) {
        return change.first == viewer->id() && (change.second & shell::kStateChange);
    }));

    // Activating restores it.
    windows.activate(viewer->id());
    EXPECT_FALSE(windows.window(viewer->id())->minimized);
    EXPECT_EQ(windows.activeWindow(), viewer->id());

    windows.setHidden(editor->id(), true);
    EXPECT_TRUE(windows.window(editor->id())->hidden);

    std::vector<uint64_t> removed;
    auto gone = windows.windowRemoved.connect([&](uint64_t id) { removed.push_back(id); });
    const uint64_t viewerId = viewer->id();
    manager.destroyWindow(viewer);
    EXPECT_EQ(removed, std::vector<uint64_t>{viewerId});
    EXPECT_EQ(windows.window(viewerId), nullptr);
    EXPECT_NE(windows.activeWindow(), viewerId);
}

TEST_F(WmTaskWindowsTest, KeepsTheApplicationAWindowOffTheTaskListIsInFrontOf)
{
    shell::WmTaskWindows windows(manager);
    wm::Window *editor = open("org.example.Editor", "Draft");
    manager.activation()->activate(editor);
    ASSERT_EQ(windows.activeWindow(), editor->id());

    wm::Window *dialog = open({}, "Log out now?");
    dialog->rootSurface()->setSkipTaskbar(true);
    manager.activation()->activate(dialog);
    EXPECT_EQ(windows.activeWindow(), editor->id());

    // Leaving the task list while active gives the bar back as well.
    manager.activation()->activate(editor);
    wm::Window *viewer = open("org.example.Viewer", "Picture");
    manager.activation()->activate(viewer);
    ASSERT_EQ(windows.activeWindow(), viewer->id());
    viewer->rootSurface()->setSkipTaskbar(true);
    EXPECT_EQ(windows.activeWindow(), editor->id());
}
