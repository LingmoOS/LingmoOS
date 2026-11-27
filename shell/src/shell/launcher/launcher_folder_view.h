// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "shell/launcher/launcher_metrics.h"
#include "shell/launcher/launcher_tween.h"
#include "ui/scroller.h"
#include "ui/view.h"

namespace Lingmo::ui {
class TextField;
} // namespace Lingmo::ui

namespace Lingmo::Shell {

class ApplicationBadges;
class LauncherCell;
class LauncherModel;
class LauncherPageIndicator;
class LauncherView;
class LauncherWallpaper;

// A full-screen overlay that shows a folder's contents in a panel that
// opens/closes with a scale+fade animation from the folder cell's position.
// Supports paged scrolling via swipe, mouse wheel and indicator tap.
// Ported from examples/cutefish-compositor LaunchpadFolderView.
class LauncherFolderView final : public ui::View, private ui::Scroller::Target
{
public:
    static constexpr int kMaximumColumns = 4;
    static constexpr std::chrono::milliseconds kOpenDuration{280};
    static constexpr std::chrono::milliseconds kCloseDuration{220};

    LauncherFolderView(LauncherView *view, LauncherModel &model,
                       ApplicationBadges &badges, LauncherWallpaper &wallpaper);
    ~LauncherFolderView() override;

    void setMetrics(const LauncherMetrics &metrics);

    // `from` is in the view's coordinates; empty → grow in place.
    void open(const std::string &folderId, bool animate, ui::RectF from = {});
    void close();
    // No longer counts as open while it shrinks; keeps cells until close().
    void dismiss();
    // Keeps the cells until close(): a drag out of the panel started on one.
    void detach();

    bool isOpen() const { return !m_folder.empty() && !m_detached && !m_closing; }
    bool isDetached() const { return m_detached; }
    const std::string &folder() const { return m_folder; }

    void sync();
    void editName();
    ui::TextField *nameField() const;

    LauncherCell *cell(const std::string &id) const;
    // In the view's coordinates.
    ui::RectF panelRect() const;
    ui::RectF cellRect(int index) const;
    // -1 outside the folder's applications.
    int indexAt(ui::PointF position) const;
    // -1 or 1 over the empty edge, 0 elsewhere.
    int edgeAt(ui::PointF position) const;

    int pageTotal() const;
    int currentPage() const { return m_currentPage; }
    void goToPage(int page);
    // Dot centres in view coordinates.
    std::vector<ui::PointF> indicatorDots() const;

protected:
    bool acceptsPointer() const override { return true; }
    void layout() override;
    void paint(ui::Painter &painter) override;
    void pointerPress(const ui::PointerEvent &event) override;
    bool scroll(const ui::ScrollEvent &event) override;

private:
    class NameField;
    class Panel;
    friend class NameField;
    friend class Panel;

    // Scroller::Target
    ui::Scroller::Axis horizontalAxis() const override;
    ui::Scroller::Axis verticalAxis() const override;
    ui::PointF position() const override;
    void applyPosition(ui::PointF position) override;
    void movingChanged(bool moving) override;

    int columns() const;
    int pageSize() const;
    int visibleRows() const;
    double padding() const;
    double nameGap() const;
    double indicatorGap() const;
    double indicatorHeight() const;
    int nameFontSize() const;
    double nameHeight() const;
    double titleHeight() const;
    ui::RectF gridRect() const;
    void setProgress(double progress);
    ui::PointF slotPosition(int index) const;
    void placeCells();
    void setContentX(double x);
    int nearestPage() const;
    void setCurrentPage(int page, bool slide);
    void resetPages();
    void handleWheel(const ui::ScrollEvent &event);
    void panelPressed(ui::PointF position);
    void panelMoved(ui::PointF position);
    void panelReleased(ui::PointF position);
    void commitName();
    void retire(std::unique_ptr<ui::View> view);

    LauncherView *m_view;
    LauncherModel &m_model;
    ApplicationBadges &m_badges;
    LauncherWallpaper &m_wallpaper;
    LauncherMetrics m_metrics;
    std::string m_folder;
    bool m_detached = false;
    bool m_closing = false;
    int m_closeSerial = 0;
    ui::RectF m_from;
    double m_progress = 1;
    Panel *m_panel = nullptr;
    NameField *m_name = nullptr;
    ui::View *m_grid = nullptr;
    LauncherPageIndicator *m_indicator = nullptr;
    std::map<std::string, LauncherCell *> m_cells;
    std::vector<std::string> m_order;
    LauncherTween m_fade;

    int m_currentPage = 0;
    double m_contentX = 0;
    bool m_swiping = false;
    LauncherTween m_slide;
    double m_wheelAccumulated = 0;
    base::Timer m_wheelReset;
    bool m_panelPressed = false;
    bool m_flicking = false;
    ui::PointF m_pressPosition;
    double m_pressContentX = 0;
    std::vector<std::pair<std::chrono::steady_clock::time_point, double>> m_flickSamples;
    std::unique_ptr<ui::Scroller> m_scroller;
    std::vector<std::unique_ptr<ui::View>> m_retired;
    base::Subscription m_wallpaperChanged;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace Lingmo::Shell
