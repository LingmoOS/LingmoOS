// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <sys/stat.h>
#include <thread>

#include "base/elapsed_timer.h"
#include "base/strings.h"
#include "lingmo/files/mime_sniffer_p.h"
#include "lingmo/files/thumbnails/md5_p.h"
#include "lingmo/files/thumbnails/thumbnail_cache.h"
#include "lingmo/files/url.h"
#include "testing.h"

using namespace Lingmo;
using namespace Lingmo::Files;
using namespace Lingmo::Files::testing;
using namespace std::chrono_literals;

static std::string localUrl(const std::string& path)
{
    return Url::fromLocalFile(path).toString();
}

static void savePng(const std::string& path, int width, int height, uint32_t argb)
{
    ui::Bitmap image(width, height, ui::Bitmap::Format::Argb32Premultiplied);
    image.fill(argb);
    ASSERT_TRUE(image.savePng(path));
}

class ThumbnailersTest : public ::testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        s_tools = new TemporaryDirectory;
        s_files = new TemporaryDirectory;
        s_data = new TemporaryDirectory;
        s_thumbnailers = s_data->filePath("thumbnailers");
        s_cache = s_data->filePath("cache/thumbnails");
        ASSERT_TRUE(makePath(s_thumbnailers));
        internal::setUserDataDirectory(s_data->path());
        internal::setThumbnailCacheRoot(s_cache);

        savePng(s_tools->filePath("source.png"), 300, 150, 0xffff0000);

        // Records its arguments and writes the source image to the last one.
        const std::string script = s_tools->filePath("thumbnail.sh");
        writeFile(script, "#!/bin/sh\nprintf '%s\\n' \"$@\" > " + s_tools->filePath("arguments")
                              + "\nfor last; do :; done\ncp " + s_tools->filePath("source.png") + " \"$last\"\n");
        ASSERT_EQ(::chmod(script.c_str(), 0700), 0);

        installThumbnailer("record.thumbnailer",
                           "TryExec=" + script + "\nExec=" + script + " -s %s %u literal%% %o\nMimeType=text/plain;\n");
        installThumbnailer("fail.thumbnailer", "Exec=false %i %o\nMimeType=text/csv;\n");
        installThumbnailer("hang.thumbnailer", "Exec=sh -c \"sleep 63\" sh %i %o\nMimeType=text/x-csrc;\n");
        installThumbnailer("missing.thumbnailer",
                           "TryExec=/nonexistent/thumbnailer\nExec=/nonexistent/thumbnailer %i %o\n"
                           "MimeType=application/json;\n");
    }

    static void TearDownTestSuite()
    {
        internal::setUserDataDirectory({});
        internal::setThumbnailCacheRoot({});
        delete s_tools;
        delete s_files;
        delete s_data;
    }

    static void installThumbnailer(const std::string& name, const std::string& body)
    {
        writeFile(s_thumbnailers + '/' + name, "[Thumbnailer Entry]\n" + body);
    }

    static TemporaryDirectory* s_tools;
    static TemporaryDirectory* s_files;
    static TemporaryDirectory* s_data;
    static std::string s_thumbnailers;
    static std::string s_cache;
};

TemporaryDirectory* ThumbnailersTest::s_tools = nullptr;
TemporaryDirectory* ThumbnailersTest::s_files = nullptr;
TemporaryDirectory* ThumbnailersTest::s_data = nullptr;
std::string ThumbnailersTest::s_thumbnailers;
std::string ThumbnailersTest::s_cache;

TEST_F(ThumbnailersTest, RunsInstalledThumbnailer)
{
    EXPECT_TRUE(ThumbnailCache::self()->canThumbnail("text/plain"));

    const std::string note = s_files->filePath("a note.txt");
    writeFile(note, "text");
    const Thumbnail thumbnail = ThumbnailCache::self()->requestThumbnail(localUrl(note), {64, 64});

    // The normal flavour is 128 pixels; the larger output is scaled into it.
    EXPECT_EQ(thumbnail.image.width(), 128);
    EXPECT_EQ(thumbnail.image.height(), 64);
    EXPECT_TRUE(thumbnail.path.starts_with(s_cache + "/normal/"));
    EXPECT_TRUE(exists(thumbnail.path));

    const std::vector<std::string> arguments =
        base::strings::split(readFile(s_tools->filePath("arguments")), '\n', base::strings::SplitBehavior::SkipEmptyParts);
    ASSERT_EQ(arguments.size(), 5u);
    EXPECT_EQ(arguments[0], "-s");
    EXPECT_EQ(arguments[1], "128");
    EXPECT_EQ(arguments[2], "file://" + s_files->path() + "/a%20note.txt");
    EXPECT_EQ(arguments[3], "literal%");
    EXPECT_TRUE(arguments[4].ends_with("/thumbnail.png"));
    EXPECT_FALSE(arguments[4].starts_with(s_cache));

    // A second request reads the cache instead of running the thumbnailer again.
    ::unlink(s_tools->filePath("arguments").c_str());
    const Thumbnail cached = ThumbnailCache::self()->requestThumbnail(localUrl(note), {64, 64});
    EXPECT_EQ(cached.path, thumbnail.path);
    EXPECT_EQ(cached.image.width(), 128);
    EXPECT_FALSE(exists(s_tools->filePath("arguments")));
}

TEST_F(ThumbnailersTest, ImagesStayInProcess)
{
    EXPECT_TRUE(ThumbnailCache::self()->canThumbnail("image/png"));

    const std::string image = s_files->filePath("image.png");
    savePng(image, 40, 20, 0xff0000ff);

    ::unlink(s_tools->filePath("arguments").c_str());
    const Thumbnail thumbnail = ThumbnailCache::self()->requestThumbnail(localUrl(image), {64, 64});
    EXPECT_EQ(thumbnail.image.width(), 128);
    EXPECT_EQ(thumbnail.image.height(), 64);
    EXPECT_FALSE(exists(s_tools->filePath("arguments")));
}

TEST_F(ThumbnailersTest, CachedThumbnailsAreReadWithoutGenerating)
{
    const std::string image = s_files->filePath("served.png");
    savePng(image, 40, 20, 0xffff0000);

    // As a view names it, with a query that keeps its key fresh.
    const std::string id = localUrl(image) + "?mtime=1";
    EXPECT_FALSE(ThumbnailCache::self()->hasCachedThumbnail(id, {96, 96}));
    EXPECT_TRUE(ThumbnailCache::self()->cachedThumbnail(id, {96, 96}).image.isNull());

    EXPECT_FALSE(ThumbnailCache::self()->requestThumbnail(localUrl(image), {96, 96}).image.isNull());
    EXPECT_TRUE(ThumbnailCache::self()->hasCachedThumbnail(id, {96, 96}));
    EXPECT_FALSE(ThumbnailCache::self()->hasCachedThumbnail(id, {200, 200}));
    const Thumbnail served = ThumbnailCache::self()->cachedThumbnail(id, {120, 100});
    EXPECT_EQ(served.image.width(), 128);
    EXPECT_EQ(served.image.height(), 64);
    EXPECT_EQ(served.image.pixel(0, 0), 0xffff0000u);

    // A changed file leaves a stale entry behind, which is not served.
    ASSERT_EQ(::truncate(image.c_str(), fileSize(image) + 1), 0);
    EXPECT_TRUE(ThumbnailCache::self()->cachedThumbnail(id, {96, 96}).image.isNull());

    // Nor is one older than the file offered at all.
    const timespec times[] = {{0, UTIME_OMIT}, {::time(nullptr) + 60, 0}};
    ASSERT_EQ(::utimensat(AT_FDCWD, image.c_str(), times, 0), 0);
    EXPECT_FALSE(ThumbnailCache::self()->hasCachedThumbnail(id, {96, 96}));
}

TEST_F(ThumbnailersTest, TruncatedCacheEntryIsNotServed)
{
    const std::string image = s_files->filePath("truncated.png");
    savePng(image, 40, 20, 0xff00ff00);
    ASSERT_FALSE(ThumbnailCache::self()->requestThumbnail(localUrl(image), {96, 96}).image.isNull());

    // Cut inside the pixel data, so decoding fails while the rows are read.
    const std::string entry = s_cache + "/normal/" + internal::md5Hex(localUrl(image)) + ".png";
    const size_t data = readFile(entry).find("IDAT");
    ASSERT_NE(data, std::string::npos);
    ASSERT_EQ(::truncate(entry.c_str(), off_t(data + 8)), 0);

    EXPECT_TRUE(ThumbnailCache::self()->cachedThumbnail(localUrl(image), {96, 96}).image.isNull());
}

TEST_F(ThumbnailersTest, FailureIsRemembered)
{
    EXPECT_TRUE(ThumbnailCache::self()->canThumbnail("text/csv"));

    const std::string table = s_files->filePath("table.csv");
    writeFile(table, "a,b\n");
    const Thumbnail thumbnail = ThumbnailCache::self()->requestThumbnail(localUrl(table), {64, 64});
    EXPECT_TRUE(thumbnail.image.isNull());

    const std::string name = internal::md5Hex(localUrl(table));
    EXPECT_TRUE(exists(s_cache + "/fail/lingmo/" + name + ".png"));
}

TEST_F(ThumbnailersTest, HangingThumbnailerIsStopped)
{
    const std::string script = s_files->filePath("source.c");
    writeFile(script, "int x;\n");

    base::ElapsedTimer timer;
    timer.start();
    const Thumbnail thumbnail = ThumbnailCache::self()->requestThumbnail(localUrl(script), {64, 64});
    EXPECT_TRUE(thumbnail.image.isNull());
    EXPECT_GE(timer.elapsedMs(), 9000);
    EXPECT_LT(timer.elapsedMs(), 15000);

    // The child of the wrapper shell goes with it.
    const std::string pgrep = "pgrep -u " + std::to_string(::getuid()) + " -fx 'sleep 63' >/dev/null";
    bool gone = false;
    for (int i = 0; i < 50 && !gone; ++i) {
        gone = WEXITSTATUS(std::system(pgrep.c_str())) == 1;
        if (!gone)
            std::this_thread::sleep_for(100ms);
    }
    EXPECT_TRUE(gone);
}

TEST_F(ThumbnailersTest, SkipsMissingPrograms)
{
    EXPECT_FALSE(ThumbnailCache::self()->canThumbnail("application/json"));
    EXPECT_FALSE(ThumbnailCache::self()->canThumbnail("application/x-lingmo-unknown"));
}

TEST_F(ThumbnailersTest, PicksUpNewThumbnailers)
{
    EXPECT_FALSE(ThumbnailCache::self()->canThumbnail("text/markdown"));
    // Directory times tick every few milliseconds; a file added in the same
    // tick as the scan would leave the directory's time unchanged.
    std::this_thread::sleep_for(20ms);
    installThumbnailer("markdown.thumbnailer", "Exec=true %i %o\nMimeType=text/markdown;\n");

    bool found = false;
    for (int i = 0; i < 60 && !found; ++i) {
        found = ThumbnailCache::self()->canThumbnail("text/markdown");
        if (!found)
            std::this_thread::sleep_for(100ms);
    }
    EXPECT_TRUE(found);
}
