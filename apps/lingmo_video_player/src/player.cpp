// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "player.h"
#include "mpvhandle.h"
#include "urls.h"

#include "base/i18n.h"
#include "base/logging.h"
#include "base/strings.h"

#include <algorithm>
#include <clocale>
#include <cmath>

using namespace Lingmo;
using Lingmo::i18n::tr;

static const base::LogCategory cfMpv("lingmo.videoplayer.mpv", base::LogLevel::Warning);

// Runs on an mpv thread when events are waiting.
static void wakeup(void* context)
{
    static_cast<MpvHandle*>(context)->notifyEvents();
}

static int trackId(const MpvNode& value)
{
    return value.isInt() ? int(value.toInt()) : 0;
}

// The scaling of mpv's "fast" profile: drawing at a new size makes no
// intermediate textures of that size and weighs no filter kernels for it.
static const std::vector<std::pair<const char*, MpvNode>>& resizeScaling()
{
    static const std::vector<std::pair<const char*, MpvNode>> options = {
        {"scale", "bilinear"},
        {"dscale", "bilinear"},
        {"correct-downscaling", false},
        {"linear-downscaling", false},
        {"sigmoid-upscaling", false},
    };
    return options;
}

static std::string optionText(const MpvNode& value)
{
    if (value.isDouble())
        return base::strings::number(value.toDouble(), 'f', 3);
    if (value.isInt())
        return std::to_string(value.toInt());
    if (value.isFlag())
        return value.toBool() ? "true" : "false";
    return value.toString();
}

Player::Player()
    : m_handle(std::make_shared<MpvHandle>())
{
    // mpv parses numbers with the C locale; the application set the user's.
    std::setlocale(LC_NUMERIC, "C");
}

Player::~Player()
{
    // First, so no callback reaches this player from here on.
    m_handle->detach();
    if (m_handle->mpv) {
        mpv_set_wakeup_callback(m_handle->mpv, nullptr, nullptr);
        // Silences the player now; the handle itself may live on in a video output.
        command({"stop"});
    }
}

std::shared_ptr<MpvHandle> Player::handle() const
{
    return m_handle;
}

// mpv starts with the first file, so an empty window costs no player.
bool Player::ensurePlayer()
{
    if (m_handle->mpv)
        return true;
    if (m_playerFailed)
        return false;
    m_playerFailed = true;
    mpv_handle* mpv = mpv_create();
    if (!mpv) {
        LM_WARNING(cfMpv) << "mpv: could not create a player";
        return false;
    }
    m_handle->mpv = mpv;

    setOption("vo", "libmpv");
    setOption("hwdec", "auto-safe");
    // Chroma upscaled with the default lanczos is about half the GPU time of
    // a 4K picture, enough to drop frames until the GPU clocks up.
    setOption("cscale", "bilinear");
    // Playback stops on the last frame; the playlist decides what comes next.
    setOption("keep-open", "yes");
    // Frames reach the video output at their display time instead of 50 ms
    // early, so drawing one never waits for that time and a window resize
    // gets its picture at once.
    setOption("video-timing-offset", 0);
    setOption("idle", "yes");
    setOption("input-default-bindings", false);
    setOption("input-vo-keyboard", false);
    setOption("osd-level", 0);
    setOption("sub-auto", "fuzzy");
    setOption("sub-codepage", "auto");
    setOption("sub-file-paths", "subs:Subs:Subtitles:subtitles");
    setOption("screenshot-format", "png");
    setOption("audio-client-name", "lingmo-video-player");
    setOption("volume-max", 150);

    // What was set before mpv started.
    for (const auto& [name, value] : m_pendingProperties)
        setOption(name.c_str(), value);
    m_pendingProperties.clear();

    if (mpv_initialize(mpv) < 0) {
        LM_WARNING(cfMpv) << "mpv: could not initialise the player";
        mpv_destroy(mpv);
        m_handle->mpv = nullptr;
        return false;
    }

    static const struct {
        const char* name;
        mpv_format format;
    } observed[] = {
        {"time-pos", MPV_FORMAT_DOUBLE},
        {"duration", MPV_FORMAT_DOUBLE},
        {"pause", MPV_FORMAT_FLAG},
        {"seekable", MPV_FORMAT_FLAG},
        {"paused-for-cache", MPV_FORMAT_FLAG},
        {"seeking", MPV_FORMAT_FLAG},
        {"eof-reached", MPV_FORMAT_FLAG},
        {"volume", MPV_FORMAT_DOUBLE},
        {"mute", MPV_FORMAT_FLAG},
        {"speed", MPV_FORMAT_DOUBLE},
        {"media-title", MPV_FORMAT_STRING},
        {"dwidth", MPV_FORMAT_INT64},
        {"dheight", MPV_FORMAT_INT64},
        {"track-list", MPV_FORMAT_NODE},
        {"aid", MPV_FORMAT_NODE},
        {"sid", MPV_FORMAT_NODE},
        {"sub-delay", MPV_FORMAT_DOUBLE},
        {"chapter-list", MPV_FORMAT_NODE},
        {"chapter", MPV_FORMAT_INT64},
        {"video-rotate", MPV_FORMAT_INT64},
        {"loop-file", MPV_FORMAT_STRING},
        {"hwdec-current", MPV_FORMAT_STRING},
        {"vo-configured", MPV_FORMAT_FLAG},
    };
    for (const auto& property : observed)
        mpv_observe_property(mpv, 0, property.name, property.format);
    for (const auto& [name, value] : resizeScaling()) {
        mpv_node node;
        if (mpv_get_property(mpv, name, MPV_FORMAT_NODE, &node) < 0)
            continue;
        m_scaling.emplace_back(name, MpvNode::from(&node));
        mpv_free_node_contents(&node);
    }

    mpv_request_log_messages(mpv, "error");
    base::EventLoop* loop = base::EventLoop::current();
    m_handle->attach(loop ? loop->handle() : base::LoopHandle(), [weak = m_weakFactory.getWeakPtr()] {
        if (weak)
            weak->processEvents();
    });
    mpv_set_wakeup_callback(mpv, wakeup, m_handle.get());
    m_playerFailed = false;
    // A video output now makes its render context, which the first load waits for.
    renderWanted.emit();
    return true;
}

Player::Size Player::probeVideoSize(const std::string& url, std::chrono::milliseconds timeout)
{
    if (!urls::isLocalFile(url))
        return {};
    std::setlocale(LC_NUMERIC, "C");
    mpv_handle* mpv = mpv_create();
    if (!mpv)
        return {};
    mpv_set_option_string(mpv, "vo", "null");
    mpv_set_option_string(mpv, "ao", "null");
    mpv_set_option_string(mpv, "aid", "no");
    mpv_set_option_string(mpv, "sid", "no");
    mpv_set_option_string(mpv, "pause", "yes");
    mpv_set_option_string(mpv, "load-scripts", "no");
    mpv_set_option_string(mpv, "ytdl", "no");

    Size size;
    const std::string path = urls::toLocalFile(url);
    const char* load[] = {"loadfile", path.c_str(), nullptr};
    if (mpv_initialize(mpv) >= 0 && mpv_command(mpv, load) >= 0) {
        using Clock = std::chrono::steady_clock;
        const Clock::time_point deadline = Clock::now() + timeout;
        while (Clock::now() < deadline) {
            const double left = std::chrono::duration<double>(deadline - Clock::now()).count();
            const mpv_event_id event = mpv_wait_event(mpv, left)->event_id;
            if (event == MPV_EVENT_END_FILE || event == MPV_EVENT_SHUTDOWN)
                break;
            if (event != MPV_EVENT_FILE_LOADED)
                continue;
            mpv_node node;
            if (mpv_get_property(mpv, "track-list", MPV_FORMAT_NODE, &node) < 0)
                break;
            const MpvNode tracks = MpvNode::from(&node);
            mpv_free_node_contents(&node);
            for (const MpvNode& track : tracks.list()) {
                if (track["type"].toString() != "video" || !track["selected"].toBool() || track["albumart"].toBool())
                    continue;
                // The display size as mpv derives it: the pixel aspect widens
                // the picture or makes it taller, never narrower.
                int width = int(track["demux-w"].toInt());
                int height = int(track["demux-h"].toInt());
                const double aspect = track["demux-par"].toDouble(1.0);
                if (aspect > 1)
                    width = int(width * aspect + 1e-6);
                else if (aspect > 0 && aspect < 1)
                    height = int(height / aspect + 1e-6);
                if (width > 0 && height > 0)
                    size = {width, height};
                break;
            }
            break;
        }
    }
    mpv_terminate_destroy(mpv);
    return size;
}

void Player::setRenderReady()
{
    if (m_renderReady)
        return;
    m_renderReady = true;
    if (m_videoLost) {
        // mpv dropped the video output with the old context: a file still open
        // gets its picture back, one that ended for want of it is reopened.
        m_videoLost = false;
        if (m_loaded) {
            // The file may still end for want of the old output; then it is reopened.
            m_recovering = true;
            setMpvProperty("vid", "no");
            setMpvProperty("vid", "auto");
        } else if (!m_source.empty() && m_pendingLoad.empty() && !m_lostState.empty()) {
            const FileOptions options = std::move(m_lostState);
            m_lostState.clear();
            reopen(options);
            return;
        }
    }
    startLoad();
}

void Player::setRenderLost()
{
    if (!m_renderReady)
        return;
    m_renderReady = false;
    m_videoLost = true;
    m_lostState.clear();
    if (m_loaded) {
        m_lostState = {
            {"aid", m_audioTrack > 0 ? MpvNode(m_audioTrack) : MpvNode("no")},
            {"sid", m_subtitleTrack > 0 ? MpvNode(m_subtitleTrack) : MpvNode("no")},
            {"speed", m_speed},
            {"start", m_position},
            {"sub-delay", m_subtitleDelay},
        };
        m_lostPaused = m_paused;
    }
}

void Player::processEvents()
{
    // Cleared first: a wakeup from here on queues another round.
    m_handle->eventsQueued = false;
    auto weak = m_weakFactory.getWeakPtr();
    while (weak && m_handle->mpv) {
        mpv_event* event = mpv_wait_event(m_handle->mpv, 0);
        if (event->event_id == MPV_EVENT_NONE)
            break;

        switch (event->event_id) {
        case MPV_EVENT_PROPERTY_CHANGE:
            handlePropertyChange(static_cast<mpv_event_property*>(event->data));
            break;
        case MPV_EVENT_START_FILE:
            m_entry = static_cast<mpv_event_start_file*>(event->data)->playlist_entry_id;
            break;
        case MPV_EVENT_FILE_LOADED:
            m_loaded = true;
            changed.emit(Property::Loaded);
            // A file reopened after a lost picture is not news to the window.
            if (!weak)
                return;
            if (!m_reopening)
                fileLoaded.emit();
            m_reopening = false;
            break;
        case MPV_EVENT_END_FILE: {
            const auto* end = static_cast<mpv_event_end_file*>(event->data);
            const bool failed = end->reason == MPV_END_FILE_REASON_ERROR;
            const std::string message = failed ? mpv_error_string(end->error) : std::string();
            // The file a load replaced may still end the way the lost output left it.
            const bool replaced = end->playlist_entry_id == m_replacedEntry;
            if (failed && !replaced && m_recovering && !m_lostState.empty() && m_renderReady) {
                // The end the lost output caused arrived after the new one came.
                m_recovering = false;
                const FileOptions options = m_lostState;
                reopen(options);
                break;
            }
            if (m_loaded) {
                m_loaded = false;
                changed.emit(Property::Loaded);
            }
            // Ending because the picture went away is recovered from, not reported.
            if (weak && failed && !m_videoLost && !replaced)
                playbackFailed.emit(message);
            break;
        }
        case MPV_EVENT_LOG_MESSAGE: {
            const auto* message = static_cast<mpv_event_log_message*>(event->data);
            LM_WARNING(cfMpv).printf("%s: %s", message->prefix,
                                     std::string(base::strings::trimmed(message->text)).c_str());
            break;
        }
        case MPV_EVENT_COMMAND_REPLY:
            if (event->reply_userdata >= FirstScreenshot) {
                const auto pending = m_pendingScreenshots.find(event->reply_userdata);
                std::string path;
                if (pending != m_pendingScreenshots.end()) {
                    path = std::move(pending->second);
                    m_pendingScreenshots.erase(pending);
                }
                screenshotSaved.emit(path, event->error >= 0);
            } else if (event->reply_userdata == SubtitleRequest) {
                subtitleAdded.emit(event->error >= 0);
            }
            break;
        default:
            break;
        }
    }
}

template<typename T>
void Player::update(T& field, const T& value, Property property)
{
    if (field == value)
        return;
    field = value;
    changed.emit(property);
}

void Player::handlePropertyChange(const mpv_event_property* property)
{
    const std::string_view name(property->name);
    const bool unset = property->format == MPV_FORMAT_NONE;
    auto asDouble = [&] { return unset ? 0.0 : *static_cast<double*>(property->data); };
    auto asFlag = [&] { return !unset && *static_cast<int*>(property->data) != 0; };
    auto asInt = [&] { return unset ? 0 : int(*static_cast<int64_t*>(property->data)); };
    auto asString = [&] { return unset ? std::string() : std::string(*static_cast<char**>(property->data)); };
    auto asNode = [&] { return unset ? MpvNode() : MpvNode::from(static_cast<mpv_node*>(property->data)); };

    if (name == "time-pos") {
        update(m_position, asDouble(), Property::Position);
    } else if (name == "duration") {
        update(m_duration, asDouble(), Property::Duration);
    } else if (name == "pause") {
        update(m_paused, asFlag(), Property::Paused);
    } else if (name == "seekable") {
        update(m_seekable, asFlag(), Property::Seekable);
    } else if (name == "paused-for-cache" || name == "seeking") {
        const bool before = buffering();
        (name == "seeking" ? m_seeking : m_pausedForCache) = asFlag();
        if (before != buffering())
            changed.emit(Property::Buffering);
    } else if (name == "eof-reached") {
        update(m_endReached, asFlag(), Property::EndReached);
    } else if (name == "volume") {
        if (!unset)
            update(m_volume, int(std::lround(asDouble())), Property::Volume);
    } else if (name == "mute") {
        update(m_muted, asFlag(), Property::Muted);
    } else if (name == "speed") {
        if (!unset)
            update(m_speed, asDouble(), Property::Speed);
    } else if (name == "media-title") {
        update(m_mediaTitle, asString(), Property::MediaTitle);
    } else if (name == "dwidth") {
        update(m_videoSize, Size{asInt(), m_videoSize.height}, Property::VideoSize);
    } else if (name == "dheight") {
        update(m_videoSize, Size{m_videoSize.width, asInt()}, Property::VideoSize);
    } else if (name == "track-list") {
        m_trackList = asNode();
        tracks::Split split = tracks::split(m_trackList);
        m_audioTracks = std::move(split.audio);
        m_subtitleTracks = std::move(split.subtitle);
        m_hasVideo = split.hasVideo;
        changed.emit(Property::Tracks);
    } else if (name == "aid") {
        update(m_audioTrack, trackId(asNode()), Property::AudioTrack);
    } else if (name == "sid") {
        update(m_subtitleTrack, trackId(asNode()), Property::SubtitleTrack);
    } else if (name == "sub-delay") {
        update(m_subtitleDelay, asDouble(), Property::SubtitleDelay);
    } else if (name == "chapter-list") {
        std::vector<Chapter> chapters;
        const MpvNode list = asNode();
        for (const MpvNode& entry : list.list())
            chapters.push_back({entry["title"].toString(), entry["time"].toDouble()});
        update(m_chapters, chapters, Property::Chapters);
    } else if (name == "chapter") {
        update(m_chapter, unset ? -1 : asInt(), Property::Chapter);
    } else if (name == "video-rotate") {
        update(m_rotation, asInt(), Property::Rotation);
    } else if (name == "loop-file") {
        const std::string loop = asString();
        update(m_loopFile, !loop.empty() && loop != "no", Property::LoopFile);
    } else if (name == "vo-configured") {
        if (asFlag())
            m_recovering = false;
        update(m_videoActive, asFlag(), Property::VideoActive);
    } else if (name == "hwdec-current") {
        const std::string decoder = asString();
        update(m_hardwareDecoder, decoder == "no" ? std::string() : decoder, Property::HardwareDecoder);
    }
}

void Player::command(const MpvNode::List& arguments, uint64_t request)
{
    if (!m_handle->mpv)
        return;
    mpv_node node;
    MpvNode(arguments).fill(&node);
    mpv_command_node_async(m_handle->mpv, request, &node);
    MpvNode::release(&node);
}

void Player::setOption(const char* name, const MpvNode& value)
{
    mpv_node node;
    value.fill(&node);
    mpv_set_option(m_handle->mpv, name, MPV_FORMAT_NODE, &node);
    MpvNode::release(&node);
}

void Player::setMpvProperty(const char* name, const MpvNode& value)
{
    if (!m_handle->mpv) {
        // Only the last value counts: an empty window sets the subtitle margin on every resize.
        const auto pending = std::find_if(m_pendingProperties.begin(), m_pendingProperties.end(),
                                          [name](const auto& property) { return property.first == name; });
        if (pending != m_pendingProperties.end())
            pending->second = value;
        else
            m_pendingProperties.emplace_back(name, value);
        return;
    }
    mpv_node node;
    value.fill(&node);
    mpv_set_property_async(m_handle->mpv, GenericRequest, name, MPV_FORMAT_NODE, &node);
    MpvNode::release(&node);
}

void Player::setPaused(bool paused)
{
    // Playing again from the last frame starts the file over.
    if (!paused && m_endReached && !m_loopFile)
        seek(0, true);
    setMpvProperty("pause", paused);
}

void Player::setVolume(int volume)
{
    setMpvProperty("volume", double(std::clamp(volume, 0, 150)));
}

void Player::setMuted(bool muted)
{
    setMpvProperty("mute", muted);
}

void Player::setSpeed(double speed)
{
    setMpvProperty("speed", std::clamp(speed, 0.25, 4.0));
}

void Player::setAudioTrack(int id)
{
    setMpvProperty("aid", id > 0 ? MpvNode(id) : MpvNode("no"));
}

void Player::setSubtitleTrack(int id)
{
    setMpvProperty("sid", id > 0 ? MpvNode(id) : MpvNode("no"));
}

void Player::setSubtitleDelay(double delay)
{
    setMpvProperty("sub-delay", delay);
}

void Player::setChapter(int chapter)
{
    if (m_chapters.empty())
        return;
    setMpvProperty("chapter", std::clamp(chapter, 0, int(m_chapters.size()) - 1));
}

void Player::setRotation(int degrees)
{
    setMpvProperty("video-rotate", ((degrees % 360) + 360) % 360);
}

void Player::setLoopFile(bool loop)
{
    setMpvProperty("loop-file", loop ? "inf" : "no");
}

void Player::relabelTracks()
{
    tracks::Split split = tracks::split(m_trackList);
    if (split.audio == m_audioTracks && split.subtitle == m_subtitleTracks)
        return;
    m_audioTracks = std::move(split.audio);
    m_subtitleTracks = std::move(split.subtitle);
    changed.emit(Property::Tracks);
}

void Player::load(const std::string& url, const FileOptions& options, bool paused)
{
    forgetLostFile();
    open(url, options, paused);
}

void Player::forgetLostFile()
{
    m_recovering = false;
    m_reopening = false;
    m_lostState.clear();
}

void Player::reopen(const FileOptions& options)
{
    m_reopening = true;
    open(m_source, options, m_lostPaused);
}

void Player::open(const std::string& url, const FileOptions& options, bool paused)
{
    if (!ensurePlayer()) {
        playbackFailed.emit(tr("mpv could not be started"));
        return;
    }
    if (m_source != url) {
        m_source = url;
        changed.emit(Property::Source);
    }
    m_replacedEntry = m_entry;
    resetMedia();
    setMpvProperty("pause", paused);
    std::vector<std::string> fileOptions;
    for (const auto& [name, value] : options)
        fileOptions.push_back(name + '=' + optionText(value));
    const std::string target = urls::isLocalFile(url) ? urls::toLocalFile(url) : url;
    m_pendingLoad = {"loadfile", target, "replace", -1, base::strings::join(fileOptions, ",")};
    startLoad();
}

void Player::startLoad()
{
    if (!m_renderReady || m_pendingLoad.empty())
        return;
    command(m_pendingLoad);
    m_pendingLoad.clear();
}

void Player::stop()
{
    forgetLostFile();
    m_replacedEntry = m_entry;
    m_pendingLoad.clear();
    command({"stop"});
    if (!m_source.empty()) {
        m_source.clear();
        changed.emit(Property::Source);
    }
    resetMedia();
}

void Player::resetMedia()
{
    // Per-file settings start over with each file.
    setMpvProperty("speed", 1.0);
    setMpvProperty("sub-delay", 0.0);
    setMpvProperty("video-rotate", 0);
}

void Player::seek(double seconds, bool exact)
{
    command({"seek", std::max(0.0, seconds), exact ? "absolute+exact" : "absolute+keyframes"});
}

void Player::seekRelative(double seconds)
{
    command({"seek", seconds, "relative+exact"});
}

void Player::frameStep(bool backwards)
{
    command({backwards ? "frame-back-step" : "frame-step"});
}

void Player::addSubtitle(const std::string& url)
{
    command({"sub-add", urls::isLocalFile(url) ? urls::toLocalFile(url) : url, "select"}, SubtitleRequest);
}

void Player::screenshot(const std::string& path)
{
    const uint64_t request = m_nextScreenshot++;
    m_pendingScreenshots[request] = path;
    command({"screenshot-to-file", path, "subtitles"}, request);
}

MpvNode::Map Player::mediaInfo() const
{
    MpvNode::Map info;
    if (!m_handle->mpv || !m_loaded)
        return info;
    static const char* const names[] = {
        "path", "file-format", "file-size", "duration", "width", "height", "container-fps",
        "video-codec", "video-bitrate", "audio-codec-name", "audio-bitrate",
        "audio-params/samplerate", "audio-params/channel-count",
    };
    for (const char* name : names) {
        mpv_node node;
        if (mpv_get_property(m_handle->mpv, name, MPV_FORMAT_NODE, &node) < 0)
            continue;
        info.emplace_back(name, MpvNode::from(&node));
        mpv_free_node_contents(&node);
    }
    return info;
}

void Player::setPreferredLanguages(const std::vector<std::string>& audio, const std::vector<std::string>& subtitle)
{
    setMpvProperty("alang", base::strings::join(audio, ","));
    setMpvProperty("slang", base::strings::join(subtitle, ","));
}

void Player::setSubtitleScale(double scale)
{
    setMpvProperty("sub-scale", std::clamp(scale, 0.5, 2.0));
}

void Player::setSubtitleBottomInset(double inset)
{
    m_subtitleInset = inset;
    applySubtitleMargin();
}

void Player::setViewHeight(double height)
{
    if (m_viewHeight == height)
        return;
    // The margin is relative to the height, so a resize moves it.
    m_viewHeight = height;
    applySubtitleMargin();
}

void Player::setResizing(bool resizing)
{
    if (m_resizing == resizing)
        return;
    m_resizing = resizing;
    if (!m_handle->mpv)
        return;
    if (resizing) {
        for (const auto& [name, value] : resizeScaling())
            setMpvProperty(name, value);
    } else {
        for (const auto& [name, value] : m_scaling)
            setMpvProperty(name.c_str(), value);
    }
}

void Player::applySubtitleMargin()
{
    // mpv's margin is in units of 1/720 of the window height; 22 is its default.
    const double margin = m_viewHeight > 0 ? m_subtitleInset / m_viewHeight * 720 : 0;
    setMpvProperty("sub-margin-y", int(std::lround(std::max(22.0, margin))));
}
