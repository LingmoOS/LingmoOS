// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <cstdint>
#include <string>

#include "shell/launcher/launcher_metrics.h"
#include "shell/launcher/launcher_model.h"
#include "shell/launcher/launcher_tween.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Label;
class ThemeIcon;
class TextField;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class CountBadge;
class LauncherView;

// One application in the launcher's grid: its icon and its count,
// its name in up to two lines, the new mark, the keyboard selection and the
// pressed silhouette. Only the icon takes the pointer; the rest of the cell
// is the page behind it.
class LauncherCell final : public ui::View
{
public:
    LauncherCell(LauncherView *view, std::string id);
    ~LauncherCell() override;

    const std::string &id() const { return m_id; }
    bool isFolder() const { return m_isFolder; }

    void setItem(const LauncherItem &item);
    // The application's count on the icon's corner; 0 for none.
    void setCount(int64_t count);
    CountBadge *countBadge() const { return m_countBadge; }
    const std::string &iconName() const { return m_iconName; }
    void setMetrics(const LauncherMetrics &metrics);
    // The page the cell is on and its index there.
    void setSlot(int page, int index) { m_page = page; m_index = index; }
    int page() const { return m_page; }
    int index() const { return m_index; }

    void setSelected(bool selected);
    bool isSelected() const { return m_selected; }
    // Its icon and name hide while it is dragged.
    void setDragging(bool dragging);
    void setMenuOpen(bool open);
    void beginRename();
    void setDragHover(bool active);
    void setFolderPreview(bool active, const std::string &incomingIconName = {});
    void animateFolderCreated();
    bool isPressDimmed() const { return (m_pressed || m_menuOpen) && !m_dragging; }
    // The colour the selection is inked against.
    void setBackdrop(ui::Color color);

    // Its place in the page; animated, as the grid's displaced transitions,
    // when `animate`.
    void moveTo(const ui::PointF &position, bool animate);
    bool isMoving() const { return m_moveX.isRunning() || m_moveY.isRunning(); }
    void finishMove();

    // In the cell's coordinates.
    ui::RectF iconRect() const;
    // The selection plate, in the cell's coordinates; empty while hidden.
    ui::RectF selectionRect() const;
    // The new mark, in the cell's coordinates; empty while hidden.
    ui::RectF newMarkRect() const;

    // Called by the icon; not part of the public API.
    void iconPressed(const ui::PointerEvent &event);
    void iconMoved(const ui::PointerEvent &event);
    void iconReleased(const ui::PointerEvent &event);

protected:
    bool acceptsPointer() const override { return true; }
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerMove(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    class Icon;

    void updateMask();
    void updateCountBadge();
    void finishRename();
    ui::RectF folderMiniRect(const ui::RectF &icon, int index) const;

    LauncherView *m_view;
    std::string m_id;
    std::string m_iconName;
    bool m_isFolder = false;
    bool m_newInstalled = false;
    LauncherMetrics m_metrics;
    int m_page = 0;
    int m_index = 0;
    Icon *m_icon = nullptr;
    ui::ThemeIcon *m_mask = nullptr;
    ui::ThemeIcon *m_dropIcon = nullptr;
    std::vector<ui::ThemeIcon *> m_folderIcons;
    size_t m_folderIconCount = 0;
    // Above the icon and its pressed look.
    CountBadge *m_countBadge = nullptr;
    ui::Label *m_label = nullptr;
    ui::TextField *m_renameField = nullptr;
    ui::Color m_backdrop {0, 0, 0, 255};
    LauncherTween m_hoverTween;
    double m_hoverProgress = 0;
    bool m_selected = false;
    bool m_dragging = false;
    bool m_menuOpen = false;
    bool m_pressed = false;
    bool m_backgroundPress = false;
    // One drag per press.
    bool m_dragArmed = false;
    ui::PointF m_pressPoint;
    LauncherSpring m_moveX;
    LauncherSpring m_moveY;
    LauncherTween m_dropTween;
    double m_dropProgress = 0;
    bool m_folderPreview = false;
    LauncherTween m_folderCreateTween;
};

} // namespace Lingmo::Shell

