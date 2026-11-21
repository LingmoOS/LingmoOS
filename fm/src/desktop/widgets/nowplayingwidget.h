// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <string>

#include "base/timer.h"
#include "desktop/widgets/widget.h"

namespace Lingmo::ui {
class ImageView;
class Label;
}

namespace fm {

class NowPlayingSource;
class PlaybackButton;

// What a player is doing, from MPRIS over the session bus: the current track,
// its artist, and the controls the player says it accepts.
class NowPlayingWidget : public DesktopWidget
{
public:
    NowPlayingWidget(WidgetLayer& layer, WidgetSize size);
    ~NowPlayingWidget() override;

protected:
    void layoutContent() override;
    void paintContent(ui::Painter& painter) override;
    void retranslateContent() override;

private:
    friend class NowPlayingSource;

    void setTrack(bool present, const std::string& title, const std::string& artist, bool playing,
                  bool canControl, bool canPrevious, bool canNext, const std::string& artUrl);
    void loadArt(const std::string& url);
    // Downloads an HTTP/HTTPS art URL to the local cache and then loads it.
    void downloadArt(const std::string& url);
    void updateControls();

    ui::Label* m_title = nullptr;
    ui::Label* m_artist = nullptr;
    PlaybackButton* m_previous = nullptr;
    PlaybackButton* m_play = nullptr;
    PlaybackButton* m_next = nullptr;
    ui::ImageView* m_artView = nullptr;
    std::unique_ptr<NowPlayingSource> m_source;
    base::Timer m_poll;
    ui::RectF m_artRect;
    std::string m_artUrl;
    std::string m_trackTitle;
    std::string m_trackArtist;
    bool m_present = false;
    bool m_playing = false;
    bool m_canControl = false;
    bool m_canPrevious = false;
    bool m_canNext = false;
    // Alive flag for async download callbacks.
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
