// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "model/pathbarmodel.h"

#include "base/signal.h"
#include "base/timer.h"
#include "ui/view.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class Icon;
class Label;
class Menu;
class TextField;
class Window;
}

namespace fm {

class CompletionPopup;
class FolderModel;

// A folder's crumbs. When they do not fit, the
// names between the root and the current folder shorten first, then fold
// into a "…" crumb, then the root does too; the current folder is elided
// only once nothing is left to fold.
class Breadcrumbs : public ui::View
{
public:
    explicit Breadcrumbs(bool openActions);
    ~Breadcrumbs() override;

    void setCrumbs(std::vector<Crumb> crumbs);
    const std::vector<Crumb>& crumbs() const { return m_crumbs; }

    std::function<void(const std::string& path)> onItemClicked;
    std::function<void(const std::string& path)> onOpenInNewTab;
    std::function<void(const std::string& path)> onOpenInNewWindow;
    // A press outside every crumb.
    std::function<void()> onEmptyClicked;
    // A press moved past the drag distance.
    std::function<void()> onDragStarted;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;
    void pointerMove(const ui::PointerEvent& event) override;
    void pointerRelease(const ui::PointerEvent& event) override;
    void hoverMove(const ui::PointerEvent& event) override;
    void hoverChanged() override;

private:
    struct CrumbView;

    void relayout();
    // The crumb under a point; the fold is -2, nothing -1.
    int crumbAt(ui::PointF point) const;
    double highlightHeight() const;

    bool m_openActions;
    std::vector<Crumb> m_crumbs;
    std::vector<CrumbView> m_views;
    ui::Label* m_foldLabel;
    ui::Icon* m_foldChevron;
    int m_foldFirst = -1;
    int m_foldLast = -1;
    double m_foldX = 0;
    double m_currentTextWidth = 0;
    double m_middleTextWidth = 0;
    int m_pressed = -1;
    bool m_dragged = false;
    ui::PointF m_pressPosition;
    ui::MouseButton m_pressedButton = ui::MouseButton::Left;
    bool m_foldMenuOpen = false;
    std::unique_ptr<ui::Menu> m_menu;
};

// The breadcrumbs, or the editor in their place, which goes to
// a typed path and searches for anything else.
class PathBar : public ui::View
{
public:
    PathBar();
    ~PathBar() override;

    void setFolderModel(FolderModel* model);
    void updateUrl(const std::string& url);
    // The editor's placeholder; the crumbs follow the next updateUrl().
    void retranslate();
    void openEditor();
    void openSearch();
    void closeEditor();
    void endSearch();
    bool isEditing() const;

    std::function<void(const std::string& path)> onItemClicked;
    std::function<void(const std::string& path)> onEditorAccepted;
    std::function<void(const std::string& path)> onOpenInNewTab;
    std::function<void(const std::string& path)> onOpenInNewWindow;
    // The editor gave the keys back to the folder view.
    std::function<void()> onFinished;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter& painter) override;
    void pointerPress(const ui::PointerEvent& event) override;
    void pointerMove(const ui::PointerEvent& event) override;
    void pointerRelease(const ui::PointerEvent& event) override;

private:
    friend class PathEditor;

    bool searching() const;
    static bool isPath(const std::string& text);
    void updateVisibility();
    void editorFocusChanged(bool focused);
    bool editorKey(const ui::KeyEvent& event);

    FolderModel* m_model = nullptr;
    std::string m_url;
    bool m_editing = false;
    Breadcrumbs* m_crumbs;
    ui::TextField* m_editor;
    ui::Icon* m_clear;
    base::Timer m_debounce;
    std::unique_ptr<CompletionPopup> m_completion;
    bool m_dragged = false;
    ui::PointF m_press;
    base::Subscription m_searchChanged;
    base::Subscription m_mountsChanged;
};

} // namespace fm
