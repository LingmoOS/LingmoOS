// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "controlbar.h"
#include "mainwindow.h"
#include "thumbnailer.h"

#include "base/i18n.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"

#include <algorithm>
#include <cmath>

using namespace Lingmo::ui;
using views::white;
using Lingmo::i18n::tr;

static constexpr double kSidePadding = 14;
static constexpr double kTopPadding = 10;
static constexpr double kMaximumVolume = 150;

// The frame and time over the pointer.
class SeekBar::Preview : public View
{
public:
    Preview()
        : m_chapter(views::addLabel(this, {}, 9, white()))
        , m_time(views::addLabel(this, {}, 9, white()))
    {
        views::setTabular(m_time);
    }

    void set(const std::string& chapter, const std::string& time, Image frame)
    {
        m_chapter->setText(chapter);
        m_chapter->setVisible(!chapter.empty());
        m_time->setText(time);
        m_frame = std::move(frame);
        requestLayout();
        requestPaint();
    }

    // The preview's width, as Column's: frame, pill, and the labels in it.
    SizeF size() const
    {
        const double timeWidth = m_time->sizeHint().width;
        const double chapterWidth = m_chapter->isVisible() ? m_chapter->sizeHint().width : 0;
        const double width = std::max({m_frame.isNull() ? 0.0 : 176.0, timeWidth + 16,
                                       m_chapter->isVisible() ? chapterWidth + 16 : 0.0});
        return {width, frameHeight(width) + (m_frame.isNull() ? 0 : 6) + pillHeight()};
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double top = m_frame.isNull() ? 0 : frameHeight(width) + 6;
        const SizeF time = m_time->sizeHint();
        const SizeF chapter = m_chapter->isVisible() ? m_chapter->sizeHint() : SizeF();
        const double labelsWidth = std::max(time.width, chapter.width);
        const double pillWidth = labelsWidth + 16;
        const double pillX = rounding::centeredStart(width, pillWidth);
        const double labelsX = pillX + rounding::centeredStart(pillWidth, labelsWidth);
        const double labelsY = top + rounding::centeredStart(pillHeight(), chapter.height + time.height);
        if (m_chapter->isVisible())
            m_chapter->setGeometry({labelsX + rounding::centeredStart(labelsWidth, chapter.width), labelsY, chapter.width,
                                    chapter.height});
        m_time->setGeometry({labelsX + rounding::centeredStart(labelsWidth, time.width), labelsY + chapter.height,
                             time.width, time.height});
    }

    void paint(Painter& painter) override
    {
        const double width = geometry().width;
        double top = 0;
        if (!m_frame.isNull()) {
            const double height = frameHeight(width);
            const RectF frame{0, 0, width, height};
            const double border = 1 / painter.scale();
            painter.drawRoundedRect(frame, {Theme::current().smallRadius, 0, Corners::Circular}, Color{0, 0, 0, 255},
                                    border, white(0.3));
            // PreserveAspectFit inside the border.
            const RectF inner = frame.adjusted(border, border, -border, -border);
            const double scale = std::min(inner.width / m_frame.width(), inner.height / m_frame.height());
            const SizeF shown{m_frame.width() * scale, m_frame.height() * scale};
            painter.save();
            painter.clipRoundedRect(inner, Theme::current().smallRadius - border);
            painter.drawImage(m_frame, {inner.x + (inner.width - shown.width) / 2,
                                        inner.y + (inner.height - shown.height) / 2, shown.width, shown.height});
            painter.restore();
            top = height + 6;
        }
        const SizeF time = m_time->sizeHint();
        const SizeF chapter = m_chapter->isVisible() ? m_chapter->sizeHint() : SizeF();
        const double pillWidth = std::max(time.width, chapter.width) + 16;
        const RectF pill{rounding::centeredStart(width, pillWidth), top, pillWidth, pillHeight()};
        painter.drawRoundedRect(pill, {pill.height / 2, 0, Corners::Circular}, views::rgba(0.08, 0.08, 0.09, 0.85));
    }

private:
    double frameHeight(double width) const
    {
        if (m_frame.isNull())
            return 0;
        return std::round(width * double(m_frame.height()) / std::max(1, m_frame.width()));
    }

    double pillHeight() const
    {
        const double chapter = m_chapter->isVisible() ? m_chapter->sizeHint().height : 0;
        return chapter + m_time->sizeHint().height + 8;
    }

    Label* m_chapter;
    Label* m_time;
    Image m_frame;
};

SeekBar::SeekBar(MainWindow& owner)
    : m_owner(owner)
    , m_preview(addChild<Preview>())
{
    setTracksHover(true);
    m_preview->setVisible(false);
    m_trackHeight.onUpdate = [this] { requestPaint(); };
    m_handleScale.onUpdate = [this] { requestPaint(); };
}

void SeekBar::update(double duration, double position, const std::vector<Player::Chapter>& chapters,
                     Thumbnailer* thumbnailer, bool enabled)
{
    const bool changed = duration != m_duration || position != m_position || chapters != m_chapters
            || thumbnailer != m_thumbnailer || enabled != m_enabled;
    m_duration = duration;
    m_position = position;
    m_chapters = chapters;
    m_thumbnailer = thumbnailer;
    if (enabled != m_enabled) {
        m_enabled = enabled;
        if (!enabled) {
            m_dragging = false;
            setHovered(false);
        }
    }
    if (changed)
        requestPaint();
    updatePreview();
}

double SeekBar::timeAt(double x) const
{
    const double width = geometry().width;
    return width > 0 ? std::clamp(x / width, 0.0, 1.0) * m_duration : 0;
}

std::string SeekBar::chapterAt(double seconds) const
{
    std::string title;
    for (const Player::Chapter& chapter : m_chapters) {
        if (chapter.time <= seconds)
            title = chapter.title;
    }
    return title;
}

void SeekBar::setHovered(bool hovered)
{
    if (m_hovered == hovered)
        return;
    m_hovered = hovered;
    const bool active = m_hovered || m_dragging;
    m_trackHeight.setTarget(this, active ? 6 : 4);
    m_handleScale.setTarget(this, active ? 1 : 0);
    updatePreview();
    m_owner.stateChanged.emit();
}

void SeekBar::updatePreview()
{
    const bool visible = (m_hovered || m_dragging) && m_duration > 0;
    if (!visible) {
        m_preview->setVisible(false);
        return;
    }
    const double time = timeAt(m_mouseX);
    Image frame;
    if (m_thumbnailer) {
        const int bucket = m_thumbnailer->bucket(time);
        if (bucket >= 0) {
            if (m_thumbnailer->has(bucket))
                frame = m_thumbnailer->image(bucket);
            else
                m_thumbnailer->request(bucket);
        }
    }
    m_preview->set(chapterAt(time), ::Application::formatTime(time, m_duration), std::move(frame));
    const SizeF size = m_preview->size();
    const double width = geometry().width;
    const double x = std::round(std::clamp(m_mouseX - size.width / 2, -8.0, width - size.width + 8));
    // 14 above the 20 px bar, which starts kHitMargin into this view.
    m_preview->setGeometry({x, kHitMargin - size.height - 14, size.width, size.height});
    m_preview->setVisible(true);
}

void SeekBar::layout()
{
    updatePreview();
}

void SeekBar::paint(Painter& painter)
{
    const double width = geometry().width;
    const double height = m_trackHeight.value();
    const double top = kHitMargin + rounding::halfAwayFromZero(10) - rounding::halfAwayFromZero(height / 2);
    const RectF track{0, top, width, height};
    painter.drawRoundedRect(track, {height / 2, 0, Corners::Circular}, white(0.25));
    if (m_duration <= 0)
        return;
    const double fraction = std::min(1.0, shownPosition() / m_duration);
    painter.drawRoundedRect({0, top, std::max(height, width * fraction), height}, {height / 2, 0, Corners::Circular},
                            white());
    // Chapter boundaries cut thin gaps into the track.
    for (const Player::Chapter& chapter : m_chapters) {
        if (chapter.time <= 0)
            continue;
        const double x = std::round(width * chapter.time / m_duration) - 1;
        painter.drawRoundedRect({x, top, 2, height}, {0, 0, Corners::Circular}, Color{0, 0, 0, 140});
    }
    const double scale = m_handleScale.value();
    if (scale > 0) {
        const double size = 14 * scale;
        const double centreX = width * fraction;
        const double centreY = kHitMargin + 10;
        painter.drawRoundedRect({centreX - size / 2, centreY - size / 2, size, size},
                                {size / 2, 0, Corners::Circular}, white());
    }
}

void SeekBar::pointerEnter()
{
    setHovered(true);
}

void SeekBar::pointerLeave()
{
    if (!m_dragging)
        setHovered(false);
}

void SeekBar::hoverMove(const PointerEvent& event)
{
    if (m_dragging)
        return;
    m_mouseX = event.position.x;
    updatePreview();
}

void SeekBar::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left || !m_enabled)
        return;
    m_mouseX = event.position.x;
    m_dragging = true;
    m_dragTime = timeAt(event.position.x);
    m_owner.player().seek(m_dragTime, false);
    if (onDraggingChanged)
        onDraggingChanged();
    requestPaint();
    updatePreview();
}

void SeekBar::pointerMove(const PointerEvent& event)
{
    m_mouseX = event.position.x;
    if (m_dragging) {
        m_dragTime = timeAt(event.position.x);
        m_owner.player().seek(m_dragTime, false);
        requestPaint();
    }
    updatePreview();
}

void SeekBar::pointerRelease(const PointerEvent& event)
{
    if (!m_dragging)
        return;
    m_mouseX = event.position.x;
    m_dragTime = timeAt(event.position.x);
    m_owner.player().seek(m_dragTime, true);
    m_dragging = false;
    if (!localRect().contains(event.position))
        setHovered(false);
    if (onDraggingChanged)
        onDraggingChanged();
    requestPaint();
    updatePreview();
}

VolumeControl::VolumeControl(MainWindow& owner)
    : m_owner(owner)
    , m_mute(addChild<views::ControlButton>("volume-high"))
{
    m_mute->setName("muteButton");
    m_mute->onClicked = [this] { m_owner.toggleMute(); };
}

void VolumeControl::update(int volume, bool muted)
{
    if (m_volume == volume && m_muted == muted)
        return;
    m_volume = volume;
    m_muted = muted;
    m_mute->setIcon(muted || volume == 0 ? "volume-muted"
                    : volume < 34      ? "volume-low"
                    : volume < 67      ? "volume-medium"
                                       : "volume-high");
    retranslate();
    requestPaint();
}

void VolumeControl::retranslate()
{
    m_mute->setToolTipText(m_muted ? tr("Unmute") : tr("Mute"));
}

RectF VolumeControl::trackRect() const
{
    // The slider is 72x36 after the 36 px button and 2 px of spacing; its
    // track is inset 6 and 4 tall, centred.
    return {38 + 6, rounding::centeredStart(36, 4), 72 - 12, 4};
}

void VolumeControl::layout()
{
    m_mute->setGeometry({0, 0, 36, 36});
}

void VolumeControl::paint(Painter& painter)
{
    const RectF track = trackRect();
    const double shown = m_muted ? 0 : m_volume / kMaximumVolume;
    const double notch = 100 / kMaximumVolume;
    painter.drawRoundedRect(track, {2, 0, Corners::Circular}, white(0.25));
    if (shown > 0)
        painter.drawRoundedRect({track.x, track.y, std::max(track.height, track.width * std::min(shown, notch)),
                                 track.height},
                                {2, 0, Corners::Circular}, white());
    // The boost above 100% is drawn in a warning colour.
    if (shown > notch)
        painter.drawRoundedRect({track.x + track.width * notch, track.y, track.width * (shown - notch), track.height},
                                {2, 0, Corners::Circular}, Lingmo::ui::accent::orange);
    painter.drawRoundedRect({track.x + std::round(track.width * notch) - 1,
                             track.y + rounding::centeredStart(track.height, 10), 2, 10},
                            {1, 0, Corners::Circular}, white(0.6));
    if (m_hovered || m_dragging)
        painter.drawRoundedRect({track.x + track.width * shown - 6, track.y + rounding::centeredStart(track.height, 12), 12,
                                 12},
                                {6, 0, Corners::Circular}, white());
}

void VolumeControl::apply(double x)
{
    const RectF track = trackRect();
    int value = int(std::lround(std::clamp((x - track.x) / track.width, 0.0, 1.0) * kMaximumVolume));
    // The notch holds a little, so 100% is easy to hit.
    if (std::abs(value - 100) <= 4)
        value = 100;
    m_owner.setVolume(value);
}

void VolumeControl::pointerEnter()
{
    m_hovered = true;
    requestPaint();
}

void VolumeControl::pointerLeave()
{
    m_hovered = false;
    requestPaint();
}

void VolumeControl::pointerPress(const PointerEvent& event)
{
    if (event.button != MouseButton::Left || event.position.x < 38)
        return;
    m_dragging = true;
    apply(event.position.x);
    if (onDraggingChanged)
        onDraggingChanged();
    requestPaint();
}

void VolumeControl::pointerMove(const PointerEvent& event)
{
    if (m_dragging)
        apply(event.position.x);
}

void VolumeControl::pointerRelease(const PointerEvent&)
{
    if (!m_dragging)
        return;
    m_dragging = false;
    if (onDraggingChanged)
        onDraggingChanged();
    requestPaint();
}

bool VolumeControl::scroll(const ScrollEvent& event)
{
    // angleDelta.y is positive away from the user, the opposite of deltaY.
    const double delta = event.value120Y != 0 ? -event.value120Y : -event.deltaY;
    const int step = delta > 0 ? 5 : delta < 0 ? -5 : 0;
    if (step != 0)
        m_owner.setVolume(std::clamp(m_volume + step, 0, std::max(100, m_volume)));
    return true;
}

ControlBar::ControlBar(MainWindow& owner)
    : m_owner(owner)
    , m_volume(addChild<VolumeControl>(owner))
    , m_skipBack(addChild<views::ControlButton>("skip-back"))
    , m_play(addChild<views::ControlButton>("play", 40, 24))
    , m_skipForward(addChild<views::ControlButton>("skip-forward"))
    , m_panel(addChild<views::ControlButton>("sidebar"))
    , m_fullScreen(addChild<views::ControlButton>("fullscreen"))
    , m_elapsed(views::addLabel(this, "0:00", 8, white()))
    , m_seekBar(addChild<SeekBar>(owner))
    , m_totalArea(addChild<views::Clickable>())
    , m_total(views::addLabel(m_totalArea, "0:00", 8, white()))
    , m_metrics(views::addLabel(this, "-0:00", 8))
{
    setName("controlBar");
    setTracksHover(true);
    m_seekBar->setName("seekBar");
    m_elapsed->setName("elapsedLabel");
    m_total->setName("totalLabel");
    m_skipBack->setName("skipBackButton");
    m_play->setName("playButton");
    m_skipForward->setName("skipForwardButton");
    m_panel->setName("sidePanelButton");
    m_fullScreen->setName("fullscreenButton");
    for (Label* label : {m_elapsed, m_total, m_metrics})
        views::setTabular(label);
    m_metrics->setVisible(false);

    retranslate();
    m_skipBack->onClicked = [this] { m_owner.seekBy(-10); };
    m_play->onClicked = [this] { m_owner.togglePause(false); };
    m_skipForward->onClicked = [this] { m_owner.seekBy(10); };
    m_panel->onClicked = [this] { m_owner.togglePanel(-1, false); };
    m_fullScreen->onClicked = [this] { m_owner.toggleFullScreen(); };
    m_totalArea->onClicked = [this] {
        m_showRemaining = !m_showRemaining;
        refresh();
    };
    m_seekBar->onDraggingChanged = [this] { m_owner.stateChanged.emit(); };
    m_volume->onDraggingChanged = [this] { m_owner.stateChanged.emit(); };
}

void ControlBar::retranslate()
{
    m_skipBack->setToolTipText(tr("Back 10 Seconds"));
    m_skipForward->setToolTipText(tr("Forward 10 Seconds"));
    m_panel->setToolTipText(tr("Playlist, Tracks and Playback"));
    m_volume->retranslate();
}

void ControlBar::refresh()
{
    const Player& player = m_owner.player();
    m_volume->update(player.volume(), player.muted());
    m_skipBack->setEnabled(player.seekable());
    m_skipForward->setEnabled(player.seekable());
    m_play->setIcon(player.endReached() ? "replay" : player.paused() ? "play" : "pause");
    m_play->setToolTipText(player.endReached() ? tr("Play Again") : player.paused() ? tr("Play") : tr("Pause"));
    m_panel->setChecked(m_owner.panelOpen());
    m_fullScreen->setIcon(m_owner.isFullScreen() ? "fullscreen-exit" : "fullscreen");
    m_fullScreen->setToolTipText(m_owner.isFullScreen() ? tr("Exit Full Screen") : tr("Full Screen"));

    m_seekBar->update(player.duration(), player.position(), player.chapters(),
                      player.hasVideo() ? &m_owner.app().thumbnailer() : nullptr,
                      player.seekable() && player.duration() > 0);
    const double duration = player.duration();
    const double shown = m_seekBar->shownPosition();
    const std::string metrics = "-" + ::Application::formatTime(duration, duration);
    const std::string elapsed = ::Application::formatTime(shown, duration);
    const std::string total = m_showRemaining ? "-" + ::Application::formatTime(std::max(0.0, duration - shown), duration)
                                              : ::Application::formatTime(duration, duration);
    if (m_metrics->text() != metrics || m_elapsed->text() != elapsed || m_total->text() != total) {
        const bool relayout = m_metrics->text() != metrics;
        m_metrics->setText(metrics);
        m_elapsed->setText(elapsed);
        m_total->setText(total);
        if (relayout)
            requestLayout();
        else
            layout();
    }
}

void ControlBar::layout()
{
    const double width = geometry().width;

    // Row one: equal side columns keep the transport centred on the plate.
    const double rowTop = kTopPadding;
    const double rowHeight = 40;
    const double side = std::max(m_volume->sizeHint().width, 36.0 + 2 + 36);
    const double transportWidth = 36 + 6 + 40 + 6 + 36;
    const double inner = width - 2 * kSidePadding;
    const double spare = std::max(0.0, inner - 2 * side - transportWidth);
    const double transportX = kSidePadding + side + std::round(spare / 2);
    m_volume->setGeometry({kSidePadding, rowTop + rounding::centeredStart(rowHeight, 36), m_volume->sizeHint().width, 36});
    m_skipBack->setGeometry({transportX, rowTop + 2, 36, 36});
    m_play->setGeometry({transportX + 42, rowTop, 40, 40});
    m_skipForward->setGeometry({transportX + 88, rowTop + 2, 36, 36});
    const double right = width - kSidePadding;
    m_fullScreen->setGeometry({right - 36, rowTop + 2, 36, 36});
    m_panel->setGeometry({right - 36 - 2 - 36, rowTop + 2, 36, 36});

    // Row two: the times line up with the outer glyphs above them.
    const double timeTop = rowTop + rowHeight + 2;
    const double timeHeight = 20;
    const double labelWidth = std::max(std::ceil(m_elapsed->sizeHint().width), m_metrics->sizeHint().width);
    const double totalWidth = std::max(std::ceil(m_total->sizeHint().width), m_metrics->sizeHint().width);
    const double left = kSidePadding + 8;
    const double end = width - kSidePadding - 8;
    const double labelHeight = m_elapsed->sizeHint().height;
    const double labelY = timeTop + views::layoutCenter(timeHeight, labelHeight);
    m_elapsed->setTextAlignment(Label::TextAlignment::Left);
    m_elapsed->setGeometry({left, labelY, labelWidth, labelHeight});
    m_totalArea->setGeometry({end - totalWidth, labelY, totalWidth, labelHeight});
    m_total->setTextAlignment(Label::TextAlignment::Right);
    m_total->setGeometry({0, 0, totalWidth, labelHeight});
    const double barX = left + labelWidth + 10;
    const double barWidth = std::max(0.0, end - totalWidth - 10 - barX);
    m_seekBar->setGeometry({barX, timeTop - SeekBar::kHitMargin, barWidth, timeHeight + 2 * SeekBar::kHitMargin});
}

void ControlBar::paint(Painter& painter)
{
    views::paintPlate(painter, localRect(), 20);
}
