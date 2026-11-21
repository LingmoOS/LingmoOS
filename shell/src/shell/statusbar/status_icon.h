// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "shell/statusbar/bar_item.h"
#include "shell/statusbar/status_bar_panels.h"
#include "ui/animation.h"

namespace Lingmo::ui {
class RowLayout;
}

namespace Lingmo::Shell {

class Battery;

// A bar icon that opens a panel, with an optional dot after
// it and an optional label, 12 px of the item on either side of them.
class StatusIcon : public BarItem
{
public:
    explicit StatusIcon(BarContext *bar);

    // The bar's own artwork, statusbar/<dark|light>/<name>.svg.
    void setIconName(std::string name);
    // A FishUI glyph in the bar's colour, in place of the artwork.
    void setGlyph(std::string name);
    void setIconWidth(double width);
    void setLabel(std::string label);
    // The dot after the icon; none when unset.
    void setIndicatorColor(std::optional<ui::Color> color);

    // The battery's level, and its percentage with `percentage`; hidden
    // without a battery.
    void showBattery(const Battery &battery, bool percentage);
    // The Wi-Fi state; hidden without Wi-Fi hardware.
    void showNetwork(const StatusBarIndicators::Network &network);

    const std::string &iconName() const { return m_iconName; }
    const std::string &glyph() const { return m_glyph; }

    // 12 + content + 12, as the bar's row lays it out.
    double preferredWidth() const;
    // The content's width changed.
    std::function<void()> onPreferredWidthChanged;
    // The bar's look changed.
    void updateLook();

protected:
    void layout() override;

private:
    class Indicator;

    void updateIcon();
    void updateContent();

    bool m_localLight = false;
    std::string m_iconName;
    std::string m_glyph;
    std::string m_label;
    double m_iconWidth = BarContext::kIconSize;
    std::optional<ui::Color> m_indicatorColor;
    ui::RowLayout *m_content = nullptr;
    BarIcon *m_icon = nullptr;
    Indicator *m_indicator = nullptr;
    BarLabel *m_text = nullptr;
};

// The clock: its text centred over its plate, the plate reaching 9 px from
// the screen's edge.
class ClockItem : public BarItem
{
public:
    explicit ClockItem(BarContext *bar);

    void setText(const std::string &text);
    void setDate(const std::string &text);
    void setRecording(bool recording);
    bool isRecording() const { return m_recording; }
    double preferredWidth() const;
    void updateLook();
    std::function<void()> onPreferredWidthChanged;

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    BarLabel *m_label;
    BarLabel *m_date;
    std::unique_ptr<ui::Animation> m_recordingAnimation;
    double m_recordingPulse = 0;
    bool m_recording = false;
};

// The stop button of an agent's control: a single-colour glyph in the bar's
// colour.
class StopItem : public BarItem
{
public:
    explicit StopItem(BarContext *bar);

    double preferredWidth() const { return BarContext::kItemGap + BarContext::kIconSize + 2 * BarContext::kSmallSpacing; }
    void updateLook();

protected:
    void layout() override;

private:
    BarIcon *m_icon;
};

} // namespace Lingmo::Shell

