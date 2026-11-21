// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/geometry.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/view.h"

#include <optional>
#include <string>

// FishUI's point sizes in logical pixels.
inline double points(double size)
{
    return size * 4 / 3;
}

// The colour laid over the base, as Qt.tint does.
Lingmo::ui::Color tint(Lingmo::ui::Color base, Lingmo::ui::Color overlay);

Lingmo::ui::Label* addLabel(Lingmo::ui::View* parent, std::string text, double pointSize = 0, int weight = 400,
                              std::optional<Lingmo::ui::Color> color = std::nullopt);

// A rounded plate in a colour, its corners the theme's unless told otherwise.
class Plate : public Lingmo::ui::View
{
public:
    explicit Plate(double radius = 0);

    void setRadius(double radius);
    // Unset: the theme's second background.
    void setColor(std::optional<Lingmo::ui::Color> color);
    void setContinuousCorners(std::optional<bool> continuous);

protected:
    void paint(Lingmo::ui::Painter& painter) override;

    Lingmo::ui::Corners corners() const;

private:
    double m_radius;
    std::optional<Lingmo::ui::Color> m_color;
    std::optional<bool> m_continuous;
};

// One device pixel at any scale, across the bottom of its rect.
void paintSeparator(Lingmo::ui::Painter& painter, double x, double bottom, double width);

// One read-only row of a ListCard: the label, then its value in secondary text.
class InfoRow : public Lingmo::ui::View
{
public:
    InfoRow(std::string text = {}, std::string value = {});

    void setText(std::string text);
    void setValue(std::string value);
    // Drawn below every visible row but the card's last.
    void setSeparatorVisible(bool visible);

    Lingmo::ui::SizeF sizeHint() const override { return {0, 44}; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;

private:
    Lingmo::ui::RowLayout* m_row;
    Lingmo::ui::Label* m_text;
    Lingmo::ui::Label* m_value;
    bool m_separator = false;
};

// A card of InfoRows: no inner padding, the rows carry their own.
class ListCard : public Plate
{
public:
    ListCard();

    InfoRow* addRow(std::string text, std::string value = {});
    // After rows were shown or hidden.
    void updateSeparators();

    Lingmo::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    void layout() override;
};

// Names the card below it; padded to the card's text edge.
class SectionLabel : public Lingmo::ui::View
{
public:
    explicit SectionLabel(std::string text = {});

    void setText(std::string text);
    void setElide(Lingmo::ui::Label::Elide elide);

    Lingmo::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    void layout() override;

private:
    Lingmo::ui::Label* m_label;
};
