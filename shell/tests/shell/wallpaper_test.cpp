// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// The desktop background in the compositor: the shared decode and its disk
// cache, the node on each output, the bar's verdict changing with the
// picture, and the desktop as window capture picks it.

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <chrono>
#include <memory>
#include <string>
#include <vector>

#include <linux/input-event-codes.h>
#include <gtest/gtest.h>

#include "base/thread_pool.h"
#include "compositor/capture_source.h"
#include "compositor/compositor.h"
#include "compositor/output_scale.h"
#include "compositor/surface.h"
#include "fake_renderer.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "loop_test_util.h"
#include "scene/node.h"
#include "scene/scene.h"
#include "scene/wallpaper_node.h"
#include "scene/window_animation.h"
#include "shell/shell.h"
#include "shell/shell_window.h"
#include "ui/embedded_window.h"
#include "ui/view.h"
#include "shell/wallpaper/bar_backdrop.h"
#include "shell/wallpaper/wallpaper.h"
#include "shell/wallpaper/wallpaper_cache.h"
#include "shm_test_util.h"
#include "temporary_directory.h"
#include "ui/bitmap.h"
#include "wm/window.h"
#include "wm/window_manager.h"

using namespace Lingmo;

namespace {

const base::Rect kOutput(0, 0, 1440, 960);
const base::Size kDevice(2160, 1440);

input::Timestamp at(int ms)
{
    return input::Timestamp(std::chrono::milliseconds(ms));
}

std::string writePicture(const test::TemporaryDirectory &directory, const std::string &name, int width, int height,
                         uint32_t argb)
{
    ui::Bitmap image(width, height, ui::Bitmap::Format::Argb32Premultiplied);
    image.fill(argb);
    const std::string path = directory.filePath(name);
    EXPECT_TRUE(image.savePng(path));
    return path;
}

std::vector<std::string> filesIn(const std::string &directory)
{
    std::vector<std::string> files;
    if (DIR *dir = opendir(directory.c_str())) {
        while (const dirent *entry = readdir(dir)) {
            if (entry->d_name[0] != '.')
                files.push_back(directory + '/' + entry->d_name);
        }
        closedir(dir);
    }
    return files;
}

ui::Bitmap loadNow(shell::WallpaperCache &cache, const std::string &path, const base::Size &size)
{
    ui::Bitmap result;
    bool done = false;
    const std::shared_ptr<void> request = cache.load(path, size, [&](const ui::Bitmap &image) {
        result = image;
        done = true;
    });
    EXPECT_TRUE(test::runUntil([&] { return done; }, std::chrono::seconds(10)));
    return result;
}

} // namespace

TEST(WallpaperCacheTest, CropsTheMiddleOfWhatCoversTheScreen)
{
    test::TemporaryDirectory directory;
    // Red, green and blue thirds, the green one in the middle.
    ui::Bitmap image(300, 100, ui::Bitmap::Format::Argb32Premultiplied);
    for (int y = 0; y < 100; ++y) {
        for (int x = 0; x < 300; ++x)
            image.setPixel(x, y, x < 100 ? 0xffff0000u : x < 200 ? 0xff00ff00u : 0xff0000ffu);
    }
    const std::string path = directory.filePath("thirds.png");
    ASSERT_TRUE(image.savePng(path));

    // A square screen sees the middle third, scaled to cover it.
    const ui::Bitmap square = shell::WallpaperCache::decode(path, {60, 60});
    ASSERT_EQ(base::Size(60, 60), square.size());
    EXPECT_EQ(0xff00ff00u, square.pixel(1, 1));
    EXPECT_EQ(0xff00ff00u, square.pixel(58, 58));
    // A screen of the picture's shape sees all of it.
    const ui::Bitmap wide = shell::WallpaperCache::decode(path, {150, 50});
    ASSERT_EQ(base::Size(150, 50), wide.size());
    EXPECT_EQ(0xffff0000u, wide.pixel(2, 25));
    EXPECT_EQ(0xff0000ffu, wide.pixel(147, 25));
    EXPECT_EQ(ui::Bitmap::Format::Argb32Premultiplied, wide.format());

    EXPECT_TRUE(shell::WallpaperCache::decode(directory.filePath("missing.png"), {60, 60}).isNull());
}

TEST(WallpaperCacheTest, SharesOneDecodeAndKeepsItForTheNextSession)
{
    test::TemporaryDirectory directory;
    const std::string cacheDirectory = directory.filePath("cache");
    const std::string path = writePicture(directory, "grey.png", 64, 48, 0xff808080u);
    {
        shell::WallpaperCache cache(cacheDirectory);
        ui::Bitmap first;
        ui::Bitmap second;
        const std::shared_ptr<void> a = cache.load(path, {32, 24}, [&](const ui::Bitmap &image) { first = image; });
        const std::shared_ptr<void> b = cache.load(path, {32, 24}, [&](const ui::Bitmap &image) { second = image; });
        ASSERT_TRUE(test::runUntil([&] { return !first.isNull() && !second.isNull(); }));
        // One decode for both.
        EXPECT_EQ(first.constBits(), second.constBits());
        // Kept: asked again, it comes back before load() returns.
        ui::Bitmap third;
        const std::shared_ptr<void> c = cache.load(path, {32, 24}, [&](const ui::Bitmap &image) { third = image; });
        EXPECT_EQ(first.constBits(), third.constBits());
        EXPECT_EQ(first.constBits(), cache.cached(path, {32, 24}).constBits());
        // A request dropped before its decode hears nothing.
        bool heard = false;
        std::shared_ptr<void> dropped = cache.load(path, {16, 12}, [&](const ui::Bitmap &) { heard = true; });
        dropped.reset();
        ASSERT_TRUE(test::runUntil([&] { return !cache.cached(path, {16, 12}).isNull(); }));
        EXPECT_FALSE(heard);
    }
    ASSERT_TRUE(test::runUntil([&] { return filesIn(cacheDirectory).size() == 2; }));

    // The next session reads the file: its last pixel, changed on disk, is
    // what it gets.
    const std::string key = shell::WallpaperCache::cacheKey(path, {32, 24});
    ASSERT_FALSE(key.empty());
    std::string cacheFile;
    for (const std::string &file : filesIn(cacheDirectory)) {
        struct stat info;
        stat(file.c_str(), &info);
        if (info.st_size > 32 * 24 * 4 && info.st_size < 16 * 12 * 4 * 8)
            cacheFile = file;
    }
    ASSERT_FALSE(cacheFile.empty());
    const int fd = open(cacheFile.c_str(), O_WRONLY);
    ASSERT_GE(fd, 0);
    const uint32_t marker = 0xff123456u;
    ASSERT_EQ(ssize_t(sizeof(marker)), pwrite(fd, &marker, sizeof(marker), lseek(fd, 0, SEEK_END) - 4));
    close(fd);
    shell::WallpaperCache next(cacheDirectory);
    EXPECT_EQ(marker, loadNow(next, path, {32, 24}).pixel(31, 23));

    // An edit of the picture misses.
    writePicture(directory, "grey.png", 64, 48, 0xff202020u);
    shell::WallpaperCache edited(cacheDirectory);
    EXPECT_EQ(0xff202020u, loadNow(edited, path, {32, 24}).pixel(31, 23));
}

TEST(WallpaperCacheTest, KeepsTheLastFourFiles)
{
    test::TemporaryDirectory directory;
    const std::string cacheDirectory = directory.filePath("cache");
    shell::WallpaperCache cache(cacheDirectory);
    for (int i = 0; i < 6; ++i) {
        const std::string path = writePicture(directory, "p" + std::to_string(i) + ".png", 20, 20, 0xff000000u + i);
        ASSERT_FALSE(loadNow(cache, path, {10, 10}).isNull());
        // Each written before the next, so their ages differ.
        ASSERT_TRUE(test::runUntil([&] { return filesIn(cacheDirectory).size() == size_t(std::min(i + 1, 4)); }));
        usleep(10000);
    }
    EXPECT_EQ(size_t(shell::WallpaperCache::kMaxCachedFiles), filesIn(cacheDirectory).size());
    // An unreadable picture is not cached.
    EXPECT_TRUE(loadNow(cache, directory.filePath("missing.png"), {10, 10}).isNull());
}

namespace {

class WallpaperTest : public testing::Test
{
protected:
    scene::Scene scene;
    scene::Node *surfaceContainer = scene.root()->addChild(std::make_unique<scene::Node>("surfaces"));
    // Already there, as the overview's nodes are when the shell starts.
    scene::Node *earlier = surfaceContainer->addChild(std::make_unique<scene::Node>("earlier"));
    compositor::Compositor compositor {surfaceContainer};
    testsupport::FakeRenderer renderer;
    input::Seat seat {&compositor};
    input::InputManager input {&seat};
    wm::WindowManager manager {&compositor, &seat};
    test::TemporaryDirectory directory;
    std::unique_ptr<shell::Shell> shell;
    shell::Wallpaper *wallpaper = nullptr;
    std::vector<std::unique_ptr<compositor::Surface>> surfaces;
    std::shared_ptr<compositor::ShmPool> pool = testsupport::makePool(1 << 22);

    void SetUp() override
    {
        scene.setSize(kOutput.size());
        compositor.setOutputScale(*compositor::OutputScale::fromNumerator(180));
        manager.setOutputBounds(kOutput);
        input.setBounds(kOutput.size());
        input.setEventSink(&manager);
        shell = std::make_unique<shell::Shell>(shell::ShellEnvironment {&compositor, &manager, &seat, nullptr});
        wallpaper = shell->addComponent(
                std::make_unique<shell::Wallpaper>(shell.get(), directory.filePath("cache")));
    }

    void TearDown() override
    {
        shell.reset();
        seat.shutdown();
    }

    scene::WallpaperNode *node() const { return wallpaper->node({}); }

    void showPicture(const std::string &path)
    {
        shell::Wallpaper::Background background;
        background.path = path;
        wallpaper->setBackground(background);
    }

    wm::Window *openApplication(const base::Rect &bounds)
    {
        surfaces.push_back(compositor.createSurface());
        compositor::Surface *surface = surfaces.back().get();
        surface->attach(pool->createBuffer(bounds.size(), 0, bounds.width() * 4, render::PixelFormat::Bgrx8888,
                                           nullptr),
                        {});
        surface->commit();
        compositor.updateTextures(renderer);
        wm::Window *window = manager.createWindow(surface, {.requestedSize = bounds.size(), .applicationId = "app"});
        window->setBounds(bounds);
        window->activate();
        return window;
    }
};

} // namespace

TEST_F(WallpaperTest, EachOutputHasANodeUnderEverythingElse)
{
    ASSERT_NE(nullptr, node());
    EXPECT_EQ(node(), surfaceContainer->children().front());
    EXPECT_EQ(kOutput, node()->geometry());
    EXPECT_EQ(scene::Node::Content::Opaque, node()->content());
    // Black until it knows what to show, and not ready.
    EXPECT_FALSE(wallpaper->isReady());
    EXPECT_EQ(0.0f, node()->color().r);

    // A window's node goes above it.
    openApplication(base::Rect(100, 100, 300, 200));
    EXPECT_EQ(node(), surfaceContainer->children().front());

    // The picture at the output's device size.
    showPicture(writePicture(directory, "dark.png", 64, 64, 0xff202020u));
    ASSERT_TRUE(test::runUntil([&] { return wallpaper->isReady(); }, std::chrono::seconds(10)));
    EXPECT_EQ(kDevice, node()->picture().size());
    EXPECT_EQ(kDevice, wallpaper->primaryPicture().size());
}

TEST_F(WallpaperTest, ANewPictureReplacesTheOldOnceDecodedAndJudged)
{
    showPicture(writePicture(directory, "dark.png", 64, 64, 0xff202020u));
    ASSERT_TRUE(test::runUntil([&] { return wallpaper->isJudged(); }, std::chrono::seconds(10)));
    // Smooth and dark: light contents, no gradient.
    EXPECT_TRUE(wallpaper->style().useLightForeground);
    EXPECT_FALSE(wallpaper->style().gradientVisible);
    EXPECT_FALSE(node()->gradient());
    const uint8_t *dark = node()->picture().constBits();

    // What the node shows whenever the bar hears of a new verdict.
    const uint8_t *shownAtVerdict = nullptr;
    base::Subscription verdicts = wallpaper->judgementChanged.connect([&] {
        shownAtVerdict = node()->picture().constBits();
    });
    showPicture(writePicture(directory, "mid.png", 64, 64, 0xffaaaaaau));
    // Still the old picture and verdict while the new one decodes.
    EXPECT_EQ(dark, node()->picture().constBits());
    EXPECT_TRUE(wallpaper->style().useLightForeground);
    EXPECT_FALSE(wallpaper->isReady());
    ASSERT_TRUE(test::runUntil([&] { return !wallpaper->style().useLightForeground; }, std::chrono::seconds(10)));
    EXPECT_NE(dark, node()->picture().constBits());
    // Picture, verdict and gradient in the same turn.
    EXPECT_EQ(node()->picture().constBits(), shownAtVerdict);
    EXPECT_EQ(0xffaaaaaau, node()->picture().pixel(10, 10));
    ASSERT_TRUE(node()->gradient());
    EXPECT_EQ((ui::Color {255, 255, 255, 115}), node()->gradient()->color);
    EXPECT_EQ(shell::BarBackdrop::kGradientHeight, node()->gradient()->height);
    EXPECT_TRUE(wallpaper->isReady());
}

TEST_F(WallpaperTest, AnUnreadablePictureKeepsWhatIsUp)
{
    showPicture(writePicture(directory, "dark.png", 64, 64, 0xff202020u));
    ASSERT_TRUE(test::runUntil([&] { return wallpaper->isReady(); }, std::chrono::seconds(10)));
    const uint8_t *dark = node()->picture().constBits();
    const shell::BarBackdrop::Style style = wallpaper->style();

    showPicture(directory.filePath("missing.jpg"));
    ASSERT_TRUE(test::runUntil([&] { return wallpaper->isReady(); }, std::chrono::seconds(10)));
    EXPECT_EQ(dark, node()->picture().constBits());
    EXPECT_EQ(style, wallpaper->style());
}

TEST_F(WallpaperTest, WithNothingUpAnUnreadablePictureShowsTheColour)
{
    shell::Wallpaper::Background background;
    background.path = directory.filePath("missing.jpg");
    background.color = ui::Color::rgb(0x336699);
    wallpaper->setBackground(background);
    ASSERT_TRUE(test::runUntil([&] { return wallpaper->isReady(); }, std::chrono::seconds(10)));
    EXPECT_TRUE(node()->picture().isNull());
    EXPECT_FLOAT_EQ(0x33 / 255.0f, node()->color().r);
    EXPECT_FLOAT_EQ(0x99 / 255.0f, node()->color().b);
}

TEST_F(WallpaperTest, AColourOrNothing)
{
    showPicture(writePicture(directory, "dark.png", 64, 64, 0xff202020u));
    ASSERT_TRUE(test::runUntil([&] { return wallpaper->isReady(); }, std::chrono::seconds(10)));

    // A plain colour shows at once, judged as the bar's backdrop is.
    shell::Wallpaper::Background background;
    background.type = 1;
    background.color = ui::Color::rgb(0xaaaaaa);
    wallpaper->setBackground(background);
    EXPECT_TRUE(wallpaper->isReady());
    EXPECT_TRUE(node()->picture().isNull());
    EXPECT_FLOAT_EQ(0xaa / 255.0f, node()->color().g);
    EXPECT_TRUE(wallpaper->isJudged());
    EXPECT_FALSE(wallpaper->style().useLightForeground);
    ASSERT_TRUE(node()->gradient());
}

TEST_F(WallpaperTest, APressOnTheBareOutputDeactivates)
{
    wm::Window *application = openApplication(base::Rect(100, 100, 300, 200));
    ASSERT_TRUE(application->isActive());
    input.onPointerMotionAbsolute(at(10), {1000, 800});
    input.onPointerButton(at(11), BTN_LEFT, true);
    input.onPointerButton(at(12), BTN_LEFT, false);
    EXPECT_FALSE(application->isActive());
    EXPECT_EQ(nullptr, manager.activation()->activeWindow());
}

TEST_F(WallpaperTest, CaptureTakesTheWallpaperWithTheDesktopOverIt)
{
    showPicture(writePicture(directory, "dark.png", 64, 64, 0xff202020u));
    ASSERT_TRUE(test::runUntil([&] { return wallpaper->isReady(); }, std::chrono::seconds(10)));
    // The desktop's icons: a transparent surface over the whole output.
    shell::ShellWindow icons(shell.get(), wm::WindowRole::Desktop);
    icons.setContentView(std::make_unique<ui::View>());
    icons.window()->setBackgroundColor(ui::Color {0, 0, 0, 0});
    icons.setGeometry(kOutput);
    icons.show();
    compositor.updateTextures(renderer);
    wm::Window *desktop = icons.wmWindow();
    ASSERT_NE(nullptr, desktop);
    if (auto *animation = dynamic_cast<scene::WindowAnimation *>(desktop->node()))
        animation->finish();

    const std::shared_ptr<compositor::CaptureSource> source = wallpaper->desktopCaptureSource(desktop, {});
    ASSERT_TRUE(source);
    EXPECT_EQ(compositor::CaptureSource::Kind::Window, source->kind);
    EXPECT_EQ(kOutput, source->bounds);
    EXPECT_DOUBLE_EQ(1.5, source->scale);
    EXPECT_EQ(kDevice, source->pixelSize());
    ASSERT_NE(nullptr, source->node);

    renderer.calls.clear();
    source->node->render(renderer);
    // The picture first, then the desktop's surface over it.
    ASSERT_GE(renderer.calls.size(), 2u);
    EXPECT_EQ(kDevice, renderer.calls.front().texture->size());
    EXPECT_EQ(kOutput, renderer.calls.front().rect);
    EXPECT_NE(renderer.calls.front().texture, renderer.calls.back().texture);
    EXPECT_EQ(nullptr, wallpaper->desktopCaptureSource(desktop, "elsewhere"));
}
