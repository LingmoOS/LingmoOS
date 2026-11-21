// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include "languages.h"
#include "mpvnode.h"
#include "playersettings.h"
#include "playlist.h"
#include "resumestore.h"
#include "tracks.h"
#include "urls.h"

#include "base/event_loop.h"
#include "base/file.h"
#include "base/settings.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace base = Lingmo::base;
namespace fs = std::filesystem;

class BackendTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(m_dir.isValid());
    }

    std::string touch(const std::string& name)
    {
        const fs::path path = fs::path(m_dir.path()) / name;
        fs::create_directories(path.parent_path());
        std::ofstream(path.string()).flush();
        return path.string();
    }

    std::string filePath(const std::string& name) const { return (fs::path(m_dir.path()) / name).string(); }
    std::string settingsFile() const { return filePath("video-player.conf"); }

    base::EventLoop m_loop;
    base::file::TemporaryDirectory m_dir{fs::temp_directory_path().string(), "video-player-test"};
};

static std::string url(const std::string& path)
{
    return urls::fromLocalFile(path);
}

TEST_F(BackendTest, FolderPlaylistNaturalOrder)
{
    touch("Episode 10.mkv");
    const std::string second = touch("Episode 2.mkv");
    touch("episode 1.mp4");
    touch("notes.txt");
    touch("Episode 2.srt");
    touch("soundtrack.FLAC");

    Playlist playlist;
    int current = 0;
    auto subscription = playlist.currentIndexChanged.connect([&] { ++current; });
    playlist.open({url(second)});

    std::vector<std::string> titles;
    for (const std::string& entry : playlist.urls())
        titles.push_back(Playlist::titleFor(entry));
    EXPECT_EQ(titles, (std::vector<std::string>{"episode 1.mp4", "Episode 2.mkv", "Episode 10.mkv", "soundtrack.FLAC"}));
    EXPECT_EQ(playlist.currentIndex(), 1);
    EXPECT_EQ(playlist.currentUrl(), url(second));
    EXPECT_EQ(current, 1);
}

TEST_F(BackendTest, OpenWithoutFolder)
{
    touch("a.mkv");
    const std::string b = touch("b.mkv");
    Playlist playlist;
    playlist.open({url(b)}, false);
    EXPECT_EQ(playlist.count(), 1);
    EXPECT_EQ(playlist.currentIndex(), 0);

    // A file the folder scan would not list, such as an unknown suffix, is still played.
    const std::string odd = touch("clip.xyz");
    playlist.open({url(odd)});
    EXPECT_EQ(playlist.currentUrl(), url(odd));
}

TEST_F(BackendTest, OpenSeveralKeepsOrder)
{
    const std::string stream = "https://example.com/live.m3u8";
    const std::string file = url(touch("z.mkv"));
    Playlist playlist;
    playlist.open({stream, file, stream});
    EXPECT_EQ(playlist.urls(), (std::vector<std::string>{stream, file}));
    EXPECT_EQ(playlist.currentIndex(), 0);
    EXPECT_EQ(Playlist::titleFor(stream), "live.m3u8");
}

TEST_F(BackendTest, DurationsAreKept)
{
    const std::string a = url(touch("d1.mkv"));
    const std::string b = url(touch("d2.mkv"));
    Playlist playlist;
    playlist.open({a, b}, false);
    EXPECT_EQ(playlist.unprobed(), (std::vector<std::string>{a, b}));

    std::vector<int> changed;
    auto subscription = playlist.rowChanged.connect([&](int row) { changed.push_back(row); });
    playlist.setDuration(b, 61.5);
    ASSERT_EQ(changed.size(), 1u);
    EXPECT_EQ(changed[0], 1);
    EXPECT_EQ(playlist.duration(1), 61.5);
    EXPECT_EQ(playlist.duration(0), 0.0);
    // A file without a length counts as read.
    playlist.setDuration(a, 0);
    EXPECT_TRUE(playlist.unprobed().empty());
    playlist.setDuration(b, 61.5);
    EXPECT_EQ(changed.size(), 2u);

    // A new queue keeps what was read before.
    playlist.open({b}, false);
    EXPECT_EQ(playlist.duration(0), 61.5);
    EXPECT_TRUE(playlist.unprobed().empty());
}

TEST_F(BackendTest, NavigationAndRepeat)
{
    Playlist playlist;
    playlist.open({"https://a/1.mp4", "https://a/2.mp4", "https://a/3.mp4"});
    EXPECT_FALSE(playlist.hasPrevious());
    EXPECT_TRUE(playlist.hasNext());
    EXPECT_TRUE(playlist.next());
    EXPECT_TRUE(playlist.next());
    EXPECT_EQ(playlist.currentIndex(), 2);
    EXPECT_FALSE(playlist.hasNext());
    EXPECT_FALSE(playlist.next(true));

    playlist.setRepeatMode(Playlist::RepeatMode::All);
    EXPECT_TRUE(playlist.hasNext());
    EXPECT_TRUE(playlist.next(true));
    EXPECT_EQ(playlist.currentIndex(), 0);
    EXPECT_EQ(playlist.previousIndex(), 2);

    // Repeat One replays the file when it ends, but Next still moves on.
    playlist.setRepeatMode(Playlist::RepeatMode::One);
    int current = 0;
    auto subscription = playlist.currentIndexChanged.connect([&] { ++current; });
    EXPECT_TRUE(playlist.next(true));
    EXPECT_EQ(playlist.currentIndex(), 0);
    EXPECT_EQ(current, 1);
    EXPECT_TRUE(playlist.next(false));
    EXPECT_EQ(playlist.currentIndex(), 1);
}

TEST_F(BackendTest, AppendRemoveMove)
{
    Playlist playlist;
    const std::string a = "https://a/a.mp4";
    const std::string b = "https://a/b.mp4";
    const std::string c = "https://a/c.mp4";
    playlist.open({a, b});
    playlist.append({b, c});
    EXPECT_EQ(playlist.urls(), (std::vector<std::string>{a, b, c}));

    playlist.setCurrentIndex(1);
    playlist.move(1, 2);
    EXPECT_EQ(playlist.urls(), (std::vector<std::string>{a, c, b}));
    EXPECT_EQ(playlist.currentIndex(), 2);
    playlist.move(0, 2);
    EXPECT_EQ(playlist.urls(), (std::vector<std::string>{c, b, a}));
    EXPECT_EQ(playlist.currentIndex(), 1);

    playlist.remove(0);
    EXPECT_EQ(playlist.currentIndex(), 0);
    EXPECT_EQ(playlist.currentUrl(), b);
    playlist.remove(0);
    EXPECT_EQ(playlist.currentIndex(), -1);
    EXPECT_EQ(playlist.count(), 1);
    EXPECT_EQ(playlist.nextIndex(), 0);
    playlist.clear();
    EXPECT_EQ(playlist.count(), 0);
}

TEST_F(BackendTest, MediaAndSubtitleSuffixes)
{
    EXPECT_TRUE(Playlist::isMediaFile("/x/Movie.MKV"));
    EXPECT_TRUE(Playlist::isMediaFile("song.opus"));
    EXPECT_FALSE(Playlist::isMediaFile("Movie.srt"));
    EXPECT_TRUE(Playlist::isSubtitleFile("Movie.zh-CN.ass"));
    EXPECT_TRUE(Application::isSubtitle(url("/x/a.vtt")));
    EXPECT_FALSE(Playlist::isSubtitleFile("a.mp4"));

    // .ts: an MPEG transport stream, or a translation source that is not media.
    const std::string translation = touch("zh_CN.ts");
    std::ofstream(translation) << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE TS>\n"
                                  "<TS version=\"2.1\" language=\"zh_CN\">\n</TS>\n";
    EXPECT_FALSE(Playlist::isMediaFile(translation));
    const std::string stream = touch("broadcast.ts");
    std::string packets(188 * 4, '\xff');
    for (size_t i = 0; i < packets.size(); i += 188)
        packets[i] = 0x47;
    std::ofstream(stream, std::ios::binary) << packets;
    EXPECT_TRUE(Playlist::isMediaFile(stream));
    EXPECT_EQ(Playlist::folderMedia(stream).size(), 1u);
}

struct ResumeCase
{
    double position;
    double duration;
    bool kept;
};

class ResumeRulesTest : public BackendTest, public testing::WithParamInterface<ResumeCase>
{
};

TEST_P(ResumeRulesTest, Rules)
{
    const ResumeCase& rule = GetParam();
    EXPECT_EQ(ResumeStore::worthKeeping(rule.position, rule.duration), rule.kept);

    ResumeStore store(settingsFile());
    const std::string film = url("/videos/film.mkv");
    store.save(film, 1200, 3600);
    store.save(film, rule.position, rule.duration);
    EXPECT_EQ(store.position(film) > 0, rule.kept);
}

INSTANTIATE_TEST_SUITE_P(Resume, ResumeRulesTest,
                         testing::Values(ResumeCase{100, 170, false},     // a short file
                                         ResumeCase{59, 3600, false},     // the first minute
                                         ResumeCase{600, 3600, true},     // the middle
                                         ResumeCase{3500, 3600, false},   // the credits
                                         ResumeCase{600, 0, false}));     // an unknown length

TEST_F(BackendTest, ResumeRoundTrip)
{
    const std::string film = url("/videos/\xe4\xb8\xad\xe6\x96\x87 film [1].mkv");
    {
        ResumeStore store(settingsFile());
        store.save(film, 754.25, 5400, {0, 2, 0, -0.3});
    }
    ResumeStore store(settingsFile());
    const auto entry = store.entry(film);
    ASSERT_TRUE(entry);
    EXPECT_EQ(entry->position, 754.25);
    EXPECT_EQ(entry->audioTrack, 2);
    EXPECT_EQ(entry->subtitleTrack, 0);
    EXPECT_EQ(entry->subtitleDelay, -0.3);
    EXPECT_FALSE(store.entry(url("/videos/other.mkv")));

    // Without choices, tracks fall back to the file's defaults.
    store.save(film, 800, 5400);
    EXPECT_EQ(store.entry(film)->audioTrack, -1);

    store.forget(film);
    EXPECT_EQ(store.position(film), 0.0);
}

TEST_F(BackendTest, ResumeCapacity)
{
    ResumeStore store(settingsFile());
    for (int i = 0; i <= ResumeStore::Capacity; ++i)
        store.save("https://example.com/" + std::to_string(i) + ".mp4", 100, 1000);
    const base::Settings settings(settingsFile());
    EXPECT_EQ(settings.childKeys("Resume").size(), size_t(ResumeStore::Capacity));
    EXPECT_GT(store.position("https://example.com/" + std::to_string(ResumeStore::Capacity) + ".mp4"), 0);
}

TEST_F(BackendTest, SettingsPersist)
{
    {
        PlayerSettings settings(settingsFile());
        EXPECT_EQ(settings.volume(), 100);
        EXPECT_TRUE(settings.resumePlayback());
        EXPECT_TRUE(settings.folderPlaylist());
        EXPECT_TRUE(settings.screenshotFolder().ends_with("/Screenshots"));
        int volume = 0;
        auto subscription = settings.volumeChanged.connect([&] { ++volume; });
        settings.setVolume(180);
        EXPECT_EQ(settings.volume(), 150);
        settings.setVolume(150);
        EXPECT_EQ(volume, 1);
        settings.setMuted(true);
        settings.setResumePlayback(false);
        settings.setFolderPlaylist(false);
        settings.setRepeatMode(2);
        settings.setSubtitleScale(125);
        settings.setSubtitleLanguage("ja");
        settings.setScreenshotFolder("/tmp/shots");
    }
    PlayerSettings settings(settingsFile());
    EXPECT_EQ(settings.volume(), 150);
    EXPECT_TRUE(settings.muted());
    EXPECT_FALSE(settings.resumePlayback());
    EXPECT_FALSE(settings.folderPlaylist());
    EXPECT_EQ(settings.repeatMode(), 2);
    EXPECT_EQ(settings.subtitleScale(), 125);
    EXPECT_EQ(settings.subtitleLanguage(), "ja");
    EXPECT_EQ(settings.screenshotFolder(), "/tmp/shots");
}

TEST_F(BackendTest, RecentFiles)
{
    PlayerSettings settings(settingsFile());
    settings.addRecentFile(url(touch("first.mkv")));
    for (int i = 0; i < PlayerSettings::RecentCapacity; ++i)
        settings.addRecentFile(url(touch("f" + std::to_string(i) + ".mkv")));
    EXPECT_EQ(settings.recentFiles().size(), size_t(PlayerSettings::RecentCapacity));
    EXPECT_EQ(settings.recentFiles().front().name, "f" + std::to_string(PlayerSettings::RecentCapacity - 1) + ".mkv");

    // Reopening moves a file to the top, and deleted files are not listed.
    settings.addRecentFile(url(filePath("f3.mkv")));
    EXPECT_EQ(settings.recentFiles().front().name, "f3.mkv");
    fs::remove(filePath("f3.mkv"));
    EXPECT_EQ(settings.recentFiles().size(), size_t(PlayerSettings::RecentCapacity - 1));

    const std::string stream = "https://example.com/a.mp4";
    settings.addRecentFile(stream);
    EXPECT_EQ(settings.recentFiles().front().folder, "example.com");
    settings.removeRecentFile(stream);
    settings.clearRecentFiles();
    EXPECT_TRUE(settings.recentFiles().empty());
    EXPECT_TRUE(PlayerSettings(settingsFile()).recentFiles().empty());
}

TEST_F(BackendTest, LanguagePreference)
{
    PlayerSettings settings(settingsFile());
    const std::vector<std::string> codes = settings.languagePreference("zh");
    ASSERT_GE(codes.size(), 3u);
    EXPECT_EQ(std::vector<std::string>(codes.begin(), codes.begin() + 3), (std::vector<std::string>{"zh", "chi", "zho"}));
    EXPECT_EQ(std::count(codes.begin(), codes.end(), "en"), 1);
    EXPECT_EQ(std::count(codes.begin(), codes.end(), "eng"), 1);

    EXPECT_EQ(languages::displayName("chi"), "\xe7\xae\x80\xe4\xbd\x93\xe4\xb8\xad\xe6\x96\x87");
    EXPECT_EQ(languages::displayName("ger"), "Deutsch");
    EXPECT_EQ(languages::displayName("xyz"), "xyz");
    EXPECT_EQ(languages::codes("ger"), (std::vector<std::string>{"de", "ger", "deu"}));
    EXPECT_EQ(languages::codes("eng"), (std::vector<std::string>{"en", "eng"}));
    EXPECT_TRUE(languages::codes("qqq").empty());
    EXPECT_TRUE(languages::codes("").empty());
}

TEST_F(BackendTest, FormatTime)
{
    EXPECT_EQ(Application::formatTime(0, 0), "0:00");
    EXPECT_EQ(Application::formatTime(65.9, 0), "1:05");
    EXPECT_EQ(Application::formatTime(3725, 0), "1:02:05");
    EXPECT_EQ(Application::formatTime(65, 7200), "0:01:05");
    EXPECT_EQ(Application::formatTime(-3, 0), "0:00");
}

TEST_F(BackendTest, UrlFromInput)
{
    EXPECT_EQ(Application::urlFromInput(" https://example.com/v.mp4 "), "https://example.com/v.mp4");
    EXPECT_EQ(Application::urlFromInput("example.com/v.mp4"), "http://example.com/v.mp4");
    EXPECT_EQ(Application::urlFromInput("rtsp://cam.local/stream"), "rtsp://cam.local/stream");
    EXPECT_EQ(Application::urlFromInput("hello"), "");
    EXPECT_EQ(Application::urlFromInput("two\nlines"), "");
    EXPECT_EQ(Application::urlFromInput("/no/such/file.mkv"), "");
    EXPECT_EQ(Application::urlFromInput("javascript:alert(1)"), "");
    EXPECT_EQ(Application::urlFromInput("example.com:8080/a b.mp4"), "http://example.com:8080/a%20b.mp4");
    const std::string file = touch("typed.mkv");
    EXPECT_EQ(Application::urlFromInput(file), url(file));

    EXPECT_EQ(Application::clipboardUrl("  https://example.com/v.mp4\n"), "https://example.com/v.mp4");
    EXPECT_EQ(Application::clipboardUrl("example.com/v.mp4"), "");
}

TEST_F(BackendTest, UserInputArguments)
{
    const std::string file = touch("arg.mkv");
    EXPECT_EQ(urls::fromUserInput("arg.mkv", m_dir.path(), true), url(file));
    // A name that does not exist yet is still a file of the working directory.
    EXPECT_EQ(urls::fromUserInput("later.mkv", m_dir.path(), true), url(filePath("later.mkv")));
    EXPECT_EQ(urls::fromUserInput("https://example.com/x", m_dir.path(), true), "https://example.com/x");
    EXPECT_EQ(urls::fromUserInput(url(file), m_dir.path(), true), url(file));
    EXPECT_EQ(urls::fromLocalFile("/videos/a b#1.mkv"), "file:///videos/a%20b%231.mkv");
    EXPECT_EQ(urls::toLocalFile("file:///videos/a%20b%231.mkv"), "/videos/a b#1.mkv");
    EXPECT_EQ(urls::displayString("https://user:secret@example.com/a%20b.mp4"), "https://user@example.com/a b.mp4");
}

TEST_F(BackendTest, OpenUrlsSplitsSubtitles)
{
    Application application(settingsFile(), filePath("covers"));
    std::vector<std::pair<std::vector<std::string>, std::vector<std::string>>> requested;
    auto subscription = application.openRequested.connect(
            [&](const std::vector<std::string>& media, const std::vector<std::string>& subtitles) {
                requested.emplace_back(media, subtitles);
            });
    const std::string video = url(touch("show/a.mkv"));
    const std::string subtitle = url(touch("show/a.srt"));
    touch("show/b.mkv");
    application.openUrls({subtitle, video});
    ASSERT_EQ(requested.size(), 1u);
    EXPECT_EQ(requested[0].first, std::vector<std::string>{video});
    EXPECT_EQ(requested[0].second, std::vector<std::string>{subtitle});

    // A dropped folder plays its media files.
    application.openUrls({url(filePath("show"))});
    ASSERT_EQ(requested.size(), 2u);
    EXPECT_EQ(requested[1].first.size(), 2u);
}

TEST_F(BackendTest, ScreenshotPathIsUnique)
{
    Application application(settingsFile(), filePath("covers"));
    application.settings().setScreenshotFolder(filePath("shots"));
    const std::string path = application.screenshotPath("AC/DC live");
    EXPECT_EQ(fs::path(path).parent_path().string(), filePath("shots"));
    EXPECT_TRUE(fs::path(path).filename().string().starts_with("AC-DC live "));
    EXPECT_TRUE(path.ends_with(".png"));
    std::ofstream(path).flush();
    const std::string next = application.screenshotPath("AC/DC live");
    if (fs::path(next).stem().string().starts_with(fs::path(path).stem().string())) {
        EXPECT_TRUE(next.ends_with(" (2).png"));
    }
    EXPECT_NE(next, path);
}

TEST_F(BackendTest, DisplayNames)
{
    EXPECT_EQ(Application::displayName(url("/v/My.Film.2024.mkv")), "My.Film.2024");
    EXPECT_EQ(Application::displayName("https://example.com/a%20b.mp4"), "a b");
    EXPECT_EQ(Application::displayName("https://example.com/"), "https://example.com/");
    EXPECT_EQ(Playlist::titleFor("https://example.com/a%20b.mp4"), "a b.mp4");
}

TEST_F(BackendTest, TrackSplit)
{
    using Map = MpvNode::Map;
    const MpvNode list(MpvNode::List{
        Map{{"id", 1}, {"type", "video"}},
        Map{{"id", 1}, {"type", "audio"}, {"lang", "jpn"}, {"selected", true}},
        Map{{"id", 2}, {"type", "audio"}, {"title", "Commentary"}, {"lang", "en"}},
        Map{{"id", 3}, {"type", "audio"}},
        Map{{"id", 1}, {"type", "sub"}, {"title", "English"}, {"lang", "eng"}, {"external", true}},
    });
    const tracks::Split split = tracks::split(list);
    EXPECT_TRUE(split.hasVideo);
    ASSERT_EQ(split.audio.size(), 3u);
    ASSERT_EQ(split.subtitle.size(), 1u);
    EXPECT_EQ(split.audio[0].label, "\xe6\x97\xa5\xe6\x9c\xac\xe8\xaa\x9e");
    EXPECT_TRUE(split.audio[0].selected);
    EXPECT_EQ(split.audio[1].label, "Commentary (English)");
    EXPECT_EQ(split.audio[2].label, "Track 3");
    // A title that only names the language is not repeated.
    EXPECT_EQ(split.subtitle[0].label, "English");
    EXPECT_TRUE(split.subtitle[0].external);

    // Cover art is not a picture to show.
    const tracks::Split audioOnly = tracks::split(MpvNode(MpvNode::List{Map{{"type", "video"}, {"albumart", true}}}));
    EXPECT_FALSE(audioOnly.hasVideo);
}

TEST_F(BackendTest, MpvNodeRoundTrip)
{
    const MpvNode value(MpvNode::Map{
        {"flag", true},
        {"int", 42},
        {"double", 1.5},
        {"text", "\xc3\xa9\xe6\xbc\xa2"},
        {"list", MpvNode::List{1, "two", MpvNode::List{}}},
    });
    mpv_node node;
    value.fill(&node);
    EXPECT_EQ(node.format, MPV_FORMAT_NODE_MAP);
    const MpvNode back = MpvNode::from(&node);
    MpvNode::release(&node);
    EXPECT_EQ(node.format, MPV_FORMAT_NONE);
    EXPECT_EQ(back, value);
    EXPECT_TRUE(back["flag"].toBool());
    EXPECT_EQ(back["int"].toInt(), 42);
    EXPECT_EQ(back["double"].toDouble(), 1.5);
    EXPECT_EQ(back["text"].toString(), "\xc3\xa9\xe6\xbc\xa2");
    ASSERT_EQ(back["list"].list().size(), 3u);
    EXPECT_EQ(back["list"].list()[1].toString(), "two");
}
