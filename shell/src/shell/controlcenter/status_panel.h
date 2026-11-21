// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/timer.h"
#include "shell/controlcenter/control_center_services.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Animation;
class BusyIndicator;
class Icon;
class Label;
class ScrollView;
class Switch;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class LevelSlider;
class SvgIcon;

// The status panels' metrics.
namespace status_panel {
inline constexpr double kWidth = 320;
inline constexpr double kContentPadding = 12;
inline constexpr double kHorizontalPadding = 16;
inline constexpr double kRowInset = 6;
// Where rows are laid out from the panel's side; their text sits kRowInset
// further in.
inline constexpr double kRowMargin = kHorizontalPadding - kRowInset;
inline constexpr double kBottomPadding = kContentPadding - kRowInset;
// A row background's gap to the panel's edges.
inline constexpr double kBackgroundMargin = kBottomPadding;
inline constexpr double kWindowRadius = 16;
inline constexpr double kBackgroundRadius = kWindowRadius - kBackgroundMargin;
inline constexpr double kHeaderHeight = 28;
inline constexpr double kSectionSpacing = 8;
inline constexpr double kSeparatorSpacing = 12;
// The height animation: FrameTween's.
inline constexpr std::chrono::milliseconds kHeightDuration {380};
} // namespace status_panel

// A list row of the status panels: an ItemDelegate with a round badge, a
// label and trailing marks, its background reaching kBackgroundMargin from
// the panel's sides, inked while hovered and pressed. Its geometry is that
// background, which takes hover and clicks.
class PanelRow : public ui::View
{
public:
    PanelRow();
    ~PanelRow() override;

    // A FishUI.Icon glyph on the badge.
    void setGlyph(std::string name);
    // An SVG file on the badge, tinted as an IconItem with a colour.
    void setArtwork(std::string file);
    void setText(std::string text);
    // A second line (the hotspot's "Hotspot"), 3 below the first.
    void setDetail(std::string detail);
    void setActive(bool active);
    bool isActive() const { return m_active; }
    void setBold(bool bold);
    void setBusy(bool busy);
    void setLocked(bool locked);
    void setRowEnabled(bool enabled);

    std::function<void()> onClicked;

    double heightForWidth(double width) const override;
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;
    void hoverChanged() override;
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;

private:
    double contentHeight() const;

    ui::Icon *m_glyph;
    SvgIcon *m_artwork;
    ui::Label *m_label;
    ui::Label *m_detail;
    ui::BusyIndicator *m_busy;
    ui::Icon *m_lock;
    bool m_active = false;
    bool m_pressed = false;
    bool m_rowEnabled = true;
};

// A status panel without its window: the header (title,
// detail, accessory), the body a panel fills, and the separator and settings
// link on the bottom edge. The panel is drawn shownBottom() tall, which
// follows contentHeight() with FrameTween's 380 ms curve while shown, and
// its window with it.
class StatusPanelView : public ui::View
{
public:
    StatusPanelView(std::string title, std::string settingsText, std::string settingsPage,
                    ControlCenterServices &services);
    ~StatusPanelView() override;

    // The work area's height less the margins.
    void setMaximumHeight(double height);
    // Whole logical pixels, as a window's height.
    double contentHeight() const;
    double shownBottom() const { return m_shownBottom; }
    // While hidden, a change of height jumps; shown, it animates.
    void setShown(bool shown);
    bool isShown() const { return m_shown; }

    // Reads the services again.
    virtual void refresh() = 0;
    virtual void opened() {}
    virtual void closed() {}
    // Takes its text again after the session language changed.
    virtual void retranslate() = 0;

    ui::View *settingsLink() const;

    // shownBottom() changed: the window's glass and input follow it.
    std::function<void()> onShownBottomChanged;
    // The panel asked to close.
    std::function<void()> onCloseRequested;

protected:
    void setTitles(std::string title, std::string settingsText);
    void setTitleDetail(std::string detail);
    // A view of the panel's, centred on the title's capitals at the
    // header's trailing end.
    void setAccessory(ui::View *accessory);
    void setBodyVisible(bool visible);
    // What the body needs at the body column's width.
    virtual double bodyHeight() const = 0;
    // Places the body's views below `top`, in body()'s coordinates, whose x
    // is the panel's; the body column runs from kHorizontalPadding over
    // kWidth - 2 kHorizontalPadding.
    virtual void layoutBody(double top) = 0;
    // The body's height changed: animates to the new height.
    void contentChanged();
    // What the body may take before the panel reaches the work area's height.
    double bodyMaximumHeight() const;

    ControlCenterServices &services() const { return m_services; }
    // Where a panel's header accessory and body views go: clipped from the
    // content padding down to the footer.
    ui::View *header() const { return m_clip; }
    ui::View *body() const { return m_body; }

    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    class SettingsLink;

    double headerHeight() const;
    double footerHeight() const;
    double layoutHeight() const;
    double footerTop() const;
    void animateTo(double target);
    void setShownBottom(double bottom);

    ControlCenterServices &m_services;
    ui::View *m_clip;
    ui::View *m_body;
    ui::Label *m_title;
    ui::Label *m_detail;
    ui::View *m_accessory = nullptr;
    SettingsLink *m_link;
    std::string m_settingsPage;
    double m_maximumHeight = 0;
    double m_shownBottom = 0;
    double m_tweenFrom = 0;
    double m_tweenTarget = 0;
    // The largest of the last move's ends, while the body stays drawn.
    double m_reach = 0;
    bool m_bodyVisible = true;
    bool m_shown = false;
    std::unique_ptr<ui::Animation> m_tween;
};

// The Wi-Fi panel: the Wi-Fi switch, the hotspot and the networks in range.
class NetworkPanel : public StatusPanelView
{
public:
    explicit NetworkPanel(ControlCenterServices &services);
    ~NetworkPanel() override;

    void refresh() override;
    void opened() override;
    void closed() override;
    void retranslate() override;
    void setShownForScans(bool shown);

    ui::Switch *wirelessSwitch() const { return m_switch; }
    const std::vector<PanelRow *> &rows() const { return m_rows; }

protected:
    double bodyHeight() const override;
    void layoutBody(double top) override;

private:
    class List;

    double listHeight() const;

    ui::Switch *m_switch;
    ui::ScrollView *m_scroll;
    List *m_list;
    PanelRow *m_hotspot;
    std::vector<PanelRow *> m_rows;
    std::vector<WirelessNetwork> m_networks;
    base::Timer m_scanTimer;
};

class BluetoothPanel : public StatusPanelView
{
public:
    explicit BluetoothPanel(ControlCenterServices &services);
    ~BluetoothPanel() override;

    void refresh() override;
    void opened() override;
    void closed() override;
    void retranslate() override;
    const std::vector<PanelRow *> &rows() const { return m_rows; }

protected:
    double bodyHeight() const override;
    void layoutBody(double top) override;

private:
    class List;
    double listHeight() const;

    ui::Switch *m_switch;
    ui::ScrollView *m_scroll;
    List *m_list;
    std::vector<PanelRow *> m_rows;
    std::vector<BluetoothDevice> m_devices;
};

// The sound panel: the default sink's level and the outputs.
class VolumePanel : public StatusPanelView
{
public:
    explicit VolumePanel(ControlCenterServices &services);
    ~VolumePanel() override;

    void refresh() override;
    void retranslate() override;

    const std::vector<PanelRow *> &rows() const { return m_rows; }

protected:
    double bodyHeight() const override;
    void layoutBody(double top) override;

private:
    ui::Label *m_noOutput;
    LevelSlider *m_slider;
    ui::Label *m_heading;
    std::vector<PanelRow *> m_rows;
    std::vector<AudioOutput> m_outputs;
};

// The battery panel: the power source, the battery's state and the power modes.
class BatteryPanel : public StatusPanelView
{
public:
    explicit BatteryPanel(ControlCenterServices &services);
    ~BatteryPanel() override;

    void refresh() override;
    void retranslate() override;

    const std::vector<PanelRow *> &rows() const { return m_rows; }

protected:
    double bodyHeight() const override;
    void layoutBody(double top) override;

private:
    ui::Label *m_source;
    ui::Label *m_state;
    ui::View *m_separator;
    ui::Label *m_heading;
    // power-saver, balanced, performance.
    std::vector<PanelRow *> m_rows;
    std::vector<std::string> m_profiles;
};

} // namespace Lingmo::Shell

