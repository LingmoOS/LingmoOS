// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sidepanel.h"
#include "mainwindow.h"
#include "urls.h"

#include "base/i18n.h"
#include "ui/drag.h"
#include "ui/keys.h"
#include "ui/list_view.h"
#include "ui/cell_layout.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"

#include <algorithm>
#include <cmath>

using namespace Lingmo::ui;
using views::white;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;
using Lingmo::i18n::trNoop;

PanelRow::PanelRow(std::string text, bool checkable, std::string detail)
    : m_check(addChild<Icon>(views::glyph("check"), 16))
    , m_label(views::addLabel(this, std::move(text), 0, white()))
    , m_detail(views::addLabel(this, std::move(detail), 0, white(0.55)))
    , m_checkable(checkable)
{
    m_check->setColor(white());
    m_check->setVisible(false);
    m_label->setElide(Label::Elide::Middle);
    views::setTabular(m_detail);
    m_detail->setVisible(!m_detail->text().empty());
}

void PanelRow::setChecked(bool checked)
{
    m_check->setVisible(m_checkable && checked);
}

void PanelRow::setText(const std::string& text)
{
    if (m_label->text() != text)
        m_label->setText(text);
}

void PanelRow::enabledChanged()
{
    Clickable::enabledChanged();
    setOpacity(isEnabled() ? 1 : 0.45);
}

void PanelRow::layout()
{
    const double width = geometry().width;
    const double height = geometry().height;
    double x = 10;
    if (m_checkable) {
        m_check->setGeometry({x, rounding::centeredStart(height, 16), 16, 16});
        x += 16 + 8;
    }
    double right = width - 10;
    if (m_detail->isVisible()) {
        const SizeF detail = m_detail->sizeHint();
        const double w = std::ceil(detail.width);
        m_detail->setGeometry({right - w, views::layoutCenter(height, detail.height), w,
                               detail.height});
        right -= w + 8;
    }
    const SizeF label = m_label->sizeHint();
    m_label->setGeometry({x, views::layoutCenter(height, label.height), std::max(0.0, right - x),
                          label.height});
}

void PanelRow::paint(Painter& painter)
{
    const double alpha = m_pressed ? 0.14 : m_hovered ? 0.08 : 0;
    if (alpha > 0)
        painter.drawRoundedRect(localRect(), {Theme::current().mediumRadius, 0, Corners::Circular}, white(alpha));
}

Label* PanelColumn::addSection(const std::string& text, bool first)
{
    Label* label = views::addLabel(this, text, 8, white(0.55), 500);
    label->setElide(Label::Elide::Right);
    label->setName(first ? "firstSection" : "section");
    return label;
}

static double childHeight(const View* child, double width)
{
    if (const auto* label = dynamic_cast<const Label*>(child))
        return (label->name() == "firstSection" ? 0 : 8) + std::ceil(label->sizeHint().height);
    return std::ceil(child->heightForWidth(width));
}

SizeF PanelColumn::sizeHint() const
{
    return {0, heightForWidth(geometry().width)};
}

double PanelColumn::heightForWidth(double width) const
{
    double height = 0;
    for (const auto& child : children()) {
        if (child->isVisible())
            height += childHeight(child.get(), width);
    }
    return height;
}

void PanelColumn::layout()
{
    const double width = geometry().width;
    double y = 0;
    for (const auto& child : children()) {
        if (!child->isVisible())
            continue;
        const double height = childHeight(child.get(), width);
        if (auto* label = dynamic_cast<Label*>(child.get())) {
            const double top = label->name() == "firstSection" ? 0 : 8;
            label->setGeometry({10, y + top, std::max(0.0, width - 20), label->sizeHint().height});
        } else {
            child->setGeometry({0, y, width, height});
        }
        y += height;
    }
}

// Flow of the speed pills, margins 10 at the sides and 4 above and below.
class SpeedFlow : public View
{
public:
    double heightForWidth(double width) const override { return place(width, false); }
    SizeF sizeHint() const override { return {0, heightForWidth(geometry().width)}; }

protected:
    void layout() override { place(geometry().width, true); }

private:
    double place(double width, bool apply) const
    {
        const double left = 10;
        const double right = width - 10;
        double x = left;
        double y = 4;
        double rowHeight = 0;
        for (const auto& child : children()) {
            const SizeF size = child->sizeHint();
            const double w = std::ceil(size.width);
            if (x > left && x + w > right) {
                x = left;
                y += rowHeight + 6;
                rowHeight = 0;
            }
            if (apply)
                child->setGeometry({x, y, w, size.height});
            x += w + 6;
            rowHeight = std::max(rowHeight, size.height);
        }
        return y + rowHeight + 4;
    }
};

// The subtitle timing: earlier, later, and the delay.
class TimingRow : public View
{
public:
    explicit TimingRow(MainWindow& owner)
        : m_label(views::addLabel(this, tr("Timing"), 0, white()))
        , m_delay(views::addLabel(this, {}, 0, white(0.55)))
        , m_earlier(addChild<views::ControlButton>("minus", 30, 16))
        , m_later(addChild<views::ControlButton>("add", 30, 16))
    {
        views::setTabular(m_delay);
        m_earlier->setToolTipText(tr("Show Earlier"));
        m_later->setToolTipText(tr("Show Later"));
        m_earlier->onClicked = [&owner] { owner.shiftSubtitles(-0.1); };
        m_later->onClicked = [&owner] { owner.shiftSubtitles(0.1); };
    }

    void update(double delay, bool enabled)
    {
        m_delay->setText(MainWindow::delayText(delay));
        setEnabled(enabled);
        setOpacity(enabled ? 1 : 0.45);
        requestLayout();
    }

    SizeF sizeHint() const override { return {0, 34}; }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        m_later->setGeometry({width - 30, rounding::centeredStart(height, 30), 30, 30});
        m_earlier->setGeometry({width - 62, rounding::centeredStart(height, 30), 30, 30});
        const SizeF delay = m_delay->sizeHint();
        const double delayWidth = std::ceil(delay.width);
        const double delayRight = width - 62 - 2 - 4;
        m_delay->setGeometry({delayRight - delayWidth, views::layoutCenter(height, delay.height),
                              delayWidth, delay.height});
        const SizeF label = m_label->sizeHint();
        const double x = 10 + 24;
        m_label->setGeometry({x, views::layoutCenter(height, label.height),
                              std::max(0.0, delayRight - delayWidth - 4 - 2 - x), label.height});
    }

private:
    Label* m_label;
    Label* m_delay;
    views::ControlButton* m_earlier;
    views::ControlButton* m_later;
};

static const char* const kTabNames[] = {trNoop("Playlist"), trNoop("Tracks"), trNoop("Playback")};

// The segmented tabs.
class SidePanel::Tabs : public View
{
public:
    explicit Tabs(MainWindow& owner)
        : m_owner(owner)
    {
        for (const char* name : kTabNames) {
            auto* tab = addChild<views::Clickable>();
            const int index = int(m_tabs.size());
            tab->setName("panelTab" + std::to_string(index));
            tab->onClicked = [this, index] { m_owner.setPanelTab(index); };
            tab->onHoverChanged = [this] { requestPaint(); };
            m_tabs.push_back(tab);
            m_labels.push_back(views::addLabel(tab, tr(name), 0, white()));
        }
    }

    void retranslate()
    {
        for (size_t i = 0; i < m_labels.size(); ++i)
            m_labels[i]->setText(tr(kTabNames[i]));
        requestLayout();
    }

    void update(int current)
    {
        m_current = current;
        for (size_t i = 0; i < m_labels.size(); ++i)
            m_labels[i]->setWeight(int(i) == current ? 500 : 400);
        requestLayout();
        requestPaint();
    }

protected:
    void layout() override
    {
        const RectF inner = localRect().adjusted(2, 2, -2, -2);
        const double width = inner.width / double(m_tabs.size());
        for (size_t i = 0; i < m_tabs.size(); ++i) {
            m_tabs[i]->setGeometry({inner.x + width * double(i), inner.y, width, inner.height});
            const SizeF size = m_labels[i]->sizeHint();
            m_labels[i]->setGeometry({rounding::centeredStart(width, size.width), rounding::centeredStart(inner.height, size.height),
                                      size.width, size.height});
        }
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        painter.drawRoundedRect(localRect(), {theme.smallRadius, 0, Corners::Circular}, white(0.08));
        for (size_t i = 0; i < m_tabs.size(); ++i) {
            const double alpha = int(i) == m_current ? 0.2 : m_tabs[i]->isHoveredNow() ? 0.06 : 0;
            if (alpha > 0)
                painter.drawRoundedRect(m_tabs[i]->geometry(), {theme.smallRadius - 2, 0, Corners::Circular},
                                        white(alpha));
        }
    }

private:
    MainWindow& m_owner;
    std::vector<views::Clickable*> m_tabs;
    std::vector<Label*> m_labels;
    int m_current = 0;
};

// A playlist row: the play glyph or the number, the title, the length.
class SidePanel::PlaylistRow : public views::Clickable
{
public:
    explicit PlaylistRow(SidePanel& panel)
        : m_panel(panel)
        , m_play(addChild<Icon>(views::glyph("play"), 14))
        , m_number(views::addLabel(this, {}, 8, white(0.5)))
        , m_title(views::addLabel(this, {}, 0, white()))
        , m_duration(views::addLabel(this, {}, 0, white(0.55)))
    {
        m_play->setColor(white());
        m_title->setElide(Label::Elide::Middle);
        views::setTabular(m_number);
        views::setTabular(m_duration);
        m_duration->setName("playlistDuration");
        onHoverChanged = [this] { requestPaint(); };
    }

    void bind(int row)
    {
        m_row = row;
        Playlist& playlist = m_panel.m_owner.app().playlist();
        const bool current = row == playlist.currentIndex();
        m_current = current;
        m_play->setVisible(current);
        m_number->setVisible(!current);
        m_number->setText(std::to_string(row + 1));
        m_title->setText(playlist.title(row));
        const double duration = playlist.duration(row);
        m_duration->setVisible(duration > 0);
        m_duration->setText(::Application::formatTime(duration, duration));
        requestLayout();
        requestPaint();
    }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double height = geometry().height;
        m_play->setGeometry({10 + rounding::centeredStart(18, 14), rounding::centeredStart(height, 14), 14, 14});
        const SizeF number = m_number->sizeHint();
        m_number->setGeometry({10 + rounding::centeredStart(18, number.width), rounding::centeredStart(height, number.height),
                               number.width, number.height});
        double right = width - 10;
        if (m_duration->isVisible()) {
            const SizeF duration = m_duration->sizeHint();
            const double w = std::ceil(duration.width);
            m_duration->setGeometry({right - w, views::layoutCenter(height, duration.height), w,
                                     duration.height});
            right -= w + 8;
        }
        const SizeF title = m_title->sizeHint();
        const double x = 10 + 18 + 8;
        m_title->setGeometry({x, views::layoutCenter(height, title.height),
                              std::max(0.0, right - x), title.height});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        Color color{0, 0, 0, 0};
        if (m_current)
            color = theme.highlight.withAlpha(0.55);
        else if (m_hovered || (m_row == m_panel.m_currentRow && m_panel.hasFocus()))
            color = white(0.08);
        if (color.a > 0)
            painter.drawRoundedRect(localRect(), {theme.mediumRadius, 0, Corners::Circular}, color);
    }

    void pointerPress(const PointerEvent& event) override
    {
        m_pressY = mapToWindow(event.position).y;
        m_panel.rowPressed(m_row);
        if (event.button == MouseButton::Right) {
            m_panel.m_owner.openPlaylistMenu(m_row, mapToWindow(event.position));
            return;
        }
        if (event.button != MouseButton::Left)
            return;
        m_pressed = true;
        if (event.clickCount == 2)
            m_panel.m_owner.playIndex(m_row);
    }

    void pointerMove(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        const double y = mapToWindow(event.position).y;
        if (!m_panel.m_dragging && std::abs(y - m_pressY) < 6)
            return;
        m_panel.setDragging(true);
        m_panel.rowDragged(m_row, y);
    }

    void pointerRelease(const PointerEvent&) override
    {
        m_pressed = false;
        m_panel.setDragging(false);
    }

private:
    SidePanel& m_panel;
    Icon* m_play;
    Label* m_number;
    Label* m_title;
    Label* m_duration;
    int m_row = -1;
    bool m_current = false;
    double m_pressY = 0;
};

SidePanel::SidePanel(MainWindow& owner)
    : m_owner(owner)
    , m_tabs(addChild<Tabs>(owner))
    , m_playlistPage(addChild<View>())
    , m_list(m_playlistPage->addChild<ListView>())
    , m_emptyLabel(views::addLabel(m_playlistPage, {}, 0, white(0.6)))
    , m_repeat(m_playlistPage->addChild<views::ControlButton>("repeat"))
    , m_add(m_playlistPage->addChild<views::ControlButton>("add"))
    , m_tracksView(addChild<ScrollView>())
    , m_tracks(m_tracksView->setContent<PanelColumn>())
    , m_playbackView(addChild<ScrollView>())
    , m_playback(m_playbackView->setContent<PanelColumn>())
{
    setName("sidePanel");
    m_list->setName("playlistView");
    m_repeat->setName("repeatButton");
    m_add->setName("addFilesButton");
    m_list->setRowHeight(36);
    m_list->setClipsChildren(true);
    m_tracksView->setClipsChildren(true);
    m_playbackView->setClipsChildren(true);
    m_tracksView->setScrollDirections(false, true);
    m_playbackView->setScrollDirections(false, true);
    m_list->setScrollDirections(false, true);
    for (ScrollView* view : {static_cast<ScrollView*>(m_list), m_tracksView, m_playbackView})
        views::darkenBars(view);
    m_list->createItem = [this] { return std::make_unique<PlaylistRow>(*this); };
    m_list->bindItem = [](View* item, size_t row) { static_cast<PlaylistRow*>(item)->bind(int(row)); };
    m_emptyLabel->setWrap(true);
    m_emptyLabel->setTextAlignment(Label::TextAlignment::Center);
    m_emptyLabel->setVerticallyCentered(true);
    m_repeat->onClicked = [this] {
        Playlist& playlist = m_owner.app().playlist();
        playlist.setRepeatMode(Playlist::RepeatMode((int(playlist.repeatMode()) + 1) % 3));
    };
    m_add->onClicked = [this] { m_owner.openFiles(true); };
    retranslate();
}

// The rows are made again in place, so the lists keep their scroll
// positions and the playlist its current row.
void SidePanel::retranslate()
{
    m_tabs->retranslate();
    m_emptyLabel->setText(tr("Drop files here to add them."));
    m_add->setToolTipText(tr("Add Files…"));
    m_tracksKey.clear();
    m_playbackKey.clear();
    refresh();
}

void SidePanel::refresh()
{
    m_tabs->update(m_owner.panelTab());
    m_playlistPage->setVisible(m_owner.panelTab() == 0);
    m_tracksView->setVisible(m_owner.panelTab() == 1);
    m_playbackView->setVisible(m_owner.panelTab() == 2);
    refreshPlaylist();
    refreshTracks();
    refreshPlayback();
}

void SidePanel::refreshPlaylist()
{
    Playlist& playlist = m_owner.app().playlist();
    const auto mode = playlist.repeatMode();
    m_repeat->setIcon(mode == Playlist::RepeatMode::One ? "repeat-one" : "repeat");
    m_repeat->setChecked(mode != Playlist::RepeatMode::Off);
    m_repeat->setToolTipText(mode == Playlist::RepeatMode::Off   ? tr("Repeat Off")
                             : mode == Playlist::RepeatMode::One ? tr("Repeat One")
                                                                 : tr("Repeat All"));
    if (m_list->rowCount() != size_t(playlist.count()))
        m_list->setRowCount(size_t(playlist.count()));
    m_list->reload();
    m_emptyLabel->setVisible(playlist.count() == 0);
    if (m_currentRow >= playlist.count())
        m_currentRow = playlist.count() - 1;
}

void SidePanel::refreshTracks()
{
    const Player& player = m_owner.player();
    // Rebuilt only when the tracks change; the checks follow every time.
    std::string key = std::to_string(player.subtitleTrack()) + "/" + std::to_string(player.audioTrack()) + "/";
    for (const auto& track : player.subtitleTracks())
        key += "s" + std::to_string(track.id) + track.label;
    for (const auto& track : player.audioTracks())
        key += "a" + std::to_string(track.id) + track.label;
    key += std::to_string(player.subtitleDelay());
    if (key == m_tracksKey)
        return;
    m_tracksKey = key;
    m_tracks->clear();

    m_tracks->addSection(tr("Subtitles"), true);
    // TRANSLATORS: no subtitles shown.
    auto* off = m_tracks->addRow<PanelRow>(tr("Off"), true);
    off->setChecked(player.subtitleTrack() == 0);
    off->onClicked = [this] { m_owner.selectSubtitle(0); };
    for (const auto& track : player.subtitleTracks()) {
        auto* row = m_tracks->addRow<PanelRow>(track.label, true);
        row->setChecked(player.subtitleTrack() == track.id);
        const int id = track.id;
        row->onClicked = [this, id] { m_owner.selectSubtitle(id); };
    }
    auto* addFile = m_tracks->addRow<PanelRow>(tr("Add Subtitle File…"), true);
    addFile->onClicked = [this] { m_owner.openSubtitleFile(); };
    auto* timing = m_tracks->addRow<TimingRow>(m_owner);
    timing->update(player.subtitleDelay(), player.subtitleTrack() > 0);

    m_tracks->addSection(tr("Audio"));
    if (player.audioTracks().empty()) {
        auto* none = m_tracks->addRow<PanelRow>(tr("No Audio Tracks"), true);
        none->setEnabled(false);
    }
    for (const auto& track : player.audioTracks()) {
        auto* row = m_tracks->addRow<PanelRow>(track.label, true);
        row->setChecked(player.audioTrack() == track.id);
        const int id = track.id;
        row->onClicked = [this, id] { m_owner.selectAudio(id); };
    }
    m_tracksView->requestLayout();
}

void SidePanel::refreshPlayback()
{
    const Player& player = m_owner.player();
    std::string key = std::to_string(player.speed()) + "/" + std::to_string(player.loopFile()) + "/"
            + std::to_string(player.hasVideo()) + "/" + std::to_string(player.chapter()) + "/"
            + std::to_string(player.duration());
    for (const auto& chapter : player.chapters())
        key += chapter.title + std::to_string(chapter.time);
    if (key == m_playbackKey)
        return;
    m_playbackKey = key;
    m_playback->clear();

    m_playback->addSection(tr("Speed"), true);
    auto* speeds = m_playback->addRow<SpeedFlow>();
    for (const double speed : MainWindow::kSpeeds) {
        auto* pill = speeds->addChild<views::PillButton>(MainWindow::speedText(speed),
                                                          std::abs(player.speed() - speed) < 0.001);
        pill->onClicked = [this, speed] { m_owner.setSpeed(speed); };
    }

    m_playback->addSection(tr("Options"));
    auto* repeat = m_playback->addRow<PanelRow>(tr("Repeat"), true);
    repeat->setChecked(player.loopFile());
    repeat->onClicked = [this] { m_owner.player().setLoopFile(!m_owner.player().loopFile()); };
    auto* left = m_playback->addRow<PanelRow>(tr("Rotate Left"), true);
    left->setEnabled(player.hasVideo());
    left->onClicked = [this] { m_owner.player().setRotation(m_owner.player().rotation() - 90); };
    auto* right = m_playback->addRow<PanelRow>(tr("Rotate Right"), true);
    right->setEnabled(player.hasVideo());
    right->onClicked = [this] { m_owner.player().setRotation(m_owner.player().rotation() + 90); };

    if (!player.chapters().empty()) {
        m_playback->addSection(tr("Chapters"));
        int index = 0;
        for (const auto& chapter : player.chapters()) {
            const std::string title = chapter.title.empty()
                    ? format(tr("Chapter {number}"), {{"number", std::to_string(index + 1)}})
                    : chapter.title;
            auto* row = m_playback->addRow<PanelRow>(title, true,
                                                     ::Application::formatTime(chapter.time, player.duration()));
            row->setChecked(player.chapter() == index);
            row->onClicked = [this, index] { m_owner.player().setChapter(index); };
            ++index;
        }
    }
    m_playbackView->requestLayout();
}

void SidePanel::focusList()
{
    if (m_currentRow < 0 && m_owner.app().playlist().count() > 0)
        setCurrentRow(0);
    setFocus(FocusReason::Keyboard);
}

void SidePanel::setCurrentRow(int row)
{
    m_currentRow = row;
    if (row >= 0)
        m_list->scrollToRow(size_t(row));
    m_list->reload();
}

void SidePanel::rowPressed(int row)
{
    setCurrentRow(row);
    setFocus(FocusReason::Mouse);
}

void SidePanel::rowDragged(int row, double windowY)
{
    const PointF point = m_list->mapFromWindow({10, windowY});
    const double offset = point.y + m_list->scrollPosition().y;
    const int target = int(std::floor(offset / 36));
    Playlist& playlist = m_owner.app().playlist();
    if (target >= 0 && target < playlist.count() && target != row) {
        playlist.move(row, target);
        m_currentRow = target;
    }
}

void SidePanel::setDragging(bool dragging)
{
    if (m_dragging == dragging)
        return;
    m_dragging = dragging;
    if (onBusyChanged)
        onBusyChanged();
}

void SidePanel::focusIn(FocusReason)
{
    m_list->reload();
    if (onBusyChanged)
        onBusyChanged();
}

void SidePanel::focusOut()
{
    m_list->reload();
    if (onBusyChanged)
        onBusyChanged();
}

bool SidePanel::canDrop(const DropOffer& offer) const
{
    return offer.hasMimeType("text/uri-list");
}

uint32_t SidePanel::dragUpdate(const DragEvent&)
{
    return CopyAction;
}

uint32_t SidePanel::drop(const DragEvent& event)
{
    event.offer->read("text/uri-list", [weak = m_owner.aliveToken(), this](std::string data) {
        if (!weak.expired())
            m_owner.app().playlist().append(urls::fromUriList(data));
    });
    return CopyAction;
}

bool SidePanel::keyPress(const KeyEvent& event)
{
    Playlist& playlist = m_owner.app().playlist();
    if (event.modifiers != NoModifier)
        return false;
    switch (event.key) {
    case key::Up:
        if (m_currentRow > 0)
            setCurrentRow(m_currentRow - 1);
        return true;
    case key::Down:
        if (m_currentRow + 1 < playlist.count())
            setCurrentRow(m_currentRow + 1);
        return true;
    case key::Delete:
        if (m_currentRow >= 0)
            playlist.remove(m_currentRow);
        return true;
    case key::Return:
    case key::KeypadEnter:
        if (m_currentRow >= 0)
            m_owner.playIndex(m_currentRow);
        return true;
    case key::Escape:
        m_owner.togglePanel(0, false);
        return true;
    default:
        return false;
    }
}

void SidePanel::layout()
{
    const RectF content = localRect().adjusted(8, 8, -8, -8);
    m_tabs->setGeometry({content.x, content.y, content.width, 30});
    const RectF page{content.x, content.y + 30 + 6, content.width, std::max(0.0, content.height - 36)};
    m_playlistPage->setGeometry(page);
    m_tracksView->setGeometry(page);
    m_playbackView->setGeometry(page);
    // The playlist: the list, then the footer row 4 below it.
    const double footer = 36;
    const double listHeight = std::max(0.0, page.height - footer - 4);
    m_list->setGeometry({0, 0, page.width, listHeight});
    m_emptyLabel->setGeometry({0, 0, page.width, listHeight});
    m_repeat->setGeometry({0, listHeight + 4, 36, 36});
    m_add->setGeometry({page.width - 36, listHeight + 4, 36, 36});
}

void SidePanel::paint(Painter& painter)
{
    views::paintPlate(painter, localRect(), 20);
}
