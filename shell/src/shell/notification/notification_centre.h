// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <optional>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"
#include "shell/notification/notification.h"
#include "shell/notification/notification_motion.h"
#include "ui/view.h"
#include "ui/window.h"

namespace Lingmo::ui {
class Label;
class ScrollView;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class ClearButton;
class PillButton;
class NotificationBanners;
class NotificationCard;
class NotificationHistory;
class Shell;
class ShellWindow;

// The notification centre: the history in a
// 380 px popup of the status bar on PanelGlass, from 6 below the work
// area's top to 6 above its bottom, under the item that opened it or 6 from
// the output's right edge. Each application's notifications stack behind
// its newest; a click fans the stack out under a header with "Show Less"
// and a clear button, or shows a card's whole body. Cards fade out and the
// gap closes before they leave the history. While it is open, new
// notifications go to the history instead of banners.
class NotificationCentre
{
public:
    static constexpr int kWidth = 380;
    static constexpr int kMargin = 6;
    static constexpr double kRadius = 18;
    static constexpr double kInset = 12;
    static constexpr double kHeaderHeight = 32;
    static constexpr double kPeekStep = 8;
    static constexpr double kGroupHeaderHeight = 26;
    static constexpr std::chrono::milliseconds kDismissFade {200};
    static constexpr std::chrono::milliseconds kRemovalDelay {650};

    class Row;
    class GroupHeader;

    NotificationCentre(Shell *shell, NotificationHistory &history, NotificationBanners &banners);
    ~NotificationCentre();

    NotificationCentre(const NotificationCentre &) = delete;
    NotificationCentre &operator=(const NotificationCentre &) = delete;

    // Opens as a popup of `parent`, the status bar on its output, with its
    // left edge at `position.x` (global logical: the opening item's left
    // inset), moved in to 6 px from the
    // output's right edge where it would not fit.
    void open(ShellWindow *parent, const base::Point &position);
    void close();
    // Opens, or closes when open.
    void toggle(ShellWindow *parent, const base::Point &position);
    bool isOpen() const { return m_open; }
    // Opened or closed: the status bar's clock item is checked while open.
    base::Signal<> openChanged;

    // Where it is, global logical, while open.
    base::Rect geometry() const { return m_geometry; }
    ShellWindow *shellWindow() const;
    ClearButton *clearButton() const { return m_clear; }
    // The rows as the history holds them.
    // Views of the popup, there once it was first opened.
    const std::vector<Row *> &rows() const { return m_rows; }
    const std::vector<GroupHeader *> &groupHeaders() const { return m_headers; }
    // Rows that take part, those fading out included.
    int liveCount() const { return m_liveCount; }
    bool isExpanded(const std::string &groupKey) const { return m_expanded.count(groupKey) > 0; }
    // An expand, collapse or dismissal is playing out.
    bool isInMotion() const { return m_motion; }

    // The layout pass, for tests: rows and headers move to their targets.
    void relayout();

private:
    class Content;
    class List;

    struct Placement {
        ShellWindow *parent = nullptr;
        base::Rect geometry;
    };

    Placement placementFor(ShellWindow *parent, const base::Point &position) const;
    void build(const Placement &placement);
    void destroyWindow();
    void applyGlass();
    void onHidden();
    void retranslate();

    // The model's rows.
    void insertRows(int first, int count);
    void removeRows(int first, int count);
    void moveRow(int from, int to);
    void updateRows(int first, int last);
    void resetRows();
    Row *makeRow(int index);

    void scheduleLayout();
    void layoutPass();
    GroupHeader *headerFor(const std::string &key);
    void pruneHeaders();
    void setExpanded(const std::string &key, bool expanded);
    void beginMotion(std::chrono::milliseconds duration = std::chrono::milliseconds(410));
    void dismissRows(const std::function<bool(const Row &)> &predicate);
    void dismissGroup(const std::string &key);
    void toggleDetail(Row *row);
    void commitRemovals();
    void disarmClears(std::optional<ui::PointF> windowPoint);

    Shell *m_shell;
    NotificationHistory &m_history;
    NotificationBanners &m_banners;

    std::unique_ptr<ui::Window> m_window;
    ShellWindow *m_parent = nullptr;
    base::Rect m_geometry;
    base::Rect m_placedGeometry;
    bool m_open = false;
    Content *m_content = nullptr;
    ui::Label *m_title = nullptr;
    ClearButton *m_clear = nullptr;
    ui::ScrollView *m_scroll = nullptr;
    List *m_list = nullptr;
    ui::Label *m_empty = nullptr;
    ui::View *m_widgets = nullptr;

    std::vector<Row *> m_rows;
    std::vector<GroupHeader *> m_headers;
    std::set<std::string> m_expanded;
    int m_liveCount = 0;
    bool m_motion = false;
    bool m_inLayout = false;
    bool m_layoutDirty = false;
    bool m_layoutScheduled = false;

    base::Timer m_motionTimer;
    base::Timer m_removalTimer;
    base::Timer m_timeTimer;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    std::vector<base::Subscription> m_subscriptions;
};

// One history row. The centre's layout pass sets
// where it goes and what part it plays in its application's group.
class NotificationCentre::Row : public ui::View
{
public:
    Row(NotificationCentre *centre, ui::Window *window);
    ~Row() override;

    void setNotification(const Notification &notification, const std::string &timeText);
    const Notification &notification() const { return m_notification; }
    const std::string &groupKey() const { return m_groupKey; }
    NotificationCard *card() const { return m_card; }

    // Set by the layout pass.
    bool placed = false;
    // The first row of its application still standing: it carries the stack.
    bool lead = true;
    bool groupExpanded = false;
    // Rows stacked behind a collapsed lead.
    int stackCount = 0;
    // Fading out in place; `gone` follows once it has, and only then does
    // the row give up its space.
    bool dismissing = false;
    bool gone = false;

    bool isStacked() const { return lead && !groupExpanded && stackCount > 0; }
    bool isShown() const { return !dismissing && (lead || groupExpanded); }
    bool isOccupying() const { return (lead || groupExpanded) && (!dismissing || !gone); }
    int peekCount() const;
    double contentHeight() const;
    double targetHeight() const;
    double targetY() const { return m_y.target(); }
    // As drawn now.
    double contentOpacity() const { return m_opacity.value(); }
    double frameHeight() const { return m_frameHeight.value(); }
    double peekOpacity(int index) const { return m_peeks[index]->value(); }

    void setDetailed(bool detailed);
    bool isDetailed() const;
    void moveTo(double y, bool animate);
    // Whether any of the card shows between `top` and `bottom` of the list,
    // as drawn now or once it is at `y` as its state says.
    bool isSeenWithin(double top, double bottom, double y) const;
    // Brings what the row shows in line with its state.
    void apply(bool animated);
    void startGoneTimer();
    void disarmUnless(ui::PointF windowPoint);
    // Puts the row and its content where the tweens are now.
    void place();

private:
    class Content;

    void placeContent();

    NotificationCentre *m_centre;
    Notification m_notification;
    std::string m_groupKey;
    Content *m_content = nullptr;
    ui::View *m_frame = nullptr;
    NotificationCard *m_card = nullptr;
    Tween m_y;
    Tween m_contentY;
    Tween m_opacity;
    Tween m_scale;
    Tween m_frameHeight;
    std::unique_ptr<Tween> m_peeks[2];
    bool m_bottomOrigin = true;
    base::Timer m_goneTimer;
};

// The title above an expanded application group, with
// "Show Less" and a clear button; it belongs to the group, not to a row.
class NotificationCentre::GroupHeader : public ui::View
{
public:
    GroupHeader(NotificationCentre *centre, ui::Window *window, std::string groupKey);
    ~GroupHeader() override;

    const std::string &groupKey() const { return m_groupKey; }
    void setAppName(const std::string &name);
    void retranslate();
    bool placed = false;
    bool isShown() const { return m_shown; }
    void setShown(bool shown);
    double targetHeight() const;
    void moveTo(double y, bool animate);
    void disarmUnless(ui::PointF windowPoint);
    PillButton *showLessButton() const { return m_showLess; }

protected:
    void layout() override;

private:
    void place();

    NotificationCentre *m_centre;
    std::string m_groupKey;
    bool m_shown = false;
    ui::Label *m_label = nullptr;
    PillButton *m_showLess = nullptr;
    ClearButton *m_clear = nullptr;
    Tween m_y;
    Tween m_opacity;
};

} // namespace Lingmo::Shell

