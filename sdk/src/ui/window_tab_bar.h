// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ui/color_mixer.h"
#include "ui/view.h"

namespace Lingmo::ui {

class ColorProvider;

// The document tab bar of the terminal, the text editor and the file manager:
// tabs on a pill-shaped track, the current one raised on it; only it or the
// hovered tab shows a close button. Tabs are dragged to reorder them and
// dragged off the bar to detach them; a crowded bar scrolls (wheel or
// touchpad), its strip clipped to the pill. The application owns the documents:
// the bar reports what the user asked for and the application changes the tabs.
class WindowTabBar : public View
{
public:
    WindowTabBar();
    ~WindowTabBar() override;

    int count() const;
    void insertTab(int index, std::string title);
    void removeTab(int index);
    // Moves a tab, as after onMoveRequested; the tabs glide while a drag
    // settles and jump otherwise.
    void moveTab(int from, int to);
    void setTabTitle(int index, std::string title);
    const std::string& tabTitle(int index) const;
    // The editor's dot in the close button's place: shown while the tab is
    // modified and its close button is not.
    void setTabModified(int index, bool modified);

    int currentIndex() const { return m_current; }
    // Scrolls the tab into view; does not call onCurrentChanged.
    void setCurrentIndex(int index);

    // The colours follow these: the bar may sit on a colour scheme rather
    // than the theme.
    void setDarkMode(std::optional<bool> dark);
    void setTextColor(std::optional<ColorRecipe> color);
    // The terminal and the editor: 140 to 220; the file manager: at least
    // 140 and no maximum (0).
    void setTabWidthRange(double minimum, double maximum);
    // The track inside the bar, as the file manager's 6, 4, 40, 4.
    void setTrackInsets(const Margins& insets);
    // A pressed tab darkens (the file manager's text at 0.10).
    void setPressedFeedback(bool pressed);
    // Off, a lone tab stays put when dragged along the bar, as the editor's
    // did; pulled off the bar it still calls onDetachRequested.
    void setLoneTabDraggable(bool draggable);
    // A tab lit with the hover colour while a file drag waits over it; -1
    // for none.
    void setSpringLoadIndex(int index);

    // The track's width now, in the bar: an editor's "+" sits after it.
    double trackWidth() const;
    // A tab's rect in the bar's coordinates, as drawn now.
    RectF tabRect(int index) const;
    // The tab under a point of the bar, or -1.
    int indexAtPosition(PointF position) const;

    // A title editor over the tab, as the terminal's Rename Tab…; Return
    // calls onRenameRequested, Escape or focus loss gives up.
    void startRename(int index);

    // A tab dragged over the bar from another window, drawn at `x` (bar
    // coordinates) with the others making room where it would drop.
    void beginIncoming(std::string title, double x);
    void setIncomingX(double x);
    // Dropped, the next inserted tab glides from where the ghost was.
    void endIncoming(bool dropped);
    // Where an incoming tab would drop, or -1.
    int incomingIndex() const;

    // A tab pulled off the bar stays hidden until it is removed, or comes
    // back with this when the detach did not happen.
    void cancelDetach();

    SizeF sizeHint() const override { return {0, 30}; }

    // A tab clicked.
    std::function<void(int index)> onCurrentChanged;
    // Its close button, or a middle click.
    std::function<void(int index)> onCloseRequested;
    std::function<void(int from, int to)> onMoveRequested;
    // Dragged 16 px above or below the bar; `position` is the pointer in the
    // window. The press goes on, so a drag can be started from it.
    std::function<void(int index, PointF position)> onDetachRequested;
    std::function<void(int index, const std::string& title)> onRenameRequested;
    // A right click on a tab, the pointer in the window: the application
    // opens its menu.
    std::function<void(int index, PointF position)> onContextMenuRequested;

protected:
    void layout() override;
    void paint(Painter& painter) override;

private:
    class Strip;
    class Tab;
    class CloseButton;
    class Ticker;
    struct Tween
    {
        double from = 0;
        double to = 0;
        uint64_t startNs = 0;
        int durationMs = 0;
        bool active = false;

        double value(uint64_t now) const;
        void start(double from, double to, int durationMs, uint64_t now);
    };

    // The bar's own colour mode's colours.
    const ColorProvider& colors() const;
    Color textColor() const;
    ColorRecipe textRecipe() const;
    Color trackColor() const;
    Color selectedColor() const;
    Color hoverColor() const;
    Color separatorColor() const;
    Color dimTextColor() const;

    bool sliding() const;
    int slotCount() const;
    double tabWidth() const;
    double contentWidth() const;
    double slotFor(int index) const;
    void restartSettle();
    void ensureVisible(int index);
    void setScroll(double x);
    void updateDrag(double x);
    void beginDrag(int index, double pressX);
    void endDrag(bool moved);
    void detach(int index, PointF windowPosition);
    void updateSlots();
    void placeTabs();
    void scheduleTicks();
    void tick(uint64_t now);
    uint64_t now() const;

    Strip* m_strip = nullptr;
    std::vector<Tab*> m_tabs;
    std::unique_ptr<Ticker> m_ticker;
    int m_current = 0;
    std::optional<bool> m_darkMode;
    std::optional<ColorRecipe> m_textColor;
    double m_minimumTabWidth = 140;
    double m_maximumTabWidth = 220;
    Margins m_trackInsets;
    bool m_pressedFeedback = false;
    bool m_loneTabDraggable = true;
    int m_springLoad = -1;

    double m_scrollX = 0;
    Tween m_slotWidth;
    Tween m_trackWidth;
    uint64_t m_settleUntil = 0;
    uint64_t m_lastTickNs = 0;
    double m_laidOutWidth = -1;

    int m_dragIndex = -1;
    int m_dropIndex = -1;
    double m_dragX = 0;
    double m_pointerX = 0;
    double m_grabOffset = 0;
    Tween m_dragLag;
    int m_leaving = -1;

    bool m_incoming = false;
    std::string m_incomingTitle;
    double m_incomingX = 0;
    double m_arrivalSlot = -1;
    View* m_ghost = nullptr;

    int m_editing = -1;
};

} // namespace Lingmo::ui
