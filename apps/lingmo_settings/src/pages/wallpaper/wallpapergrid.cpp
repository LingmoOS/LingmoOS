// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/wallpaper/wallpapergrid.h"

#include <algorithm>
#include <cctype>
#include <cmath>

#include "base/i18n.h"
#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/image_view.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/colors.h"
#include "widgets/metrics.h"

using namespace Lingmo::ui;
using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace settings {

static constexpr double kHeaderHeight = 24;
static constexpr double kPreferredThumbnailWidth = 160;
static constexpr double kMinimumThumbnailWidth = 128;
// Between a thumbnail's box and its picture, where the selection ring goes.
static constexpr double kThumbnailInset = 4;
static constexpr double kCloseButtonSize = 20;
// FishUI.Units.smallSpacing, a Button's top and bottom padding.
static constexpr double kButtonPadding = 6;
static constexpr double kSwatchSize = 32;
static constexpr double kSwatchCell = 50;
static const char* const kSwatchColors[] = {"#2B8ADA", "#4DA4ED", "#B7E786", "#F2BB73",
                                            "#EE72EB", "#F0905A", "#595959", "#000000"};

static Color parseColor(const std::string& text)
{
    if (text.size() != 7 || text[0] != '#')
        return Color::rgb(0);
    return Color::rgb(uint32_t(std::stoul(text.substr(1), nullptr, 16)));
}

static std::string lowerCase(std::string text)
{
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return char(std::tolower(c)); });
    return text;
}

// A cross in a 20 px circle: after a hovered section title, where it closes
// the section, or over a hovered thumbnail, where it removes the wallpaper.
class WallpaperCloseButton : public View
{
public:
    enum class Style {
        Section,
        Thumbnail,
    };

    explicit WallpaperCloseButton(Style style)
        : m_style(style)
    {
    }

    // Shown while the pointer is over what it belongs to, or over itself.
    void setOwnerHovered(bool hovered)
    {
        m_ownerHovered = hovered;
        update();
    }

    bool isHovered() const { return m_hovered; }

    SizeF sizeHint() const override { return {kCloseButtonSize, kCloseButtonSize}; }
    bool acceptsPointer() const override { return isShown(); }

    std::function<void()> onClicked;
    // The pointer came onto it or left it.
    std::function<void()> onHoverChanged;

protected:
    void paint(Painter& painter) override
    {
        if (!isShown())
            return;
        const Theme& theme = Theme::current();
        const RectF bounds = localRect();
        Color fill;
        Color cross;
        if (m_style == Style::Section) {
            fill = theme.text.withAlpha(m_pressed ? 0.2 : m_hovered ? 0.1 : 0);
            cross = theme.text;
        } else {
            fill = Color::rgb(0).withAlpha(m_pressed ? 0.7 : 0.55);
            cross = Color::rgb(0xFFFFFF);
        }
        if (fill.a > 0)
            painter.drawSmoothRectangle(bounds, bounds.width / 2, fill);
        // Two 10 by 1.5 bars turned about their centres, which anchors put
        // half a bar's thickness above the box's centre once rounded.
        const PointF centre{bounds.width / 2, bounds.height / 2 - 1 + 0.75};
        const double reach = (10 - 1.5) / 2 / std::sqrt(2.0);
        const double width = 1.5 * painter.scale();
        painter.drawLine({centre.x - reach, centre.y - reach}, {centre.x + reach, centre.y + reach}, cross, width);
        painter.drawLine({centre.x - reach, centre.y + reach}, {centre.x + reach, centre.y - reach}, cross, width);
    }

    void pointerEnter() override
    {
        m_hovered = true;
        hoverChanged();
    }

    void pointerLeave() override
    {
        m_hovered = false;
        m_pressed = false;
        hoverChanged();
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        m_pressed = true;
        requestPaint();
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        requestPaint();
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    bool isShown() const { return m_ownerHovered || m_hovered; }

    void hoverChanged()
    {
        update();
        if (onHoverChanged)
            onHoverChanged();
    }

    void update() { requestPaint(); }

    Style m_style;
    bool m_ownerHovered = false;
    bool m_hovered = false;
    bool m_pressed = false;
};

// Text in the accent colour that acts when clicked, dimmed while pressed.
class WallpaperLinkButton : public View
{
public:
    WallpaperLinkButton()
    {
        m_label = addChild<Label>();
        m_label->setVerticallyCentered(true);
    }

    void setText(std::string text)
    {
        m_label->setText(std::move(text));
        requestLayout();
    }

    // FishUI's Button keeps its top and bottom padding when the page zeroes
    // the others.
    SizeF sizeHint() const override
    {
        const SizeF label = m_label->sizeHint();
        return {label.width, label.height + 2 * kButtonPadding};
    }
    bool acceptsPointer() const override { return true; }
    // A Qt Quick Controls button: Tab reaches it and Space presses it.
    bool acceptsFocus() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        m_label->setColor(Theme::current().highlight);
        m_label->setGeometry(localRect().adjusted(0, kButtonPadding, 0, -kButtonPadding));
    }

    bool keyPress(const KeyEvent& event) override
    {
        if (event.key != key::Space || event.modifiers != NoModifier)
            return false;
        if (onClicked)
            onClicked();
        return true;
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left)
            setPressed(true);
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        setPressed(false);
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    void setPressed(bool pressed)
    {
        m_pressed = pressed;
        m_label->setOpacity(pressed ? 0.6 : 1);
    }

    Label* m_label;
    bool m_pressed = false;
};

namespace {

// The section's title, hovered as one with its close button.
class SectionTitle : public RowLayout
{
public:
    explicit SectionTitle(std::string icon)
    {
        setSpacing(metrics::controlSpacing);
        this->icon = addChild<Icon>(std::move(icon), 16);
        this->icon->setVisible(!this->icon->iconName().empty());
        this->icon->setLayoutHints({.preferredWidth = 16, .preferredHeight = 16});
        label = addChild<Label>();
        count = addChild<Label>();
        close = addChild<WallpaperCloseButton>(WallpaperCloseButton::Style::Section);
        close->setLayoutHints({.preferredWidth = kCloseButtonSize, .preferredHeight = kCloseButtonSize});
        close->setVisible(false);
        close->onHoverChanged = [this] { update(); };
    }

    void setClosable(bool closable)
    {
        m_closable = closable;
        // Laid out while hidden, so the title does not move when it shows.
        close->setVisible(closable);
        requestLayout();
    }

    bool acceptsPointer() const override { return m_closable; }

    Icon* icon;
    Label* label;
    Label* count;
    WallpaperCloseButton* close;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        icon->setColor(theme.text);
        count->setColor(theme.disabledText);
        count->setFontSize(metrics::points(std::max(theme.fontPointSize - 1, 1.0)));
        RowLayout::layout();
    }

    void pointerEnter() override
    {
        m_hovered = true;
        update();
    }

    void pointerLeave() override
    {
        m_hovered = false;
        update();
    }

private:
    void update() { close->setOwnerHovered(m_closable && (m_hovered || close->isHovered())); }

    bool m_closable = false;
    bool m_hovered = false;
};

} // namespace

// One cell of the grid: a wallpaper's thumbnail, or the tile that adds one.
class WallpaperTile : public View
{
public:
    WallpaperTile(std::string path, bool addTile)
        : m_path(std::move(path))
        , m_addTile(addTile)
    {
        if (m_addTile) {
            m_addLabel = addChild<Label>(tr("Add wallpaper"));
            return;
        }
        m_placeholder = addChild<Icon>("image", 32);
        m_image = addChild<ImageView>();
        m_image->setThumbnail(true);
        m_image->setFillMode(ImageView::FillMode::PreserveAspectCrop);
        // A QML Image in a layer masked to the picture's corners, mipmapped.
        m_image->setRendering(ImageView::Rendering::MaskedLayer);
        m_image->setMipmap(true);
        // Fixed, so a resize of the window decodes nothing again.
        m_image->setSourceSize({kPreferredThumbnailWidth, std::round(kPreferredThumbnailWidth * 0.625)});
        m_image->onStatusChanged = [this](ImageView::Status) { updateImage(); };
        m_image->setSource(m_path);
        m_remove = addChild<WallpaperCloseButton>(WallpaperCloseButton::Style::Thumbnail);
        m_remove->setVisible(false);
        m_remove->onHoverChanged = [this] { updateRemove(); };
        m_remove->onClicked = [this] {
            if (onRemove)
                onRemove();
        };
    }

    const std::string& path() const { return m_path; }

    // Rings the thumbnail once it is there.
    void setSelected(bool selected)
    {
        if (selected == m_selected)
            return;
        m_selected = selected;
        requestPaint();
    }

    void setRemovable(bool removable)
    {
        m_removable = removable;
        if (m_remove)
            m_remove->setVisible(removable);
        updateRemove();
    }

    bool acceptsPointer() const override { return true; }

    // With the point of the window clicked.
    std::function<void(PointF position)> onClicked;
    std::function<void()> onRemove;

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        const RectF picture = pictureRect();
        if (m_addTile) {
            m_addLabel->setColor(theme.text);
            m_addLabel->setFontSize(metrics::points(std::max(theme.fontPointSize - 1, 7.0)));
            const SizeF label = m_addLabel->sizeHint();
            // The plus and the label, a Column centred in the tile: the label
            // at its left, the plus centred over it.
            const double width = std::max(kPlusSize, label.width);
            const double height = kPlusSize + metrics::xs + label.height;
            const double left = picture.x + rounding::centeredStart(picture.width, width);
            const double top = picture.y + rounding::centeredStart(picture.height, height);
            m_plusRect = {left + rounding::centeredStart(width, kPlusSize), top, kPlusSize, kPlusSize};
            m_addLabel->setGeometry({left, top + kPlusSize + metrics::xs, label.width, label.height});
            return;
        }
        m_placeholder->setColor(color(SettingsColor::WallpaperPlaceholder));
        m_placeholder->setGeometry({rounding::centeredStart(geometry().width, 32),
                                    rounding::centeredStart(geometry().height, 32), 32, 32});
        m_image->setCornerRadius(theme.bigRadius, themeCorners());
        // The Image sits 3/8 device px down-right of the frame, so that its
        // layer and the frame round the same way at 125-250%.
        const double shift = 0.375 / (window() ? window()->scale() : 1);
        m_image->setGeometry({picture.x + shift, picture.y + shift, picture.width, picture.height});
        m_remove->setGeometry({geometry().width - metrics::s - kCloseButtonSize, metrics::s, kCloseButtonSize,
                               kCloseButtonSize});
    }

    void paint(Painter& painter) override
    {
        const Theme& theme = Theme::current();
        // The picture's place, drawn as FishUI does a shape inset in its frame.
        const RoundedShape picture{theme.bigRadius, kThumbnailInset, themeCorners()};
        if (m_addTile) {
            painter.drawRoundedRect(pictureRect(), picture, theme.alternateBackground);
            // Two 12 by 1.5 bars, each anchored at the centre of the 18 px box.
            const double along = rounding::centeredStart(kPlusSize, 12);
            const double across = rounding::centeredStart(kPlusSize, 1.5);
            painter.drawSmoothRectangle({m_plusRect.x + along, m_plusRect.y + across, 12, 1.5}, 0.75, theme.text);
            painter.drawSmoothRectangle({m_plusRect.x + across, m_plusRect.y + along, 1.5, 12}, 0.75, theme.text);
            return;
        }
        if (!isReady()) {
            painter.drawRoundedRect(pictureRect(), picture, theme.background);
            return;
        }
        // The ring grows out of the picture's shape, concentric with it.
        if (m_selected)
            painter.drawRoundedRect(localRect(), {theme.bigRadius, -kThumbnailInset, themeCorners()},
                                    Color::rgb(0).withAlpha(0), 2, theme.highlight);
    }

    void pointerEnter() override
    {
        m_hovered = true;
        updateRemove();
    }

    void pointerLeave() override
    {
        m_hovered = false;
        updateRemove();
    }

    void pointerPress(const PointerEvent& event) override { m_pressed = event.button == MouseButton::Left; }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        // The add tile reacts on its picture only, the thumbnail on its whole box.
        const RectF area = m_addTile ? pictureRect() : localRect();
        if (area.contains(event.position) && onClicked)
            onClicked(mapToWindow(event.position));
    }

private:
    static constexpr double kPlusSize = 18;

    RectF pictureRect() const
    {
        return localRect().adjusted(kThumbnailInset, kThumbnailInset, -kThumbnailInset, -kThumbnailInset);
    }

    bool isReady() const { return m_image && m_image->status() == ImageView::Status::Ready; }

    void updateImage()
    {
        m_placeholder->setVisible(!isReady());
        requestPaint();
    }

    void updateRemove()
    {
        if (m_remove)
            m_remove->setOwnerHovered(m_removable && (m_hovered || m_remove->isHovered()));
    }

    std::string m_path;
    bool m_addTile;
    bool m_selected = false;
    bool m_removable = false;
    bool m_hovered = false;
    bool m_pressed = false;
    Label* m_addLabel = nullptr;
    Icon* m_placeholder = nullptr;
    ImageView* m_image = nullptr;
    WallpaperCloseButton* m_remove = nullptr;
    RectF m_plusRect;
};

WallpaperGrid::WallpaperGrid(std::string title, std::string icon)
{
    m_header = addChild<RowLayout>();
    m_header->setSpacing(metrics::controlSpacing);
    auto* header = m_header->addChild<SectionTitle>(std::move(icon));
    header->setLayoutHints({.fillHeight = true});
    header->label->setText(std::move(title));
    header->close->onClicked = [this] {
        if (onCloseRequested)
            onCloseRequested();
    };
    m_title = header;
    m_icon = header->icon;
    m_label = header->label;
    m_count = header->count;
    m_close = header->close;
    m_header->addChild<View>()->setLayoutHints({.fillWidth = true});
    m_showAll = m_header->addChild<WallpaperLinkButton>();
    m_showAll->setName("wallpaper:showAll");
    m_showAll->onClicked = [this] {
        m_expanded = !m_expanded;
        updateHeader();
        requestLayout();
    };
    m_tiles = addChild<View>();
    m_tiles->setClipsChildren(true);
    updateHeader();
}

WallpaperGrid::~WallpaperGrid() = default;

void WallpaperGrid::setWallpapers(std::vector<std::string> wallpapers)
{
    if (wallpapers == m_wallpapers)
        return;
    m_wallpapers = std::move(wallpapers);
    rebuildTiles();
}

void WallpaperGrid::setCurrent(const std::string& path, bool imageSelected)
{
    m_current = path;
    m_imageSelected = imageSelected;
    updateTiles();
}

void WallpaperGrid::setShowAddWallpaper(bool show)
{
    if (show == m_showAddWallpaper)
        return;
    m_showAddWallpaper = show;
    rebuildTiles();
}

void WallpaperGrid::setRemovable(bool removable)
{
    m_removable = removable;
    updateTiles();
}

void WallpaperGrid::setClosable(bool closable)
{
    m_closable = closable;
    static_cast<SectionTitle*>(m_title)->setClosable(closable);
}

int WallpaperGrid::itemCount() const
{
    return int(m_wallpapers.size()) + (m_showAddWallpaper ? 1 : 0);
}

WallpaperGrid::Geometry WallpaperGrid::geometryFor(double width) const
{
    // The spacing between columns, also used between rows.
    const double spacing = metrics::tileSpacing;
    Geometry result;
    const int base = std::max(int(std::floor((width + spacing) / (kPreferredThumbnailWidth + spacing))), 1);
    const double additionalWidth = (width - base * spacing) / (base + 1);
    const bool useAdditional = additionalWidth >= kMinimumThumbnailWidth;
    result.columns = base + (useAdditional ? 1 : 0);
    result.thumbnailWidth = useAdditional ? additionalWidth : std::min(kPreferredThumbnailWidth, width);
    result.thumbnailHeight = std::round(result.thumbnailWidth * 0.625);
    result.cellWidth = width / result.columns;
    result.cellHeight = result.thumbnailHeight + spacing;
    const int count = itemCount();
    result.rows = m_expanded ? (count + result.columns - 1) / result.columns
                             : (std::min(count, result.columns) > 0 ? 1 : 0);
    result.height = std::max(result.rows * result.cellHeight - spacing, 0.0);
    return result;
}

SizeF WallpaperGrid::sizeHint() const
{
    return {0, heightForWidth(geometry().width)};
}

double WallpaperGrid::heightForWidth(double width) const
{
    return kHeaderHeight + metrics::cardSpacing + geometryFor(width).height;
}

void WallpaperGrid::rebuildTiles()
{
    while (!m_tiles->children().empty())
        m_tiles->takeChild(m_tiles->children().back().get());
    m_tileViews.clear();
    std::vector<std::string> items;
    if (m_showAddWallpaper)
        items.push_back({});
    items.insert(items.end(), m_wallpapers.begin(), m_wallpapers.end());
    for (size_t i = 0; i < items.size(); ++i) {
        const bool addTile = m_showAddWallpaper && i == 0;
        WallpaperTile* tile = m_tiles->addChild<WallpaperTile>(items[i], addTile);
        if (addTile) {
            tile->onClicked = [this](PointF position) {
                if (onAddRequested)
                    onAddRequested(position);
            };
        } else {
            tile->onClicked = [this, path = items[i]](PointF) {
                if (onSelected)
                    onSelected(path);
            };
            tile->onRemove = [this, path = items[i]] {
                if (onRemoveRequested)
                    onRemoveRequested(path);
            };
        }
        m_tileViews.push_back(tile);
    }
    updateTiles();
    updateHeader();
    requestLayout();
}

void WallpaperGrid::updateTiles()
{
    for (WallpaperTile* tile : m_tileViews) {
        if (tile->path().empty())
            continue;
        tile->setRemovable(m_removable);
        tile->setSelected(m_imageSelected && tile->path() == m_current);
    }
}

void WallpaperGrid::updateHeader()
{
    m_count->setVisible(!m_wallpapers.empty());
    // TRANSLATORS: the number of wallpapers after a section's title, such as "(12)".
    m_count->setText(format(tr("({count})"), {{"count", std::to_string(m_wallpapers.size())}}));
    m_showAll->setText(m_expanded ? tr("Show less") : tr("Show all"));
    m_title->requestLayout();
}

void WallpaperGrid::layout()
{
    const double width = geometry().width;
    const Geometry grid = geometryFor(width);
    m_showAll->setVisible(itemCount() > grid.columns);
    // The header RowLayout has Layout.preferredHeight 24 in the section's ColumnLayout.
    m_header->setGeometry({0, 0, width, kHeaderHeight});

    const double top = kHeaderHeight + metrics::cardSpacing;
    m_tiles->setGeometry({0, top, width, grid.height});
    for (size_t i = 0; i < m_tileViews.size(); ++i) {
        const int column = int(i) % grid.columns;
        const int row = int(i) / grid.columns;
        WallpaperTile* tile = m_tileViews[i];
        tile->setVisible(row < grid.rows);
        const double offset =
                grid.columns > 1 ? column * (grid.cellWidth - grid.thumbnailWidth) / (grid.columns - 1) : 0;
        tile->setGeometry({column * grid.cellWidth + offset, row * grid.cellHeight, grid.thumbnailWidth,
                           grid.thumbnailHeight});
    }
}

// A colour in a ring that shows while it is chosen or pressed.
class WallpaperSwatch : public View
{
public:
    explicit WallpaperSwatch(std::string color)
        : m_name(std::move(color))
        , m_color(parseColor(m_name))
    {
    }

    const std::string& name() const { return m_name; }

    void setSelected(bool selected)
    {
        m_selected = selected;
        requestPaint();
    }

    SizeF sizeHint() const override { return {kSwatchSize + metrics::m, kSwatchSize + metrics::m}; }
    bool acceptsPointer() const override { return true; }

    std::function<void()> onClicked;

protected:
    void paint(Painter& painter) override
    {
        const RectF bounds = localRect();
        const double radius = bounds.width / 2;
        const double ring = m_selected ? 3 : m_pressed ? 2 : 0;
        if (ring > 0) {
            painter.drawRoundedRect(bounds, {radius, 0, Corners::Circular}, Color::rgb(0).withAlpha(0), ring,
                                    m_color.withAlpha(m_pressed ? 0.6 : 0.4));
        }
        // A parallel curve of the ring, so both snap to the same circle and
        // the gap is even all round.
        const double gap = metrics::m / 2;
        painter.drawRoundedRect(bounds.adjusted(gap, gap, -gap, -gap), {radius, gap, Corners::Circular}, m_color);
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        m_pressed = true;
        requestPaint();
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        requestPaint();
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    std::string m_name;
    Color m_color;
    bool m_selected = false;
    bool m_pressed = false;
};

ColorSwatches::ColorSwatches()
{
    for (const char* color : kSwatchColors) {
        WallpaperSwatch* swatch = addChild<WallpaperSwatch>(color);
        swatch->onClicked = [this, name = std::string(color)] {
            if (onSelected)
                onSelected(name);
        };
        m_swatches.push_back(swatch);
    }
}

void ColorSwatches::setCurrent(const std::string& color, bool colorSelected)
{
    for (WallpaperSwatch* swatch : m_swatches)
        swatch->setSelected(colorSelected && lowerCase(swatch->name()) == lowerCase(color));
}

SizeF ColorSwatches::sizeHint() const
{
    return {0, heightForWidth(geometry().width)};
}

double ColorSwatches::heightForWidth(double width) const
{
    const int columns = std::max(int(std::floor(width / kSwatchCell)), 1);
    return std::ceil(double(m_swatches.size()) / columns) * kSwatchCell;
}

void ColorSwatches::layout()
{
    const int columns = std::max(int(std::floor(geometry().width / kSwatchCell)), 1);
    for (size_t i = 0; i < m_swatches.size(); ++i) {
        const SizeF size = m_swatches[i]->sizeHint();
        m_swatches[i]->setGeometry(
                {(int(i) % columns) * kSwatchCell, (int(i) / columns) * kSwatchCell, size.width, size.height});
    }
}

} // namespace settings
