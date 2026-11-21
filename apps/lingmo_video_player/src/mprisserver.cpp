// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mprisserver.h"

#include "base/logging.h"

#include <algorithm>

using namespace Lingmo;
using base::dbus::MethodCall;
using base::dbus::ObjectPath;
using base::dbus::Variant;
using base::dbus::VariantMap;

static const base::LogCategory cfMpris("lingmo.videoplayer.mpris", base::LogLevel::Warning);

static const char* const kPath = "/org/mpris/MediaPlayer2";
static const char* const kRootInterface = "org.mpris.MediaPlayer2";
static const char* const kPlayerInterface = "org.mpris.MediaPlayer2.Player";
static const char* const kNotSupported = "org.freedesktop.DBus.Error.NotSupported";
static const char* const kInvalidArgs = "org.freedesktop.DBus.Error.InvalidArgs";

MprisServer::MprisServer(base::dbus::Connection* bus, const std::string& serviceName, Description description)
    : m_bus(bus)
    , m_name(std::string(kRootInterface) + '.' + serviceName)
    , m_description(std::move(description))
{
    if (!m_bus) {
        LM_WARNING(cfMpris) << "No session bus for media controls";
        return;
    }

    m_root = std::make_unique<base::dbus::Object>(m_bus, kPath, kRootInterface);
    m_root->addProperty("CanQuit", [this] { return m_description.canQuit; });
    m_root->addProperty("CanRaise", [this] { return m_description.canRaise; });
    m_root->addProperty("CanSetFullscreen", [this] { return m_description.canSetFullscreen; });
    m_root->addProperty("DesktopEntry", [this] { return m_description.desktopEntry; });
    m_root->addProperty("Fullscreen", [this] { return m_fullscreen; },
                        [this](bool fullscreen) {
                            if (m_description.canSetFullscreen)
                                fullscreenRequested.emit(fullscreen);
                            return true;
                        });
    m_root->addProperty("HasTrackList", [this] { return m_description.hasTrackList; });
    m_root->addProperty("Identity", [this] { return m_description.identity; });
    m_root->addProperty("SupportedMimeTypes", [this] { return m_description.supportedMimeTypes; });
    m_root->addProperty("SupportedUriSchemes", [this] { return m_description.supportedUriSchemes; });
    m_root->addMethod("Quit", "", "", [this](MethodCall& call) {
        if (!m_description.canQuit) {
            call.replyError(kNotSupported, "Quit requested but not supported.");
            return;
        }
        call.reply();
        quitRequested.emit();
    });
    m_root->addMethod("Raise", "", "", [this](MethodCall& call) {
        if (!m_description.canRaise) {
            call.replyError(kNotSupported, "Raise requested but not supported.");
            return;
        }
        call.reply();
        raiseRequested.emit();
    });

    m_player = std::make_unique<base::dbus::Object>(m_bus, kPath, kPlayerInterface);
    m_player->addProperty("CanControl", [this] { return m_canControl; });
    m_player->addProperty("CanGoNext", [this] { return m_canControl && m_canGoNext; });
    m_player->addProperty("CanGoPrevious", [this] { return m_canControl && m_canGoPrevious; });
    m_player->addProperty("CanPause", [this] { return m_canControl && m_canPause; });
    m_player->addProperty("CanPlay", [this] { return m_canControl && m_canPlay; });
    m_player->addProperty("CanSeek", [this] { return m_canControl && m_canSeek; });
    m_player->addProperty("LoopStatus", [this] { return toString(m_loopStatus); },
                          [this](const std::string& value) {
                              if (!m_canControl)
                                  return true;
                              if (value == "None")
                                  loopStatusRequested.emit(LoopStatus::None);
                              else if (value == "Track")
                                  loopStatusRequested.emit(LoopStatus::Track);
                              else if (value == "Playlist")
                                  loopStatusRequested.emit(LoopStatus::Playlist);
                              return true;
                          });
    m_player->addProperty("MaximumRate", [this] { return m_maximumRate; });
    m_player->addProperty("Metadata", [this] { return typedMetadata(); });
    m_player->addProperty("MinimumRate", [this] { return m_minimumRate; });
    m_player->addProperty("PlaybackStatus", [this] { return toString(m_playbackStatus); });
    m_player->addProperty("Position", [this] { return m_position; });
    m_player->addProperty("Rate", [this] { return m_rate; },
                          [this](double value) {
                              // A setter cannot reply with an error; out-of-range rates are ignored.
                              if (!m_canControl || value < m_minimumRate || value > m_maximumRate)
                                  return true;
                              if (value == 0)
                                  pauseRequested.emit();
                              else
                                  rateRequested.emit(value);
                              return true;
                          });
    m_player->addProperty("Shuffle", [this] { return m_shuffle; },
                          [this](bool value) {
                              if (m_canControl)
                                  shuffleRequested.emit(value);
                              return true;
                          });
    m_player->addProperty("Volume", [this] { return m_volume; },
                          [this](double value) {
                              if (m_canControl)
                                  volumeRequested.emit(std::max(0.0, value));
                              return true;
                          });

    m_player->addMethod("Next", "", "", [this](MethodCall& call) {
        if (!m_canControl)
            return call.replyError(kNotSupported, "Wanted to move to next track but it is not supported.");
        call.reply();
        if (m_canGoNext)
            nextRequested.emit();
    });
    m_player->addMethod("Previous", "", "", [this](MethodCall& call) {
        if (!m_canControl)
            return call.replyError(kNotSupported, "Wanted to move to previous track but it is not supported.");
        call.reply();
        if (m_canGoPrevious)
            previousRequested.emit();
    });
    m_player->addMethod("OpenUri", "s", "", [this](MethodCall& call) {
        std::string uri;
        call.read(uri);
        if (!m_canControl)
            return call.replyError(kNotSupported, "Wanted to open an url but it is not supported.");
        const size_t colon = uri.find(':');
        if (colon == std::string::npos || colon == 0)
            return call.replyError(kInvalidArgs, "Wanted to open an url but the url is invalid.");
        const std::string scheme = uri.substr(0, colon);
        const auto& schemes = m_description.supportedUriSchemes;
        if (std::find(schemes.begin(), schemes.end(), scheme) == schemes.end())
            return call.replyError(kNotSupported, "Wanted to open an url but the scheme is not supported.");
        // No MIME type is listed as supported: nothing is opened this way.
        if (m_description.supportedMimeTypes.empty())
            return call.replyError(kNotSupported, "Wanted to open an url but the mime type is not supported.");
        call.reply();
        openUriRequested.emit(uri);
    });
    m_player->addMethod("Pause", "", "", [this](MethodCall& call) {
        if (!m_canControl)
            return call.replyError(kNotSupported, "Wanted to pause but it is not supported.");
        call.reply();
        if (m_canPause && m_playbackStatus != PlaybackStatus::Paused)
            pauseRequested.emit();
    });
    m_player->addMethod("Play", "", "", [this](MethodCall& call) {
        if (!m_canControl)
            return call.replyError(kNotSupported, "Wanted to play but it is not supported.");
        call.reply();
        if (m_canPlay && m_playbackStatus != PlaybackStatus::Playing)
            playRequested.emit();
    });
    m_player->addMethod("PlayPause", "", "", [this](MethodCall& call) {
        if (!m_canControl)
            return call.replyError(kNotSupported, "Wanted to play or pause but it is not supported.");
        if (m_playbackStatus == PlaybackStatus::Playing) {
            if (!m_canPause)
                return call.replyError(kNotSupported, "Wanted to pause but it is not supported.");
            call.reply();
            pauseRequested.emit();
            return;
        }
        if (!m_canPlay)
            return call.replyError(kNotSupported, "Wanted to play but it is not supported.");
        call.reply();
        playRequested.emit();
    });
    m_player->addMethod("Seek", "x", "", [this](MethodCall& call) {
        int64_t offset = 0;
        call.read(offset);
        if (!m_canControl)
            return call.replyError(kNotSupported, "Wanted to seek but it is not supported.");
        call.reply();
        if (!m_canSeek || offset == 0)
            return;
        if (offset > 0 && m_position + offset > m_metadata.length) {
            nextRequested.emit();
            return;
        }
        seekRequested.emit(offset);
    });
    m_player->addMethod("SetPosition", "ox", "", [this](MethodCall& call) {
        ObjectPath track;
        int64_t position = 0;
        call.read(track, position);
        if (!m_canControl)
            return call.replyError(kNotSupported, "Wanted to move to position but it is not supported.");
        if (!m_canSeek || m_metadata.trackId.empty())
            return call.reply();
        if (track.path != m_metadata.trackId)
            return call.replyError(kInvalidArgs, "Wanted to move to position but the TrackId is not the current one.");
        if (position < 0 || position > m_metadata.length)
            return call.replyError(kInvalidArgs, "Wanted to move to position but the position is off range.");
        call.reply();
        setPositionRequested.emit(track.path, position);
    });
    m_player->addMethod("Stop", "", "", [this](MethodCall& call) {
        if (!m_canControl)
            return call.replyError(kNotSupported, "Wanted to stop but it is not supported.");
        call.reply();
        if (m_playbackStatus != PlaybackStatus::Stopped)
            stopRequested.emit();
    });
    m_player->addSignal("Seeked", "x", {"Position"});

    if (!m_root->publish() || !m_player->publish()) {
        LM_WARNING(cfMpris) << "Could not register the media controls object";
        return;
    }
    m_registered = m_bus->requestName(m_name);
    if (!m_registered) {
        LM_WARNING(cfMpris) << "Could not register" << m_name << "- already taken?";
    }
}

MprisServer::~MprisServer()
{
    if (m_registered)
        m_bus->releaseName(m_name);
}

std::string MprisServer::toString(PlaybackStatus status)
{
    switch (status) {
    case PlaybackStatus::Playing:
        return "Playing";
    case PlaybackStatus::Paused:
        return "Paused";
    case PlaybackStatus::Stopped:
        break;
    }
    return "Stopped";
}

std::string MprisServer::toString(LoopStatus status)
{
    switch (status) {
    case LoopStatus::Track:
        return "Track";
    case LoopStatus::Playlist:
        return "Playlist";
    case LoopStatus::None:
        break;
    }
    return "None";
}

VariantMap MprisServer::typedMetadata() const
{
    VariantMap metadata;
    if (!m_metadata.trackId.empty())
        metadata["mpris:trackid"] = Variant(ObjectPath{m_metadata.trackId});
    metadata["xesam:title"] = Variant(m_metadata.title);
    metadata["mpris:length"] = Variant(m_metadata.length);
    metadata["xesam:url"] = Variant(m_metadata.url);
    if (!m_metadata.artUrl.empty())
        metadata["mpris:artUrl"] = Variant(m_metadata.artUrl);
    return metadata;
}

void MprisServer::changedPlayer(const std::vector<std::string>& names)
{
    if (m_player && m_registered)
        m_player->emitPropertiesChanged(names);
}

void MprisServer::setIdentity(const std::string& identity)
{
    if (m_description.identity == identity)
        return;
    m_description.identity = identity;
    if (m_root && m_registered)
        m_root->emitPropertiesChanged({"Identity"});
}

void MprisServer::setFullscreen(bool fullscreen)
{
    if (m_fullscreen == fullscreen)
        return;
    m_fullscreen = fullscreen;
    if (m_root && m_registered)
        m_root->emitPropertiesChanged({"Fullscreen"});
}

void MprisServer::setCanControl(bool can)
{
    if (m_canControl == can)
        return;
    m_canControl = can;
    // CanControl itself is constant on the bus; the rest follow it.
    changedPlayer({"CanGoNext", "CanGoPrevious", "CanPause", "CanPlay", "CanSeek"});
}

void MprisServer::setCanPlay(bool can)
{
    if (m_canPlay == can)
        return;
    m_canPlay = can;
    if (m_canControl)
        changedPlayer({"CanPlay"});
}

void MprisServer::setCanPause(bool can)
{
    if (m_canPause == can)
        return;
    m_canPause = can;
    if (m_canControl)
        changedPlayer({"CanPause"});
}

void MprisServer::setCanSeek(bool can)
{
    if (m_canSeek == can)
        return;
    m_canSeek = can;
    if (m_canControl)
        changedPlayer({"CanSeek"});
}

void MprisServer::setCanGoNext(bool can)
{
    if (m_canGoNext == can)
        return;
    m_canGoNext = can;
    if (m_canControl)
        changedPlayer({"CanGoNext"});
}

void MprisServer::setCanGoPrevious(bool can)
{
    if (m_canGoPrevious == can)
        return;
    m_canGoPrevious = can;
    if (m_canControl)
        changedPlayer({"CanGoPrevious"});
}

void MprisServer::setPlaybackStatus(PlaybackStatus status)
{
    if (m_playbackStatus == status)
        return;
    m_playbackStatus = status;
    changedPlayer({"PlaybackStatus"});
}

void MprisServer::setLoopStatus(LoopStatus status)
{
    if (m_loopStatus == status)
        return;
    m_loopStatus = status;
    changedPlayer({"LoopStatus"});
}

void MprisServer::setRate(double rate)
{
    if (m_rate == rate)
        return;
    m_rate = rate;
    changedPlayer({"Rate"});
}

void MprisServer::setMinimumRate(double rate)
{
    if (m_minimumRate == rate)
        return;
    m_minimumRate = rate;
    if (rate <= 1)
        changedPlayer({"MinimumRate"});
}

void MprisServer::setMaximumRate(double rate)
{
    if (m_maximumRate == rate)
        return;
    m_maximumRate = rate;
    if (rate >= 1)
        changedPlayer({"MaximumRate"});
}

void MprisServer::setVolume(double volume)
{
    if (m_volume == volume)
        return;
    m_volume = volume;
    changedPlayer({"Volume"});
}

void MprisServer::setPosition(int64_t position)
{
    m_position = position;
}

void MprisServer::setMetadata(const Metadata& metadata)
{
    if (m_metadata == metadata)
        return;
    m_metadata = metadata;
    changedPlayer({"Metadata"});
}

void MprisServer::seeked(int64_t position)
{
    if (m_player && m_registered)
        m_player->emitSignal("Seeked", position);
}
