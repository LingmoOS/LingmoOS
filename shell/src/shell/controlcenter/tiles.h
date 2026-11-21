// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "base/timer.h"
#include "shell/controlcenter/control_center_services.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Icon;
class ImageView;
class Label;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

// An SVG file drawn at its rect's device size on whole device pixels, as
// FishUI's IconItem draws a file; tinted when given a colour.
class SvgIcon : public ui::View
{
public:
    void setFile(std::string file);
    void setTint(std::optional<ui::Color> tint);

protected:
    void paint(ui::Painter &painter) override;

private:
    std::string m_file;
    std::optional<ui::Color> m_tint;
};

// A toggle tile: a round plate with the glyph, a
// title and a status line. The whole card clicks; holding it 800 ms does the
// press-and-hold action instead. No hover state, no animation.
class CardItem : public ui::View
{
public:
    static constexpr double kPlateSize = 28;
    static constexpr double kIconSize = 16;
    static constexpr double kHeight = kPlateSize + 12 * 2 + 4;

    CardItem();
    ~CardItem() override;

    // `name`'s light/ artwork, dark/ when dark or checked.
    void setArtwork(std::string name);
    // A FishUI.Icon glyph in place of artwork.
    void setGlyph(std::string name);
    void setChecked(bool checked);
    bool isChecked() const { return m_checked; }
    void setTitle(std::string title);
    void setStatus(std::string status);

    std::function<void()> onClicked;
    std::function<void()> onPressAndHold;

    ui::SizeF sizeHint() const override { return {0, kHeight}; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;

private:
    void updateIcon();

    std::string m_artwork;
    bool m_checked = false;
    bool m_pressed = false;
    bool m_held = false;
    SvgIcon *m_svg;
    ui::Icon *m_glyph;
    ui::Label *m_title;
    ui::Label *m_status;
    base::Timer m_holdTimer;
};

// An icon at three quarters of the button, a
// round ink plate under it while hovered and enabled.
class IconButton : public ui::View
{
public:
    IconButton();

    void setFile(std::string file);
    // What the button sits on, for the ink of its plate.
    void setSurfaceColor(ui::Color color);

    std::function<void()> onClicked;

    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;
    void hoverChanged() override;
    void enabledChanged() override;
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerMove(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;

private:
    SvgIcon *m_icon;
    ui::Color m_surface;
    bool m_pressed = false;
    bool m_inside = false;
};

// A small "More / Less" row that expands or collapses the secondary cards.
// Height is fixed at kHeight; the chevron is hand-drawn in paint().
class ExpandButton : public ui::View
{
public:
    static constexpr double kHeight = 28;

    explicit ExpandButton();

    void setExpanded(bool expanded);
    bool expanded() const { return m_expanded; }
    void setLabel(std::string more, std::string less);

    std::function<void()> onClicked;

    ui::SizeF sizeHint() const override { return {0, kHeight}; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;
    void pointerEnter() override;
    void pointerLeave() override;
    void pointerRelease(const ui::PointerEvent &event) override;

private:
    ui::Label *m_label   = nullptr;
    bool m_expanded = false;
    bool m_hovered  = false;
};

// The media card, 60 tall. Always shown; without a
// playing player it reads "Not Playing" with the default cover and three
// dimmed, disabled buttons.
class MediaTile : public ui::View
{
public:
    static constexpr double kHeight = 60;

    MediaTile();

    void setState(const MediaState &state);
    IconButton *previousButton() const { return m_previous; }
    IconButton *playButton() const { return m_play; }
    IconButton *nextButton() const { return m_next; }

    std::function<void()> onPrevious;
    std::function<void()> onPlay;
    std::function<void()> onPause;
    std::function<void()> onNext;

    ui::SizeF sizeHint() const override { return {0, kHeight}; }

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    void updateArtwork();

    MediaState m_state;
    SvgIcon *m_cover;
    ui::ImageView *m_art;
    ui::Label *m_title;
    ui::Label *m_artist;
    IconButton *m_previous;
    IconButton *m_play;
    IconButton *m_next;
    bool m_artReady = false;
};

// The applications behind the bar's privacy dot, one sentence each, in a
// tile above the cards.
class DeviceUsersTile : public ui::View
{
public:
    static constexpr double kIconSize = 20;

    DeviceUsersTile();

    void setUsers(const std::vector<DeviceUser> &users);
    const std::vector<DeviceUser> &users() const { return m_users; }
    // Takes the sentences again after the session language changed.
    void retranslate();

    double heightForWidth(double width) const override;
    ui::SizeF sizeHint() const override { return {0, heightForWidth(0)}; }

protected:
    void layout() override;
    void paint(ui::Painter &painter) override;

private:
    struct Row {
        ui::View *icon;
        ui::Label *name;
        ui::Label *usage;
        ui::Color dot;
    };

    double rowHeight() const;
    double inset() const;

    std::vector<DeviceUser> m_users;
    std::vector<Row> m_rows;
};

// DeviceUsers' sentence for what an application uses, after its name.
std::string deviceUsageText(const DeviceUser &user);

} // namespace Lingmo::Shell

