// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include "loop_test_util.h"
#include <linux/input-event-codes.h>

#include <cstdlib>
#include <filesystem>
#include <optional>

#include "base/byte_stream.h"
#include "compositor/compositor.h"
#include "cursor/cursor.h"
#include "input/input_manager.h"
#include "input/seat.h"
#include "input_test_util.h"
#include "scene/scene.h"

using namespace Lingmo;

TEST(CursorTheme, EveryStandardShapeLoadsWithPremultipliedPixels)
{
    for (int value = 1; value <= 34; ++value) {
        for (int size : {24, 32, 48, 64}) {
            const auto frames = cursor::load(static_cast<input::CursorShape>(value), size);
            ASSERT_FALSE(frames.empty()) << value << " size " << size;
            for (const auto &frame : frames) {
                EXPECT_FALSE(frame.image.isNull());
                EXPECT_TRUE(frame.image.rect().contains(frame.hotspot));
                bool visible = false;
                for (int y = 0; y < frame.image.height(); ++y) {
                    for (int x = 0; x < frame.image.width(); ++x) {
                        const uint32_t pixel = frame.image.pixel(x, y);
                        const uint32_t alpha = pixel >> 24;
                        EXPECT_LE((pixel >> 16) & 0xff, alpha);
                        EXPECT_LE((pixel >> 8) & 0xff, alpha);
                        EXPECT_LE(pixel & 0xff, alpha);
                        visible |= alpha != 0;
                    }
                }
                EXPECT_TRUE(visible);
            }
        }
    }
    EXPECT_TRUE(cursor::load(input::CursorShape::Hidden, 24).empty());
}

namespace {
std::string cursorFile()
{
    base::ByteWriter out(std::endian::little);
    for (const uint32_t value : {0x72756358u, 16u, 1u, 1u, 0xfffd0002u, 24u, 28u, 36u, 0xfffd0002u,
                                 24u, 1u, 2u, 1u, 1u, 0u, 37u, 0xff123456u, 0xffabcdefu})
        out.write(value);
    return out.bytes();
}

class CursorSearchTest : public testing::Test
{
protected:
    void SetUp() override { cursor::setThemePathsForTesting({directory.path()}); }

    void TearDown() override { cursor::setThemePathsForTesting({}); }

    input_test::TemporaryDirectory directory;
};

// A cursor sink that keeps a copy of the last image.
class CopyingSink : public render::CursorSink
{
public:
    bool setCursorImage(const render::CursorImage &frame) override
    {
        image = ui::Bitmap::copyOf(frame.pixels, frame.size.width(), frame.size.height(),
                                   size_t(frame.stride), ui::Bitmap::Format::Argb32Premultiplied);
        return true;
    }

    void hideCursor() override { }

    void moveCursor(const base::PointF &) override { }

    ui::Bitmap image;
};
} // namespace

TEST(CursorTheme, ReadsPixelsHotspotAndDelay)
{
    const auto frames = cursor::readXCursor(cursorFile(), 24);
    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].image.size(), base::Size(2, 1));
    EXPECT_EQ(frames[0].image.pixel(0, 0), 0xff123456u);
    EXPECT_EQ(frames[0].image.pixel(1, 0), 0xffabcdefu);
    EXPECT_EQ(frames[0].hotspot, base::Point(1, 0));
    EXPECT_EQ(frames[0].delayMs, 37);
}

TEST(CursorTheme, RejectsTruncatedAndOversizedFiles)
{
    const auto bytes = cursorFile();
    for (size_t size = 0; size < bytes.size(); ++size)
        EXPECT_TRUE(cursor::readXCursor(bytes.substr(0, size), 24).empty());
    auto corrupt = bytes;
    corrupt[12] = char(0xff);
    corrupt[13] = char(0xff);
    EXPECT_TRUE(cursor::readXCursor(corrupt, 24).empty());
    corrupt = bytes;
    corrupt[45] = char(0xff);
    EXPECT_TRUE(cursor::readXCursor(corrupt, 24).empty());
}

TEST_F(CursorSearchTest, LoadsAdwaitaFromSearchPathAndFallsBackToPointer)
{
    ASSERT_TRUE(directory.isValid());
    ASSERT_TRUE(input_test::writeFile(directory.filePath("Adwaita/cursors/left_ptr"), cursorFile()));
    const auto frames = cursor::load(input::CursorShape::Wait, 24);
    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].image.pixel(0, 0), 0xff123456u);
    EXPECT_TRUE(cursor::load(input::CursorShape::Hidden, 24).empty());
}

TEST_F(CursorSearchTest, MissingThemeKeepsPointerVisible)
{
    const auto frames = cursor::load(input::CursorShape::Default, 24);
    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames.front().image.size(), base::Size(24, 24));
    EXPECT_FALSE(frames.front().image.isNull());
}

TEST_F(CursorSearchTest, ResolvesInheritanceAndLegacyShapeAliases)
{
    ASSERT_TRUE(input_test::writeFile(directory.filePath("Child/index.theme"),
                                      "[Icon Theme]\nInherits=Parent\n"));
    ASSERT_TRUE(input_test::writeFile(directory.filePath("Parent/cursors/hand2"), cursorFile()));
    const auto frames = cursor::load(input::CursorShape::Pointer, 24, "Child");
    ASSERT_EQ(frames.size(), 1u);
    EXPECT_EQ(frames[0].image.pixel(0, 0), 0xff123456u);
}

TEST_F(CursorSearchTest, CyclicInheritanceAndInvalidNamesFallBack)
{
    ASSERT_TRUE(input_test::writeFile(directory.filePath("Loop/index.theme"),
                                      "[Icon Theme]\nInherits=Loop\n"));
    EXPECT_FALSE(cursor::load(input::CursorShape::Default, 24, "Loop").empty());
    for (const std::string name : {"../escape", "/absolute", ".."}) {
        EXPECT_FALSE(cursor::validThemeName(name));
        EXPECT_FALSE(cursor::load(input::CursorShape::Default, 24, name).empty());
    }
}

TEST(CursorScene, PhysicalPositionAndOverrideUseIndependentClientLayer)
{
    scene::Scene scene;
    auto *clients = scene.root()->addChild(std::make_unique<scene::Node>());
    compositor::Compositor compositor(nullptr);
    input::Seat seat(&compositor);
    seat.setCursorContainer(clients);
    input::InputManager input(&seat);
    cursor::Cursor cursor(seat, *scene.root(), 24);
    auto *node = scene.root()->children().back();
    input.warpPointer({100, 90});
    EXPECT_EQ(node->position(),
              base::Point(100, 90) - cursor::load(input::CursorShape::Default, 24).front().hotspot);
    EXPECT_TRUE(node->isVisible());
    seat.setCompositorCursorShape(input::CursorShape::Hidden);
    EXPECT_FALSE(node->isVisible());
    seat.setCursorOverride(input::CursorShape::EwResize);
    EXPECT_TRUE(node->isVisible());
    EXPECT_FALSE(clients->isVisible());
    seat.setCursorOverride(std::nullopt);
    EXPECT_TRUE(clients->isVisible());
    EXPECT_FALSE(node->isVisible());
}

TEST(CursorScene, AHeldCursorLeavesThePlaneAloneUntilReleased)
{
    // A compositor starting up keeps off the plane, where the previous one's
    // cursor still is, until its displays settle; then it takes it over once.
    class Sink : public render::CursorSink
    {
    public:
        bool setCursorImage(const render::CursorImage &image) override
        {
            ++images;
            hotspot = image.hotspot;
            return true;
        }

        void hideCursor() override { ++hides; }

        void moveCursor(const base::PointF &position) override
        {
            moves.push_back(position);
        }

        int images = 0;
        int hides = 0;
        base::Point hotspot;
        std::vector<base::PointF> moves;
    } sink;
    scene::Scene scene;
    compositor::Compositor compositor(nullptr);
    input::Seat seat(&compositor);
    input::InputManager input(&seat);
    cursor::Cursor cursor(seat, *scene.root(), 24);
    auto *node = scene.root()->children().back();
    cursor.setHeld(true);
    EXPECT_FALSE(node->isVisible());
    cursor.setSink(&sink);
    cursor.setScale(1.5);
    input.warpPointer({100, 90});
    cursor.refresh();
    EXPECT_EQ(sink.images, 0);
    EXPECT_EQ(sink.hides, 0);
    EXPECT_TRUE(sink.moves.empty());
    EXPECT_FALSE(node->isVisible());
    cursor.setHeld(false);
    EXPECT_EQ(sink.images, 1);
    EXPECT_EQ(sink.hides, 0);
    // What the takeover adds to the old image's corner is what the plane takes off.
    EXPECT_EQ(cursor.deviceHotspot(), sink.hotspot);
    ASSERT_FALSE(sink.moves.empty());
    EXPECT_TRUE(base::fuzzyCompare(sink.moves.back(), base::PointF(100, 90)));
    EXPECT_TRUE(cursor.isHardware());
    EXPECT_FALSE(node->isVisible());
}

TEST_F(CursorSearchTest, SwitchingThemeRefreshesHardwareImageWithoutPointerMotion)
{
    for (const std::string theme : {"First", "Second"}) {
        auto bytes = cursorFile();
        if (theme == "Second")
            bytes[bytes.size() - 8] = char(0x78);
        ASSERT_TRUE(input_test::writeFile(directory.filePath(theme + "/cursors/left_ptr"), bytes));
    }

    CopyingSink sink;

    scene::Scene scene;
    compositor::Compositor compositor(nullptr);
    input::Seat seat(&compositor);
    cursor::Cursor cursor(seat, *scene.root(), 24);
    cursor.setSink(&sink);
    cursor.setTheme("First", 24);
    ASSERT_TRUE(cursor.isHardware());
    EXPECT_EQ(sink.image.pixel(0, 0), 0xff123456u);
    cursor.setTheme("Second", 24);
    EXPECT_EQ(sink.image.pixel(0, 0), 0xff123478u);
    cursor.setTheme("First", 24);
    EXPECT_EQ(sink.image.pixel(0, 0), 0xff123456u);
    cursor.setTheme("Missing", 48);
    EXPECT_EQ(sink.image.size(), base::Size(48, 48));
}

TEST_F(CursorSearchTest, AShapeIsReadOncePerThemeAndSize)
{
    const auto write = [&](const std::string &theme, char blue) {
        auto bytes = cursorFile();
        bytes[bytes.size() - 8] = blue;
        ASSERT_TRUE(input_test::writeFile(directory.filePath(theme + "/cursors/left_ptr"), bytes));
    };
    write("First", char(0x56));
    write("Second", char(0x78));

    CopyingSink sink;

    scene::Scene scene;
    compositor::Compositor compositor(nullptr);
    input::Seat seat(&compositor);
    cursor::Cursor cursor(seat, *scene.root(), 24);
    cursor.setSink(&sink);
    cursor.setTheme("First", 24);
    ASSERT_TRUE(cursor.isHardware());
    EXPECT_EQ(sink.image.pixel(0, 0), 0xff123456u);
    write("First", char(0x9a));
    // A shape not shown yet is read now; one shown before is not read again.
    seat.setCompositorCursorShape(input::CursorShape::Pointer);
    EXPECT_EQ(sink.image.pixel(0, 0), 0xff12349au);
    seat.setCompositorCursorShape(input::CursorShape::Default);
    EXPECT_EQ(sink.image.pixel(0, 0), 0xff123456u);
    cursor.setTheme("Second", 24);
    EXPECT_EQ(sink.image.pixel(0, 0), 0xff123478u);
    cursor.setTheme("First", 24);
    EXPECT_EQ(sink.image.pixel(0, 0), 0xff12349au);
}

TEST(CursorLocator, ShakeEnlargesWithoutMovingTheHotspotAndRestoresHardware)
{
    CopyingSink sink;
    scene::Scene scene;
    compositor::Compositor compositor(nullptr);
    input::Seat seat(&compositor);
    cursor::Cursor cursor(seat, *scene.root(), 24);
    cursor.setSink(&sink);
    seat.updateCursorPosition({100, 100});
    const auto original = cursor.captureBounds();
    ASSERT_TRUE(cursor.isHardware());
    for (int i = 0; i < 7; ++i) seat.updateCursorPosition({100.0 + (i % 2) * 80, 100});
    ASSERT_TRUE(test::runUntil([&] { return cursor.locatorScale() == 2.5; }));
    EXPECT_FALSE(cursor.isHardware());
    EXPECT_EQ(seat.cursorPosition(), base::PointF(100, 100));
    const auto enlarged = cursor.captureBounds();
    EXPECT_NEAR(enlarged.width(), original.width() * 2.5, 0.1);
    // Scale about the hotspot rather than moving the actual pointer.
    EXPECT_NEAR(100 - enlarged.x(), (100 - original.x()) * 2.5, 0.1);
    ASSERT_TRUE(test::runUntil([&] { return cursor.locatorScale() == 1; }));
    EXPECT_TRUE(cursor.isHardware());
    EXPECT_EQ(cursor.captureBounds(), original);
    cursor.setSink(nullptr);
}

TEST(CursorLocator, DisableAndTouchResetTheEffectAndDraggingDoesNotTrigger)
{
    scene::Scene scene;
    compositor::Compositor compositor(nullptr);
    input::Seat seat(&compositor);
    cursor::Cursor cursor(seat, *scene.root(), 24);
    cursor.setShakeToLocate(false);
    for (int i = 0; i < 8; ++i) seat.updateCursorPosition({100.0 + i%2*80, 100});
    test::runFor(std::chrono::milliseconds(150));
    EXPECT_EQ(cursor.locatorScale(), 1);
    cursor.setShakeToLocate(true);
    seat.updatePointerButtonState(BTN_LEFT, true);
    for (int i = 0; i < 8; ++i) seat.updateCursorPosition({100.0 + i%2*80, 100});
    test::runFor(std::chrono::milliseconds(150));
    EXPECT_EQ(cursor.locatorScale(), 1);
    seat.updatePointerButtonState(BTN_LEFT, false);
    for (int i = 0; i < 8; ++i) seat.updateCursorPosition({100.0 + i%2*80, 100});
    ASSERT_TRUE(test::runUntil([&] { return cursor.locatorScale() > 1; }));
    cursor.setShakeToLocate(false);
    EXPECT_EQ(cursor.locatorScale(), 1);
    cursor.setShakeToLocate(true);
    for (int i = 0; i < 8; ++i) seat.updateCursorPosition({100.0 + i%2*80, 100});
    ASSERT_TRUE(test::runUntil([&] { return cursor.locatorScale() > 1; }));
    seat.setCursorHiddenByTouch(true);
    EXPECT_EQ(cursor.locatorScale(), 1);
    EXPECT_TRUE(cursor.captureBounds().isEmpty());
}
