// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <chrono>
#include <memory>
#include <optional>

#include "ui/view.h"

namespace Lingmo::ui {

class Animation;

// A progress bar: a pill track filled in the accent colour. An
// indeterminate bar sends a third of its length across, back and forth.
class ProgressBar : public View
{
public:
    ProgressBar();
    ~ProgressBar() override;

    // From 0 to 1.
    void setValue(double value);
    double value() const { return m_value; }
    void setIndeterminate(bool indeterminate);

    SizeF sizeHint() const override { return {200, 6}; }

protected:
    void paint(Painter& painter) override;

private:
    double m_value = 0;
    bool m_indeterminate = false;
    double m_phase = 0;
    std::unique_ptr<Animation> m_animation;
};

// A busy indicator: the refresh glyph turning once a second.
class BusyIndicator : public View
{
public:
    BusyIndicator();
    ~BusyIndicator() override;

    void setRunning(bool running);
    bool isRunning() const { return m_running; }
    // One turn; 1000 ms unless set.
    void setDuration(std::chrono::milliseconds duration);
    // Unset follows the theme's text colour.
    void setColor(std::optional<Color> color);
    // The current turn, in degrees clockwise.
    double angle() const { return m_angle; }

    SizeF sizeHint() const override { return {22, 22}; }

protected:
    void paint(Painter& painter) override;

private:
    bool m_running = true;
    double m_angle = 0;
    std::chrono::milliseconds m_duration{1000};
    std::optional<Color> m_color;
    std::unique_ptr<Animation> m_animation;
};

} // namespace Lingmo::ui
