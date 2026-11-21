// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "widgets.h"

#include <string>
#include <vector>

class MainWindow;

namespace Lingmo::ui {
class ListView;
class ScrollView;
}

// An optional check mark, the text, an optional detail.
class PanelRow : public views::Clickable
{
public:
    PanelRow(std::string text, bool checkable, std::string detail = {});

    void setChecked(bool checked);
    void setText(const std::string& text);
    Lingmo::ui::SizeF sizeHint() const override { return {0, 34}; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;
    void enabledChanged() override;

private:
    Lingmo::ui::Icon* m_check;
    Lingmo::ui::Label* m_label;
    Lingmo::ui::Label* m_detail;
    bool m_checkable;
};

// A column of side panel sections and rows, in a scroll view.
class PanelColumn : public Lingmo::ui::View
{
public:
    // A section heading; the first section has no top margin.
    Lingmo::ui::Label* addSection(const std::string& text, bool first = false);
    template<typename T, typename... Args>
    T* addRow(Args&&... args)
    {
        return addChild<T>(std::forward<Args>(args)...);
    }
    void clear() { while (!children().empty()) takeChild(children().back().get()); }

    Lingmo::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    void layout() override;
};

// The playlist, the tracks and playback settings.
class SidePanel : public Lingmo::ui::View
{
public:
    explicit SidePanel(MainWindow& owner);

    void refresh();
    void retranslate();
    void refreshPlaylist();
    void refreshTracks();
    void refreshPlayback();
    void focusList();
    bool isBusy() const { return m_dragging || hasFocus(); }
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    std::function<void()> onBusyChanged;

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;
    bool scroll(const Lingmo::ui::ScrollEvent&) override { return true; }
    bool keyPress(const Lingmo::ui::KeyEvent& event) override;
    // Dropped files join the playlist.
    bool canDrop(const Lingmo::ui::DropOffer& offer) const override;
    uint32_t dragUpdate(const Lingmo::ui::DragEvent& event) override;
    uint32_t drop(const Lingmo::ui::DragEvent& event) override;
    void focusIn(Lingmo::ui::FocusReason) override;
    void focusOut() override;

private:
    class Tabs;
    class PlaylistRow;
    friend class PlaylistRow;

    void setCurrentRow(int row);
    void rowPressed(int row);
    void rowDragged(int row, double windowY);
    void setDragging(bool dragging);

    MainWindow& m_owner;
    Tabs* m_tabs;
    Lingmo::ui::View* m_playlistPage;
    Lingmo::ui::ListView* m_list;
    Lingmo::ui::Label* m_emptyLabel;
    views::ControlButton* m_repeat;
    views::ControlButton* m_add;
    Lingmo::ui::ScrollView* m_tracksView;
    PanelColumn* m_tracks;
    Lingmo::ui::ScrollView* m_playbackView;
    PanelColumn* m_playback;
    int m_currentRow = -1;
    bool m_dragging = false;
    // What the track and chapter rows were built for.
    std::string m_tracksKey;
    std::string m_playbackKey;
};
