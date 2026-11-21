// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "durationprobe.h"
#include "player.h"
#include "softwarevideo.h"
#include "thumbnailer.h"
#include "urls.h"
#include "waitfor.h"

#include "base/elapsed_timer.h"
#include "base/event_loop.h"
#include "base/file.h"
#include "base/paths.h"
#include "ui/bitmap.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>

namespace base = Lingmo::base;
namespace ui = Lingmo::ui;
namespace fs = std::filesystem;
using namespace std::chrono_literals;

static std::string quoted(const std::string& text)
{
    std::string result = "'";
    for (const char c : text)
        result += c == '\'' ? std::string("'\\''") : std::string(1, c);
    return result + "'";
}

static bool ffmpeg(const std::vector<std::string>& arguments)
{
    std::string command = "ffmpeg -v error -y";
    for (const std::string& argument : arguments)
        command += ' ' + quoted(argument);
    return std::system(command.c_str()) == 0;
}

static int litSamples(const ui::Bitmap& image)
{
    int lit = 0;
    for (int y = 0; y < image.height(); y += 10) {
        for (int x = 0; x < image.width(); x += 10) {
            const uint32_t pixel = image.pixel(x, y);
            const int gray = (int((pixel >> 16) & 0xff) * 11 + int((pixel >> 8) & 0xff) * 16 + int(pixel & 0xff) * 5) / 32;
            lit += gray > 40;
        }
    }
    return lit;
}

// A player with the software video output a window would give it, drawn at
// 320x240 whenever mpv has a frame.
struct TestPlayer
{
    TestPlayer()
    {
        subscriptions.push_back(player.renderWanted.connect([this] { attach(); }));
    }

    void attach()
    {
        if (!video) {
            video = std::make_unique<SoftwareVideo>(player);
            frames = video->frameDue.connect([this] { video->render(320, 240); });
        }
        video->ensureContext();
    }

    void detach()
    {
        frames.reset();
        video.reset();
    }

    Player player;
    std::unique_ptr<SoftwareVideo> video;
    base::Subscription frames;
    std::vector<base::Subscription> subscriptions;
};

class MediaTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        s_dir = new base::file::TemporaryDirectory(fs::temp_directory_path().string(), "video-player-media");
        if (base::paths::findExecutable("ffmpeg").empty())
            return;
        const std::string metadata = path("chapters.txt");
        std::ofstream(metadata) << ";FFMETADATA1\n"
                                   "[CHAPTER]\nTIMEBASE=1/1000\nSTART=0\nEND=4000\ntitle=Opening\n"
                                   "[CHAPTER]\nTIMEBASE=1/1000\nSTART=4000\nEND=8000\ntitle=Finale\n";
        const std::string subtitle = path("embedded.srt");
        std::ofstream(subtitle) << "1\n00:00:01,000 --> 00:00:03,000\nHello\n";
        s_made = ffmpeg({"-f", "lavfi", "-i", "testsrc=size=320x240:rate=25:duration=8", "-i", subtitle, "-i", metadata,
                         "-map", "0", "-map", "1", "-map_metadata", "2", "-map_chapters", "2",
                         "-metadata:s:s:0", "language=eng", "-c:v", "libx264", "-g", "25", "-pix_fmt", "yuv420p",
                         "-c:s", "srt", path("clip.mkv")});
        // Found next to the video by its name.
        std::error_code error;
        fs::copy_file(subtitle, path("clip.zh.srt"), error);
        fs::copy_file(path("clip.mkv"), path("other.mkv"), error);
    }

    static void TearDownTestSuite()
    {
        delete s_dir;
        s_dir = nullptr;
    }

    void SetUp() override
    {
        if (!s_made)
            GTEST_SKIP() << "ffmpeg is needed to make the test clips";
    }

    static std::string path(const std::string& name) { return (fs::path(s_dir->path()) / name).string(); }
    static std::string clip() { return urls::fromLocalFile(path("clip.mkv")); }

    bool waitFor(auto done, std::chrono::milliseconds timeout = 5s) { return ::waitFor(m_loop, done, timeout); }

    // Waits for the next fileLoaded of player.
    bool waitForLoad(Player& player, std::chrono::milliseconds timeout = 10s)
    {
        bool loaded = false;
        auto subscription = player.fileLoaded.connect([&] { loaded = true; });
        return waitFor([&] { return loaded; }, timeout);
    }

    base::EventLoop m_loop;
    static inline base::file::TemporaryDirectory* s_dir = nullptr;
    static inline bool s_made = false;
};

TEST_F(MediaTest, LoadsTracksAndChapters)
{
    TestPlayer test;
    Player& player = test.player;
    bool loaded = false;
    auto subscription = player.fileLoaded.connect([&] { loaded = true; });
    player.load(clip(), {}, true);
    ASSERT_TRUE(waitFor([&] { return loaded; }, 10s));
    EXPECT_TRUE(waitFor([&] { return std::lround(player.duration()) == 8; }));
    EXPECT_TRUE(player.loaded());
    EXPECT_TRUE(player.seekable());
    EXPECT_TRUE(waitFor([&] { return player.hasVideo(); }));
    EXPECT_EQ(player.subtitleTracks().size(), 2u);
    EXPECT_TRUE(player.audioTracks().empty());
    ASSERT_TRUE(waitFor([&] { return player.chapters().size() == 2; }));
    EXPECT_EQ(player.chapters()[1].title, "Finale");
    EXPECT_EQ(player.chapters()[1].time, 4.0);

    const MpvNode info(player.mediaInfo());
    EXPECT_NE(info["video-codec"].toString().find("264"), std::string::npos);
    EXPECT_EQ(info["width"].toInt(), 320);

    const int id = player.subtitleTracks().back().id;
    player.setSubtitleTrack(id);
    EXPECT_TRUE(waitFor([&] { return player.subtitleTrack() == id; }, 3s));
    player.setSubtitleTrack(0);
    EXPECT_TRUE(waitFor([&] { return player.subtitleTrack() == 0; }, 3s));

    player.setChapter(1);
    EXPECT_TRUE(waitFor([&] { return player.position() >= 3.9; }));
    EXPECT_TRUE(waitFor([&] { return player.chapter() == 1; }, 3s));
}

TEST_F(MediaTest, SeeksAndEnds)
{
    TestPlayer test;
    Player& player = test.player;
    player.load(clip(), {}, true);
    ASSERT_TRUE(waitForLoad(player));
    EXPECT_TRUE(waitFor([&] { return player.paused(); }));

    player.seek(5.0, true);
    EXPECT_TRUE(waitFor([&] { return std::abs(player.position() - 5.0) < 0.1; }));
    player.seekRelative(-2);
    EXPECT_TRUE(waitFor([&] { return std::abs(player.position() - 3.0) < 0.1; }));

    player.setSpeed(2.0);
    EXPECT_TRUE(waitFor([&] { return player.speed() == 2.0; }));
    player.setVolume(130);
    EXPECT_TRUE(waitFor([&] { return player.volume() == 130; }));
    player.setMuted(true);
    EXPECT_TRUE(waitFor([&] { return player.muted(); }));

    // Kept on the last frame and paused at the end.
    player.setPaused(false);
    EXPECT_TRUE(waitFor([&] { return player.endReached(); }, 10s));
    EXPECT_TRUE(waitFor([&] { return player.paused(); }));
    EXPECT_TRUE(player.loaded());

    // Playing again starts over.
    player.setPaused(false);
    EXPECT_TRUE(waitFor([&] { return !player.endReached() && player.position() < 3; }));

    // A new file starts at normal speed.
    player.load(clip(), {}, true);
    ASSERT_TRUE(waitForLoad(player));
    EXPECT_TRUE(waitFor([&] { return player.speed() == 1.0; }));
}

TEST_F(MediaTest, ResumesWithFileOptions)
{
    TestPlayer test;
    Player& player = test.player;
    player.load(clip(), {{"start", 6.5}, {"sub-delay", 0.4}}, true);
    ASSERT_TRUE(waitForLoad(player));
    EXPECT_TRUE(waitFor([&] { return std::abs(player.position() - 6.5) < 0.2; }));
    EXPECT_TRUE(waitFor([&] { return std::abs(player.subtitleDelay() - 0.4) < 0.001; }));
}

TEST_F(MediaTest, Screenshot)
{
    TestPlayer test;
    Player& player = test.player;
    player.load(clip(), {{"start", 2}}, true);
    ASSERT_TRUE(waitForLoad(player));
    EXPECT_TRUE(waitFor([&] { return std::abs(player.position() - 2) < 0.1 && !player.buffering(); }));
    std::optional<std::pair<std::string, bool>> saved;
    auto subscription = player.screenshotSaved.connect([&](const std::string& file, bool success) {
        saved.emplace(file, success);
    });
    const std::string shot = path("shot.png");
    player.screenshot(shot);
    ASSERT_TRUE(waitFor([&] { return saved.has_value(); }, 10s));
    EXPECT_EQ(saved->first, shot);
    EXPECT_TRUE(saved->second);
    // At the clip's own resolution, not the output's.
    const ui::Bitmap image = ui::Bitmap::decodeFile(shot);
    EXPECT_EQ(image.width(), 320);
    EXPECT_EQ(image.height(), 240);
}

TEST_F(MediaTest, DrawsFrames)
{
    TestPlayer test;
    Player& player = test.player;
    player.load(clip(), {{"start", 2}}, true);
    ASSERT_TRUE(waitForLoad(player));
    ASSERT_TRUE(waitFor([&] { return player.videoActive(); }));
    runFor(m_loop, 200ms);
    const ui::Image& frame = test.video->render(320, 240);
    ui::Bitmap bitmap(320, 240, ui::Bitmap::Format::Argb32Premultiplied);
    for (int y = 0; y < 240; ++y) {
        for (int x = 0; x < 320; ++x)
            bitmap.setPixel(x, y, frame.pixel(x, y).argb());
    }
    EXPECT_GT(litSamples(bitmap), 300);
}

TEST_F(MediaTest, RecoversLostRenderContext)
{
    TestPlayer test;
    Player& player = test.player;
    player.load(clip(), {{"start", 2.0}}, false);
    ASSERT_TRUE(waitForLoad(player));
    ASSERT_TRUE(waitFor([&] { return player.videoActive(); }));
    EXPECT_TRUE(waitFor([&] { return player.subtitleTrack() > 0; }));

    // Without its output, mpv drops its video output.
    test.detach();
    ASSERT_TRUE(waitFor([&] { return !player.videoActive(); }));
    const double position = player.position();
    int failed = 0;
    int reloaded = 0;
    auto failures = player.playbackFailed.connect([&](const std::string&) { ++failed; });
    auto loads = player.fileLoaded.connect([&] { ++reloaded; });
    test.attach();
    EXPECT_TRUE(waitFor([&] { return player.videoActive(); }));
    EXPECT_TRUE(player.loaded());
    // Back where it was, quietly: no failure, no new file for the window.
    EXPECT_GE(player.position(), position - 0.5);
    EXPECT_EQ(failed, 0);
    EXPECT_EQ(reloaded, 0);
    EXPECT_GT(player.subtitleTrack(), 0);
}

// Recovery from the lost output is over once another file is opened.
TEST_F(MediaTest, LoadsAnotherFileAfterTheRenderContextCameBack)
{
    TestPlayer test;
    Player& player = test.player;
    player.load(clip(), {}, false);
    ASSERT_TRUE(waitForLoad(player));
    ASSERT_TRUE(waitFor([&] { return player.videoActive(); }));

    test.detach();
    ASSERT_TRUE(waitFor([&] { return !player.videoActive(); }));
    test.attach();
    int failed = 0;
    auto failures = player.playbackFailed.connect([&](const std::string&) { ++failed; });
    const std::string other = urls::fromLocalFile(path("other.mkv"));
    player.load(other, {}, false);
    ASSERT_TRUE(waitForLoad(player));
    EXPECT_EQ(player.source(), other);
    EXPECT_TRUE(waitFor([&] { return player.videoActive(); }));
    EXPECT_EQ(failed, 0);

    // Lost again with the new file open: the new file is what comes back.
    test.detach();
    ASSERT_TRUE(waitFor([&] { return !player.videoActive(); }));
    test.attach();
    EXPECT_TRUE(waitFor([&] { return player.videoActive(); }));
    EXPECT_EQ(player.source(), other);
    EXPECT_EQ(failed, 0);
}

// Destroying the player while mpv still has events and frames to deliver.
TEST_F(MediaTest, PlayerGoesWhilePlaying)
{
    for (int i = 0; i < 5; ++i) {
        auto test = std::make_unique<TestPlayer>();
        test->player.load(clip(), {}, false);
        ASSERT_TRUE(waitForLoad(test->player));
        test->player.setSpeed(4.0);
        runFor(m_loop, std::chrono::milliseconds(20 * i));
        test.reset();
        runFor(m_loop, 10ms);
    }
}

TEST_F(MediaTest, ReportsFailure)
{
    TestPlayer test;
    Player& player = test.player;
    std::optional<std::string> failure;
    auto subscription = player.playbackFailed.connect([&](const std::string& message) { failure = message; });
    player.load(urls::fromLocalFile(path("missing.mkv")));
    ASSERT_TRUE(waitFor([&] { return failure.has_value(); }, 10s));
    EXPECT_FALSE(failure->empty());
    EXPECT_FALSE(player.loaded());
}

TEST_F(MediaTest, Thumbnails)
{
    Thumbnailer thumbnailer(path("covers"));
    thumbnailer.setSource(clip());
    thumbnailer.setDuration(8);
    const int bucket = thumbnailer.bucket(4.2);
    EXPECT_EQ(bucket, 4);
    thumbnailer.request(bucket);
    ASSERT_TRUE(waitFor([&] { return thumbnailer.has(bucket); }, 10s));
    const ui::Image image = thumbnailer.image(bucket);
    EXPECT_EQ(image.width(), 240);
    EXPECT_EQ(image.height(), 180);
    // testsrc is colourful everywhere; a black frame means nothing was drawn.
    ui::Bitmap bitmap(image.width(), image.height(), ui::Bitmap::Format::Argb32Premultiplied);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x)
            bitmap.setPixel(x, y, image.pixel(x, y).argb());
    }
    EXPECT_GT(litSamples(bitmap), 100);

    // A new file drops the old frames.
    thumbnailer.setSource({});
    EXPECT_FALSE(thumbnailer.has(bucket));

    // Quitting mid-work does not wait out the worker's timeouts.
    for (int i = 0; i < 3; ++i) {
        auto busy = std::make_unique<Thumbnailer>(path("covers"));
        busy->setSource(clip());
        busy->setDuration(8);
        busy->request(busy->bucket(1.0 + i));
        runFor(m_loop, std::chrono::milliseconds(i * 15));
        base::ElapsedTimer timer;
        timer.start();
        busy.reset();
        EXPECT_LT(timer.elapsedMs(), 1000);
    }
}

TEST_F(MediaTest, Cover)
{
    // Black at first, so a frame from the file's start shows up as one.
    const std::string dark = path("dark-start.mkv");
    ASSERT_TRUE(ffmpeg({"-filter_complex",
                        "color=c=black:size=320x240:rate=25:duration=1[a];"
                        "testsrc=size=320x240:rate=25:duration=19[b];[a][b]concat",
                        "-c:v", "libx264", "-g", "25", "-pix_fmt", "yuv420p", dark}));

    auto thumbnailer = std::make_unique<Thumbnailer>(path("covers"));
    // Either order: the length often arrives after the file.
    thumbnailer->setDuration(20);
    thumbnailer->setSource(urls::fromLocalFile(dark));
    ASSERT_TRUE(waitFor([&] { return urls::isLocalFile(thumbnailer->cover()); }, 10s));
    const std::string first = urls::toLocalFile(thumbnailer->cover());
    const ui::Bitmap image = ui::Bitmap::decodeFile(first);
    EXPECT_EQ(image.width(), 180);
    EXPECT_EQ(image.height(), 180);
    EXPECT_GT(litSamples(image), 100);

    // Each file gets its own name, and the old one goes.
    thumbnailer->setSource({});
    EXPECT_TRUE(thumbnailer->cover().empty());
    EXPECT_FALSE(fs::exists(first));
    thumbnailer->setSource(urls::fromLocalFile(dark));
    ASSERT_TRUE(waitFor([&] { return urls::isLocalFile(thumbnailer->cover()); }, 10s));
    const std::string second = urls::toLocalFile(thumbnailer->cover());
    EXPECT_NE(second, first);
    thumbnailer.reset();
    EXPECT_FALSE(fs::exists(second));

    // An audio file's embedded picture.
    const std::string picture = path("picture.png");
    const std::string song = path("song.mp3");
    ASSERT_TRUE(ffmpeg({"-f", "lavfi", "-i", "color=c=red:size=300x300:duration=1", "-frames:v", "1", picture}));
    ASSERT_TRUE(ffmpeg({"-f", "lavfi", "-i", "sine=frequency=440:duration=3", "-i", picture, "-map", "0", "-map",
                        "1", "-c:a", "libmp3lame", "-c:v", "png", "-disposition:v", "attached_pic", song}));
    Thumbnailer audio(path("covers"));
    audio.setSource(urls::fromLocalFile(song));
    audio.setDuration(3);
    ASSERT_TRUE(waitFor([&] { return urls::isLocalFile(audio.cover()); }, 10s));
    const ui::Bitmap art = ui::Bitmap::decodeFile(urls::toLocalFile(audio.cover()));
    EXPECT_EQ(art.width(), 240);
    EXPECT_EQ(art.height(), 240);
    const uint32_t centre = art.pixel(120, 120);
    EXPECT_GT((centre >> 16) & 0xff, 200u);
    EXPECT_LT((centre >> 8) & 0xff, 60u);
}

TEST_F(MediaTest, ProbesDurations)
{
    const std::string broken = path("broken.mkv");
    std::ofstream(broken) << "not a video";

    const std::string bad = urls::fromLocalFile(broken);
    const std::string stream = "https://example.com/live.m3u8";
    DurationProbe probe;
    std::vector<std::pair<std::string, double>> found;
    auto subscription = probe.found.connect([&](const std::string& url, double seconds) { found.emplace_back(url, seconds); });
    probe.probe({bad, stream, clip()});
    ASSERT_TRUE(waitFor([&] { return found.size() == 2; }, 15s));
    EXPECT_EQ(found[0].first, bad);
    EXPECT_EQ(found[0].second, 0.0);
    EXPECT_EQ(found[1].first, clip());
    EXPECT_NEAR(found[1].second, 8, 0.1);

    // The same mpv reads file after file.
    probe.probe({clip()});
    ASSERT_TRUE(waitFor([&] { return found.size() == 3; }, 10s));
    EXPECT_NEAR(found[2].second, 8, 0.1);
    runFor(m_loop, 200ms);
    EXPECT_EQ(found.size(), 3u);
}

TEST_F(MediaTest, ProbesVideoSize)
{
    const Player::Size size = Player::probeVideoSize(clip(), 2000ms);
    EXPECT_EQ(size.width, 320);
    EXPECT_EQ(size.height, 240);
    EXPECT_TRUE(Player::probeVideoSize("https://example.com/a.mp4", 100ms).isEmpty());
}
