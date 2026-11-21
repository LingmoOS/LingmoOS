// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <string>

#include "base/geometry.h"
#include "render/glass.h"
#include "ui/geometry.h"
#include "ui/path.h"

namespace Lingmo::ui {
class Animation;
class Label;
class View;
class Window;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class DockWindow;
class Shell;

// The dock's tooltip: a glass bubble whose arrow points at the icon, a popup
// of its own size that the next icon moves rather than makes again.
class DockTooltip
{
public:
    DockTooltip(DockWindow *dock, Shell *shell);
    ~DockTooltip();

    // `target`: the icon's centre in the dock window's coordinates.
    void show(const std::string &text, const base::PointF &target);
    void hide();
    bool isShown() const;
    const std::string &text() const { return m_text; }
    ui::Window *window() const { return m_window.get(); }
    // The fade-in's progress, 1 once the bubble is whole; for tests.
    double fade() const { return m_progress; }

private:
    // Paints the fade over the content and the glass.
    void applyFade();

    DockWindow *m_dock;
    Shell *m_shell;
    std::unique_ptr<ui::Window> m_window;
    ui::View *m_content = nullptr;
    ui::Label *m_label = nullptr;
    std::unique_ptr<ui::Animation> m_fade;
    double m_progress = 1;
    // What the bubble is made of, kept to fade it in: the glass body's region,
    // radius and shape, as the last show() left them.
    render::GlassParams m_glass;
    base::Rect m_glassRegion;
    int m_glassRadius = 0;
    ui::Path m_glassShape;
    // Measures the text at the dock window's scale, as the label is drawn:
    // hinted text is wider at 1.5 than at 1.
    ui::Label *m_measure = nullptr;
    std::string m_text;
};

} // namespace Lingmo::Shell

