// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <utility>

#include "ui/geometry.h"

namespace Lingmo::ui {

class View;
class Window;

// A view dropped down from a control in a popup window of its own, as a
// combo box drops its list: on the menu's popup glass (an opaque fill
// without the compositor's blur), gap pixels under the control, or above it when there is no room
// below. Escape, a press elsewhere or close() put it away; the content has
// the keyboard focus when it takes it and sees Escape first. The content
// stays with the popup between openings, so it keeps its state.
class DropDownPopup
{
public:
    DropDownPopup();
    ~DropDownPopup();

    DropDownPopup(const DropDownPopup&) = delete;
    DropDownPopup& operator=(const DropDownPopup&) = delete;

    // Fills the popup, which takes its preferred size: the size hint's width
    // and the height for that width, rounded up. It paints over the bubble
    // and keeps its own margins.
    void setContentView(std::unique_ptr<View> view);
    template<typename T, typename... Args>
    T* setContent(Args&&... args)
    {
        auto view = std::make_unique<T>(std::forward<Args>(args)...);
        T* content = view.get();
        setContentView(std::move(view));
        return content;
    }
    View* contentView() const { return m_content; }

    // gap pixels under the view, or under a rect of it in its coordinates,
    // from its left edge.
    void popupFor(View* view, double gap = 3, std::optional<RectF> rect = {});
    void close();
    bool isOpen() const { return m_window != nullptr; }
    // The popup while open.
    Window* window() const { return m_window.get(); }
    // Measures the content again and resizes the open popup to it.
    void updateSize();

    std::function<void()> onClosed;

private:
    class Surface;

    SizeF contentSize() const;
    void applySize(SizeF size);

    // The content while closed; while open it lives in the surface.
    std::unique_ptr<View> m_ownedContent;
    View* m_content = nullptr;
    std::unique_ptr<Window> m_window;
    Surface* m_surface = nullptr;
};

} // namespace Lingmo::ui
