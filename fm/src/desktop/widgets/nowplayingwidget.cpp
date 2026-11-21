// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktop/widgets/nowplayingwidget.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/strings.h"
#include "base/thread_pool.h"
#include "base/url.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/path.h"
#include "ui/theme.h"

#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <functional>

static const char* tr(const char* text) { return Lingmo::i18n::dtr("lingmo-filemanager", text); }

namespace fm {

namespace {

constexpr const char* kMprisPrefix = "org.mpris.MediaPlayer2.";
constexpr const char* kMprisPath = "/org/mpris/MediaPlayer2";
constexpr const char* kMprisPlayerInterface = "org.mpris.MediaPlayer2.Player";

std::string stringOf(const base::dbus::VariantMap& properties, const char* name)
{
    const auto it = properties.find(name);
    return it == properties.end() ? std::string() : it->second.value<std::string>();
}

bool flagOf(const base::dbus::VariantMap& properties, const char* name, bool fallback)
{
    const auto it = properties.find(name);
    return it == properties.end() ? fallback : it->second.value<bool>(fallback);
}

std::string artistOf(const base::dbus::VariantMap& metadata)
{
    const auto it = metadata.find("xesam:artist");
    if (it == metadata.end())
        return {};
    // A list by the spec; a few players send one string.
    if (const auto artists = it->second.get<std::vector<std::string>>())
        return Lingmo::base::strings::join(*artists, ", ");
    return it->second.value<std::string>();
}

} // namespace

// A small playback control drawn with the theme's text colour.
class PlaybackButton final : public ui::View
{
public:
    enum class Shape {
        Previous,
        Play,
        Pause,
        Next,
    };

    explicit PlaybackButton(Shape shape)
        : m_shape(shape)
    {
        setCursorShape(ui::CursorShape::Pointer);
    }

    void setShape(Shape shape)
    {
        if (m_shape == shape)
            return;
        m_shape = shape;
        requestPaint();
    }

    bool acceptsPointer() const override { return true; }
    std::function<void()> onClicked;

    void paint(ui::Painter& painter) override
    {
        const ui::Theme& theme = widgetTheme(this);
        const ui::Color color = theme.text.withAlpha(isEnabled() ? (m_hovered ? 1.0 : 0.72) : 0.26);
        const ui::RectF rect = localRect();
        const double cx = rect.width / 2;
        const double cy = rect.height / 2;
        const auto triangle = [&painter, color](double x, double y, bool forward) {
            ui::Path path;
            path.moveTo(x, y - 5);
            path.lineTo(x, y + 5);
            path.lineTo(forward ? x + 9 : x - 9, y);
            path.close();
            painter.fillPath(path, color);
        };
        switch (m_shape) {
        case Shape::Previous:
            painter.fillRoundedRect({cx - 7, cy - 5, 2, 10}, 1, color);
            triangle(cx - 4, cy, false);
            break;
        case Shape::Play:
            triangle(cx - 3, cy, true);
            break;
        case Shape::Pause:
            painter.fillRoundedRect({cx - 5, cy - 5, 3, 10}, 1, color);
            painter.fillRoundedRect({cx + 2, cy - 5, 3, 10}, 1, color);
            break;
        case Shape::Next:
            triangle(cx + 4, cy, true);
            painter.fillRoundedRect({cx + 7, cy - 5, 2, 10}, 1, color);
            break;
        }
    }

protected:
    void pointerEnter() override
    {
        m_hovered = true;
        requestPaint();
    }
    void pointerLeave() override
    {
        m_hovered = false;
        requestPaint();
    }
    void pointerRelease(const ui::PointerEvent& event) override
    {
        if (isEnabled() && localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    Shape m_shape;
    bool m_hovered = false;
};

// The session's MPRIS players. A player is chosen once there is one, and the
// card follows its PropertiesChanged signals; a short poll catches players
// that do not announce a track change.
class NowPlayingSource
{
public:
    explicit NowPlayingSource(NowPlayingWidget& widget)
        : m_widget(widget)
        , m_bus(base::dbus::Connection::session())
        , m_alive(std::make_shared<bool>(true))
    {
        if (!m_bus) {
            m_widget.setTrack(false, {}, {}, false, false, false, false, {});
            return;
        }

        m_daemon = base::dbus::Proxy(m_bus, "org.freedesktop.DBus", "/org/freedesktop/DBus",
                                     "org.freedesktop.DBus");
        base::dbus::MatchRule rule;
        rule.sender = "org.freedesktop.DBus";
        rule.path = "/org/freedesktop/DBus";
        rule.interface = "org.freedesktop.DBus";
        rule.member = "NameOwnerChanged";
        m_names = m_bus->subscribe(rule, [this, alive = m_alive](const base::dbus::Message&) {
            if (*alive)
                refresh();
        });
        refresh();
    }

    ~NowPlayingSource()
    {
        *m_alive = false;
        m_names = {};
        m_properties = {};
    }

    void refresh()
    {
        if (!m_bus)
            return;
        m_listing = m_daemon.callAsync("ListNames", [this, alive = m_alive](const base::dbus::Reply& reply) {
            if (!*alive)
                return;
            const std::vector<std::string> names =
                    reply.value<std::vector<std::string>>().value_or(std::vector<std::string>());
            choose(names);
        });
    }

    void previous()
    {
        if (m_canControl && m_canPrevious)
            m_player.send("Previous");
    }

    void playPause()
    {
        if (m_canControl)
            m_player.send("PlayPause");
    }

    void next()
    {
        if (m_canControl && m_canNext)
            m_player.send("Next");
    }

private:
    void choose(const std::vector<std::string>& names)
    {
        std::string chosen;
        for (const std::string& name : names) {
            if (!name.starts_with(kMprisPrefix))
                continue;
            if (chosen.empty() || name == m_service)
                chosen = name;
        }
        if (chosen.empty()) {
            m_service.clear();
            m_player = {};
            m_properties = {};
            m_metadata.clear();
            m_widget.setTrack(false, {}, {}, false, false, false, false, {});
            return;
        }
        if (chosen != m_service) {
            m_service = chosen;
            m_player = base::dbus::Proxy(m_bus, m_service, kMprisPath, kMprisPlayerInterface);
            m_properties = m_player.onPropertiesChanged(
                    [this, alive = m_alive](const base::dbus::VariantMap& changed,
                                            const std::vector<std::string>& invalidated) {
                        if (!*alive)
                            return;
                        for (const auto& [name, value] : changed)
                            m_map[name] = value;
                        if (!invalidated.empty()) {
                            load();
                            return;
                        }
                        publish();
                    });
        }
        load();
    }

    void load()
    {
        m_loading = m_player.getAllAsync([this, alive = m_alive](const base::dbus::Reply& reply) {
            if (!*alive)
                return;
            const auto properties = reply.value<base::dbus::VariantMap>();
            if (!properties) {
                m_widget.setTrack(false, {}, {}, false, false, false, false, {});
                return;
            }
            m_map = *properties;
            m_metadata.clear();
            const auto metadata = m_map.find("Metadata");
            if (metadata != m_map.end())
                m_metadata = metadata->second.value<base::dbus::VariantMap>();
            publish();
        });
    }

    void publish()
    {
        const std::string title = stringOf(m_metadata, "xesam:title");
        const std::string status = stringOf(m_map, "PlaybackStatus");
        m_canControl = flagOf(m_map, "CanControl", false);
        m_canPrevious = flagOf(m_map, "CanGoPrevious", false);
        m_canNext = flagOf(m_map, "CanGoNext", false);
        m_widget.setTrack(!title.empty() || status == "Playing" || status == "Paused", title,
                          artistOf(m_metadata), status == "Playing", m_canControl, m_canPrevious,
                          m_canNext, stringOf(m_metadata, "mpris:artUrl"));
    }

    NowPlayingWidget& m_widget;
    base::dbus::Connection* m_bus = nullptr;
    std::shared_ptr<bool> m_alive;
    base::dbus::Proxy m_daemon;
    base::dbus::Proxy m_player;
    base::dbus::Match m_names;
    base::dbus::Match m_properties;
    base::dbus::PendingCall m_listing;
    base::dbus::PendingCall m_loading;
    base::dbus::VariantMap m_map;
    base::dbus::VariantMap m_metadata;
    std::string m_service;
    bool m_canControl = false;
    bool m_canPrevious = false;
    bool m_canNext = false;
};

NowPlayingWidget::NowPlayingWidget(WidgetLayer& layer, WidgetSize size)
    : DesktopWidget(layer, WidgetKind::NowPlaying, size)
{
    setName("nowplaying-widget");
    m_title = addChild<ui::Label>();
    m_title->setWeight(600);
    m_title->setElide(ui::Label::Elide::Right);
    m_artist = addChild<ui::Label>();
    m_artist->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.62)));
    m_artist->setElide(ui::Label::Elide::Right);

    m_previous = addChild<PlaybackButton>(PlaybackButton::Shape::Previous);
    m_previous->setToolTip(tr("Previous"));
    m_previous->onClicked = [this] {
        if (m_source)
            m_source->previous();
    };
    m_play = addChild<PlaybackButton>(PlaybackButton::Shape::Play);
    m_play->setToolTip(tr("Play"));
    m_play->onClicked = [this] {
        if (m_source)
            m_source->playPause();
    };
    m_next = addChild<PlaybackButton>(PlaybackButton::Shape::Next);
    m_next->setToolTip(tr("Next"));
    m_next->onClicked = [this] {
        if (m_source)
            m_source->next();
    };
    m_artView = addChild<ui::ImageView>();
    m_artView->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
    m_artView->setAutoTransform(true);
    m_artView->setVisible(false);

    setTrack(false, {}, {}, false, false, false, false, {});
    m_source = std::make_unique<NowPlayingSource>(*this);
    m_poll.setInterval(std::chrono::seconds(10));
    m_poll.setSingleShot(false);
    m_poll.onTimeout = [this] {
        if (m_source)
            m_source->refresh();
    };
    m_poll.start();
}

NowPlayingWidget::~NowPlayingWidget()
{
    *m_alive = false;
    m_poll.stop();
    m_source.reset();
}

void NowPlayingWidget::setTrack(bool present, const std::string& title, const std::string& artist,
                                bool playing, bool canControl, bool canPrevious, bool canNext,
                                const std::string& artUrl)
{
    m_present = present;
    m_playing = playing;
    m_canControl = canControl;
    m_canPrevious = canPrevious;
    m_canNext = canNext;
    m_trackTitle = title;
    m_trackArtist = artist;
    m_title->setText(present ? (title.empty() ? tr("Unknown track") : title) : tr("Nothing playing"));
    m_artist->setText(present ? artist : tr("No media"));
    loadArt(present ? artUrl : std::string());
    updateControls();
    requestLayout();
    requestPaint();
}

void NowPlayingWidget::loadArt(const std::string& url)
{
    if (url == m_artUrl) {
        // Artwork is hidden while the widget layer is monochrome. The URL
        // does not change when focus returns to the desktop, so refresh the
        // visibility here instead of waiting for a new MPRIS metadata event.
        m_artView->setVisible(!m_artView->source().empty() && !widgetMonochrome(this));
        m_artView->setSourceSize({m_artRect.width, m_artRect.height});
        requestLayout();
        requestPaint();
        return;
    }
    m_artUrl = url;
    std::string path;
    if (!url.empty()) {
        // Local file (file:// URL)
        if (const std::optional<std::string> local = base::url::toLocalFile(url)) {
            path = *local;
        } else if (url.starts_with("http://") || url.starts_with("https://")) {
            // Remote URL: download asynchronously to cache
            downloadArt(url);
            return;
        }
    }
    m_artView->setSource(path);
    m_artView->setVisible(!path.empty() && !widgetMonochrome(this));
    m_artView->setSourceSize({m_artRect.width, m_artRect.height});
}

void NowPlayingWidget::downloadArt(const std::string& url)
{
    // Build a stable cache path from the URL hash
    const std::string cacheDir = base::paths::cacheHome() + "/lingmo-filemanager/album-art/";
    std::filesystem::create_directories(cacheDir);
    const std::string filename = std::to_string(std::hash<std::string>{}(url)) + ".jpg";
    const std::string cachePath = cacheDir + filename;

    // Already cached — load immediately
    if (std::filesystem::exists(cachePath)) {
        m_artView->setSource(cachePath);
        m_artView->setVisible(!widgetMonochrome(this));
        m_artView->setSourceSize({m_artRect.width, m_artRect.height});
        return;
    }

    // Hide the art while downloading
    m_artView->setSource({});
    m_artView->setVisible(false);

    base::EventLoop* loop = base::EventLoop::current();
    if (!loop)
        return;

    base::ThreadPool::global().post([url, cachePath, loop, alive = m_alive, this] {
        // Download via libcurl on the worker thread
        CURL* curl = curl_easy_init();
        if (!curl)
            return;

        std::string body;
        const auto appendBody = [](char* data, size_t size, size_t count, void* user) -> size_t {
            static_cast<std::string*>(user)->append(data, size * count);
            return size * count;
        };

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "lingmo-filemanager/5.0");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +appendBody);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &body);

        const CURLcode result = curl_easy_perform(curl);
        long status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        curl_easy_cleanup(curl);

        bool ok = (result == CURLE_OK && status >= 200 && status < 300 && !body.empty());
        if (ok) {
            // Write to cache file
            std::ofstream file(cachePath, std::ios::binary);
            ok = file.good();
            if (ok)
                file.write(body.data(), std::streamsize(body.size()));
        }

        loop->post([alive, ok, cachePath, this] {
            if (!*alive)
                return;
            if (ok) {
                m_artView->setSource(cachePath);
                m_artView->setVisible(!widgetMonochrome(this));
                m_artView->setSourceSize({m_artRect.width, m_artRect.height});
            }
        });
    });
}

void NowPlayingWidget::updateControls()
{
    m_play->setShape(m_playing ? PlaybackButton::Shape::Pause : PlaybackButton::Shape::Play);
    m_play->setToolTip(m_playing ? tr("Pause") : tr("Play"));
    m_previous->setEnabled(m_present && m_canControl && m_canPrevious);
    m_next->setEnabled(m_present && m_canControl && m_canNext);
    m_play->setEnabled(m_present && m_canControl);
    m_previous->setToolTip(tr("Previous"));
    m_next->setToolTip(tr("Next"));
}

void NowPlayingWidget::layoutContent()
{
    const ui::RectF content = contentRect();
    if (content.width <= 0 || content.height <= 0)
        return;
    const bool small = size() == WidgetSize::Small;
    const bool large = size() == WidgetSize::Large;
    const double art = small ? 52 : large ? 124 : 78;
    m_artRect = {content.x, content.y, art, art};

    const double textX = small ? content.x : content.x + art + 12;
    const double textWidth = std::max(0.0, content.right() - textX);
    const double titleHeight = std::max(18.0, m_title->sizeHint().height);
    m_title->setFontSize(small ? 12 : large ? 17 : 14);
    m_title->setGeometry({textX, content.y + (small ? art + 8 : 1), textWidth, titleHeight});
    m_artist->setFontSize(small ? 10 : large ? 12 : 11);
    m_artist->setGeometry({textX, content.y + (small ? art + 10 : 2) + titleHeight, textWidth,
                           small ? 16.0 : std::max(0.0, content.height - titleHeight - 30)});

    m_artist->setColor(widgetTheme(this).text.withAlpha(.65));
    const double button = small ? 24 : 30;
    const double gap = 2;
    const double controlsWidth = button * 3 + gap * 2;
    const double controlsY = content.bottom() - button;
    const double controlsX = std::max(content.x, content.right() - controlsWidth);
    m_previous->setGeometry({controlsX, controlsY, button, button});
    m_play->setGeometry({controlsX + button + gap, controlsY, button, button});
    m_next->setGeometry({controlsX + (button + gap) * 2, controlsY, button, button});

    m_artView->setVisible(!m_artView->source().empty() && !widgetMonochrome(this));
    m_artView->setGeometry(m_artRect);
    m_artView->setCornerRadius(size() == WidgetSize::Small ? 8 : 12);
    m_artView->setSourceSize({m_artRect.width, m_artRect.height});
}

void NowPlayingWidget::paintContent(ui::Painter& painter)
{
    if (m_artRect.width <= 0)
        return;
    const ui::Theme& theme = widgetTheme(this);
    const double radius = size() == WidgetSize::Small ? 8 : 12;
    painter.fillRoundedRect(m_artRect, radius,
                            theme.highlight.withAlpha(theme.darkMode ? 0.18 : 0.11));
    const double note = std::min(m_artRect.width, m_artRect.height) * 0.44;
    const double cx = m_artRect.x + m_artRect.width / 2;
    const double cy = m_artRect.y + m_artRect.height / 2;
    const ui::Color ink = theme.text.withAlpha(m_present ? 0.72 : 0.32);
    painter.fillEllipse({cx - note * 0.56, cy + note * 0.18, note * 0.52, note * 0.38}, ink);
    painter.fillEllipse({cx + note * 0.18, cy + note * 0.30, note * 0.52, note * 0.38}, ink);
    painter.drawLine({cx - note * 0.32, cy + note * 0.30}, {cx - note * 0.32, cy - note * 0.62},
                     ink, std::max(1.0, note * 0.08));
    painter.drawLine({cx + note * 0.42, cy + note * 0.42}, {cx + note * 0.42, cy - note * 0.50},
                     ink, std::max(1.0, note * 0.08));
    painter.drawLine({cx - note * 0.32, cy - note * 0.62}, {cx + note * 0.42, cy - note * 0.50},
                     ink, std::max(1.0, note * 0.08));
    painter.drawRoundedRect(m_artRect, {radius, 0, ui::Corners::Continuous}, ui::Color {0, 0, 0, 0},
                            1, theme.text.withAlpha(0.08));
}

void NowPlayingWidget::retranslateContent()
{
    m_artist->setColor(ui::ColorRecipe(widgetTheme(this).text.withAlpha(0.62)));
    loadArt(m_artUrl);
    setTrack(m_present, m_trackTitle, m_trackArtist, m_playing, m_canControl, m_canPrevious,
             m_canNext, m_artUrl);
}

} // namespace fm
