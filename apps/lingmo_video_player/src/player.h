// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "mpvnode.h"
#include "tracks.h"

#include "base/signal.h"
#include "base/weak_ptr.h"

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct MpvHandle;

// The player: libmpv driven through its asynchronous client API from the
// loop's thread, so no call here ever blocks. Pictures are drawn by a video
// output (VideoOutput) holding mpv's render context; files load only once
// one is ready, or mpv would find no video output and play without a picture.
class Player
{
public:
    enum class Property {
        Source,
        Loaded,
        Paused,
        Position,
        Duration,
        Seekable,
        Buffering,
        EndReached,
        Volume,
        Muted,
        Speed,
        MediaTitle,
        VideoSize,
        Tracks,
        AudioTrack,
        SubtitleTrack,
        SubtitleDelay,
        Chapters,
        Chapter,
        Rotation,
        LoopFile,
        HardwareDecoder,
        VideoActive,
    };

    struct Size
    {
        int width = 0;
        int height = 0;

        bool isEmpty() const { return width <= 0 || height <= 0; }
        bool operator==(const Size&) const = default;
    };

    struct Chapter
    {
        std::string title;
        double time = 0;

        bool operator==(const Chapter&) const = default;
    };

    // mpv file options such as start, aid, sid and sub-delay.
    using FileOptions = std::vector<std::pair<std::string, MpvNode>>;

    Player();
    ~Player();

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    // The picture size the player would report for a local file, read from
    // its container without decoding; empty if mpv cannot tell in time.
    static Size probeVideoSize(const std::string& url, std::chrono::milliseconds timeout);

    // For a video output: the handle its render context is made on, once
    // the player has started (renderWanted then fires). Null before.
    std::shared_ptr<MpvHandle> handle() const;
    Lingmo::base::WeakPtr<Player> weakPtr() { return m_weakFactory.getWeakPtr(); }
    // A video output can draw: files load only after that.
    void setRenderReady();
    // The render context went away; video comes back with the next one.
    void setRenderLost();

    const std::string& source() const { return m_source; }
    bool loaded() const { return m_loaded; }
    bool paused() const { return m_paused; }
    void setPaused(bool paused);
    double position() const { return m_position; }
    double duration() const { return m_duration; }
    bool seekable() const { return m_seekable; }
    bool buffering() const { return m_pausedForCache || m_seeking; }
    bool endReached() const { return m_endReached; }
    int volume() const { return m_volume; }
    void setVolume(int volume);
    bool muted() const { return m_muted; }
    void setMuted(bool muted);
    double speed() const { return m_speed; }
    void setSpeed(double speed);
    const std::string& mediaTitle() const { return m_mediaTitle; }
    Size videoSize() const { return m_videoSize; }
    const std::vector<tracks::Track>& audioTracks() const { return m_audioTracks; }
    const std::vector<tracks::Track>& subtitleTracks() const { return m_subtitleTracks; }
    bool hasVideo() const { return m_hasVideo; }
    // The track labels again, in the language of the moment.
    void relabelTracks();
    int audioTrack() const { return m_audioTrack; }
    void setAudioTrack(int id);
    int subtitleTrack() const { return m_subtitleTrack; }
    void setSubtitleTrack(int id);
    double subtitleDelay() const { return m_subtitleDelay; }
    void setSubtitleDelay(double delay);
    const std::vector<Chapter>& chapters() const { return m_chapters; }
    int chapter() const { return m_chapter; }
    void setChapter(int chapter);
    int rotation() const { return m_rotation; }
    void setRotation(int degrees);
    bool loopFile() const { return m_loopFile; }
    void setLoopFile(bool loop);
    const std::string& hardwareDecoder() const { return m_hardwareDecoder; }
    // mpv's video output is up and drawing.
    bool videoActive() const { return m_videoActive; }

    // Opens url (a URL) with file options; paused leaves it on its first frame.
    void load(const std::string& url, const FileOptions& options = {}, bool paused = false);
    void stop();
    void seek(double seconds, bool exact = false);
    void seekRelative(double seconds);
    void frameStep(bool backwards);
    void addSubtitle(const std::string& url);
    // Saves the current frame, with subtitles, as a PNG at path.
    void screenshot(const std::string& path);
    // Codecs, size, frame rate and bit rates of the loaded file, by mpv's
    // property names; blocks for mpv's answer.
    MpvNode::Map mediaInfo() const;
    // Languages to pick audio and subtitle tracks by, most wanted first.
    void setPreferredLanguages(const std::vector<std::string>& audio, const std::vector<std::string>& subtitle);
    void setSubtitleScale(double scale);
    // Keeps subtitles above the bottom inset logical pixels of a picture
    // viewHeight tall, such as controls drawn over the video.
    void setSubtitleBottomInset(double inset);
    void setViewHeight(double height);
    // While the user drags the window's edge: scaling that is cheap to draw
    // at a new size every frame, mpv's own afterwards.
    void setResizing(bool resizing);

    Lingmo::base::Signal<Property> changed;
    Lingmo::base::Signal<> fileLoaded;
    Lingmo::base::Signal<const std::string&> playbackFailed;
    Lingmo::base::Signal<const std::string&, bool> screenshotSaved;
    Lingmo::base::Signal<bool> subtitleAdded;
    // mpv has started and wants a video output.
    Lingmo::base::Signal<> renderWanted;

private:
    // Reply ids: screenshots count up from FirstScreenshot.
    enum Request : uint64_t {
        GenericRequest,
        SubtitleRequest,
        FirstScreenshot = 1000,
    };

    bool ensurePlayer();
    void open(const std::string& url, const FileOptions& options, bool paused);
    // Opens the current file again after the lost output, quietly.
    void reopen(const FileOptions& options);
    void forgetLostFile();
    void startLoad();
    void processEvents();
    void handlePropertyChange(const struct mpv_event_property* property);
    void command(const MpvNode::List& arguments, uint64_t request = GenericRequest);
    void setOption(const char* name, const MpvNode& value);
    void setMpvProperty(const char* name, const MpvNode& value);
    void resetMedia();
    void applySubtitleMargin();
    template<typename T>
    void update(T& field, const T& value, Property property);

    std::shared_ptr<MpvHandle> m_handle;
    std::map<uint64_t, std::string> m_pendingScreenshots;
    uint64_t m_nextScreenshot = FirstScreenshot;
    bool m_renderReady = false;
    // The render context was lost while a file played; video comes back with the next one.
    bool m_videoLost = false;
    // Without a picture mpv may end the file; then it is reopened where it was.
    FileOptions m_lostState;
    bool m_lostPaused = false;
    bool m_reopening = false;
    // The picture came back for a file still open; its end may yet arrive.
    bool m_recovering = false;
    bool m_playerFailed = false;
    // mpv's playlist entry started last, and the one the last load replaced.
    int64_t m_entry = 0;
    int64_t m_replacedEntry = 0;
    std::vector<std::pair<std::string, MpvNode>> m_pendingProperties;
    MpvNode::List m_pendingLoad;

    std::string m_source;
    bool m_loaded = false;
    bool m_paused = false;
    double m_position = 0;
    double m_duration = 0;
    bool m_seekable = false;
    bool m_pausedForCache = false;
    bool m_seeking = false;
    bool m_endReached = false;
    int m_volume = 100;
    bool m_muted = false;
    double m_speed = 1.0;
    std::string m_mediaTitle;
    Size m_videoSize;
    MpvNode m_trackList;
    std::vector<tracks::Track> m_audioTracks;
    std::vector<tracks::Track> m_subtitleTracks;
    bool m_hasVideo = false;
    int m_audioTrack = 0;
    int m_subtitleTrack = 0;
    double m_subtitleDelay = 0;
    double m_subtitleInset = 0;
    double m_viewHeight = 0;
    bool m_resizing = false;
    // mpv's scaling options as it started, for after a resize.
    std::vector<std::pair<std::string, MpvNode>> m_scaling;
    std::vector<Chapter> m_chapters;
    int m_chapter = -1;
    int m_rotation = 0;
    bool m_loopFile = false;
    std::string m_hardwareDecoder;
    bool m_videoActive = false;

    Lingmo::base::WeakPtrFactory<Player> m_weakFactory{this};
};
