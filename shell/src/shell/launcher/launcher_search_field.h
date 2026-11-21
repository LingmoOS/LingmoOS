// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <string>

#include "shell/launcher/launcher_metrics.h"
#include "shell/launcher/launcher_tween.h"
#include "ui/text_field.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Icon;
class Label;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

// The launcher's search pill: a translucent white
// field whose placeholder, a search glyph and "Search", sits centred until
// the field takes the focus and then slides to the start, where the typed
// text begins. Input methods compose in it through the compositor's relay.
class LauncherSearchField final : public ui::View
{
public:
    LauncherSearchField();
    ~LauncherSearchField() override;

    void setMetrics(const LauncherMetrics &metrics);
    // Animations play only while the launcher is up, so the first layout
    // and a reset while hidden do not animate.
    void setAnimated(bool animated) { m_animated = animated; }

    const std::string &text() const;
    void setText(std::string text);
    bool hasFieldFocus() const;
    void focusField();
    // Takes the placeholder's text again after the session language changed.
    void retranslate();

    // The text changed, from the user, an input method or setText().
    std::function<void()> onTextChanged;
    // Escape, Up, Down, Return and Enter in the field; true when taken.
    std::function<bool(uint32_t key)> onKey;

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    class Field;

    void focusChanged(bool focused);
    void updatePlaceholder();
    void layoutPlaceholder();

    LauncherMetrics m_metrics;
    Field *m_field = nullptr;
    ui::View *m_placeholder = nullptr;
    ui::Icon *m_glyph = nullptr;
    ui::Label *m_label = nullptr;
    bool m_animated = false;
    bool m_focused = false;
    double m_focusProgress = 0;
    double m_backgroundOpacity = 0.18;
    double m_backgroundScale = 0.985;
    double m_glyphScale = 1;
    LauncherTween m_focusTween;
    LauncherTween m_opacityTween;
    LauncherTween m_scaleTween;
    LauncherTween m_glyphTween;
};

} // namespace Lingmo::Shell

