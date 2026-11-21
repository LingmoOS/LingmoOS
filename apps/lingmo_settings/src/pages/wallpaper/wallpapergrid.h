// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ui/view.h"

namespace Lingmo::ui {
class Icon;
class Label;
class RowLayout;
}

namespace settings {

class WallpaperCloseButton;
class WallpaperLinkButton;
class WallpaperSwatch;
class WallpaperTile;

// A titled section of wallpapers: one row of thumbnails, or all of them once
// shown in full. Thumbnails stay 160 wide while they fit; a column is added
// once every column can be at least 128 wide.
class WallpaperGrid : public Lingmo::ui::View
{
public:
    // icon: a glyph before the title, for sections that stand for something
    // other than the wallpapers themselves.
    explicit WallpaperGrid(std::string title, std::string icon = {});
    ~WallpaperGrid() override;

    void setWallpapers(std::vector<std::string> wallpapers);
    // Rings the current wallpaper while the background is a picture.
    void setCurrent(const std::string& path, bool imageSelected);
    // A tile before the wallpapers that adds one.
    void setShowAddWallpaper(bool show);
    // A remove button over the hovered thumbnail.
    void setRemovable(bool removable);
    // A close button after the hovered title.
    void setClosable(bool closable);

    Lingmo::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

    std::function<void(const std::string& path)> onSelected;
    // At the point of the window where the add tile was clicked.
    std::function<void(Lingmo::ui::PointF position)> onAddRequested;
    std::function<void(const std::string& path)> onRemoveRequested;
    std::function<void()> onCloseRequested;

protected:
    void layout() override;

private:
    struct Geometry
    {
        int columns = 1;
        double thumbnailWidth = 0;
        double thumbnailHeight = 0;
        double cellWidth = 0;
        double cellHeight = 0;
        int rows = 0;
        double height = 0;
    };

    Geometry geometryFor(double width) const;
    int itemCount() const;
    void rebuildTiles();
    void updateTiles();
    void updateHeader();

    std::vector<std::string> m_wallpapers;
    std::string m_current;
    bool m_imageSelected = true;
    bool m_showAddWallpaper = false;
    bool m_removable = false;
    bool m_closable = false;
    bool m_expanded = false;

    Lingmo::ui::RowLayout* m_header;
    Lingmo::ui::View* m_title;
    Lingmo::ui::Icon* m_icon;
    Lingmo::ui::Label* m_label;
    Lingmo::ui::Label* m_count;
    WallpaperCloseButton* m_close;
    WallpaperLinkButton* m_showAll;
    Lingmo::ui::View* m_tiles;
    std::vector<WallpaperTile*> m_tileViews;
};

// The plain colours a background can be.
class ColorSwatches : public Lingmo::ui::View
{
public:
    ColorSwatches();

    void setCurrent(const std::string& color, bool colorSelected);

    Lingmo::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

    std::function<void(const std::string& color)> onSelected;

protected:
    void layout() override;

private:
    std::vector<WallpaperSwatch*> m_swatches;
};

} // namespace settings
