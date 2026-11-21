// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/system_tray_view.h"

#include <algorithm>
#include <cmath>

#include "base/event_loop.h"
#include "core/main_loop.h"
#include "shell/statusbar/dbus_menu.h"
#include "shell/statusbar/dbus_menu_popup.h"
#include "shell/statusbar/status_notifier_item.h"
#include "shell/statusbar/system_tray_model.h"
#include "shell/shell_data.h"
#include "ui/drag.h"
#include "ui/icon.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/window.h"

namespace Lingmo::Shell {

// How long a menu whose first layout is still on its way may take to open.
static constexpr std::chrono::milliseconds kMenuWait {250};
// The tray's menus open this far below the bar (Units.smallSpacing).
static constexpr double kMenuGap = 6;
// A move of the held pointer this far starts a drag (startDragDistance).
static constexpr double kDragThreshold = 10;
static constexpr char kTrayItemMimeType[] = "application/x-lingmo-tray-item";

class OverflowButton final : public BarItem
{
public:
    OverflowButton(BarContext *bar, std::function<void()> clicked)
        : BarItem(bar)
    {
        setName("tray-overflow");
        setInsets(BarContext::kHighlightGap / 2, BarContext::kHighlightGap / 2);
        setGlyphWidth(16);
        onClicked = [clicked = std::move(clicked)](const ui::PointerEvent &) { clicked(); };
    }

    void setOpen(bool open, ui::Window *window)
    {
        if (open == m_open && !m_animation)
            return;
        m_open = open;
        if (!window) {
            m_angle = open ? 180 : 0;
            requestPaint();
            return;
        }
        if (!m_animation) {
            m_animation = std::make_unique<ui::Animation>(window);
            m_animation->setDuration(std::chrono::milliseconds(180));
            m_animation->setEasing(ui::Animation::Easing::InOutQuad);
            m_animation->onUpdate = [this](double value) {
                m_angle = m_from + (m_to - m_from) * value;
                requestPaint();
            };
            m_animation->onFinished = [this] { m_animation.reset(); };
        }
        m_from = m_angle;
        m_to = open ? 180 : 0;
        m_animation->start();
    }

protected:
    void paint(ui::Painter &painter) override
    {
        BarItem::paint(painter);
        painter.save();
        painter.rotate(m_angle, {geometry().width / 2, geometry().height / 2});
        painter.drawSvg(shellDataFile("statusbar/chevron.svg"), centred({16, 16}), bar()->textColor());
        painter.restore();
    }

private:
    std::unique_ptr<ui::Animation> m_animation;
    double m_angle = 0;
    double m_from = 0;
    double m_to = 0;
    bool m_open = false;
};

class OverflowRoot final : public ui::View
{
public:
    OverflowRoot() { setClipsChildren(true); }

    void add(std::unique_ptr<TrayItemView> item)
    {
        m_items.push_back(item.get());
        addChild(std::move(item));
    }

    void layout() override
    {
        const int columns = std::max(1, int(std::floor((geometry().width - 24) / SystemTrayView::kItemWidth)));
        for (size_t i = 0; i < m_items.size(); ++i) {
            const int col = int(i % size_t(columns));
            const int row = int(i / size_t(columns));
            m_items[i]->setGeometry({12 + col * SystemTrayView::kItemWidth,
                                     12 + row * SystemTrayView::kItemWidth,
                                     SystemTrayView::kItemWidth, SystemTrayView::kItemWidth});
        }
    }

    double popupWidth() const
    {
        const int columns = std::max(1, std::min(10, int(m_items.size())));
        return columns * SystemTrayView::kItemWidth + 24;
    }

    double popupHeight() const
    {
        const int columns = std::max(1, std::min(10, int(m_items.size())));
        const int rows = std::max(1, int((m_items.size() + size_t(columns) - 1) / size_t(columns)));
        return rows * SystemTrayView::kItemWidth + 24;
    }

private:
    std::vector<TrayItemView *> m_items;
};

// FishUI's ThemeManager::isMonochromeIcon on a 32 px rendering: every
// opaque pixel one colour, and that colour nearly grey.
static bool isMonochromeBitmap(const ui::Bitmap &bitmap)
{
    if (bitmap.isNull())
        return false;
    bool seen = false;
    uint32_t first = 0;
    for (int y = 0; y < bitmap.height(); ++y) {
        for (int x = 0; x < bitmap.width(); ++x) {
            const uint32_t pixel = bitmap.pixel(x, y);
            const uint32_t alpha = pixel >> 24;
            if (alpha < 250)
                continue;
            // Unpremultiplied, rounded to nearest.
            const auto channel = [alpha](uint32_t value) { return std::min<uint32_t>(255, (value * 255 + alpha / 2) / alpha); };
            const uint32_t opaque = 0xff000000u | channel((pixel >> 16) & 0xff) << 16
                    | channel((pixel >> 8) & 0xff) << 8 | channel(pixel & 0xff);
            if (!seen) {
                seen = true;
                first = opaque;
                continue;
            }
            if (opaque != first)
                return false;
        }
    }
    if (!seen)
        return false;
    const int red = int((first >> 16) & 0xff);
    const int green = int((first >> 8) & 0xff);
    const int blue = int(first & 0xff);
    const int max = std::max({red, green, blue});
    const int min = std::min({red, green, blue});
    // HSV saturation, 0-255.
    const int saturation = max == 0 ? 0 : (255 * (max - min) + max / 2) / max;
    return saturation < 24;
}

// The choice among an item's pictures for a size: the smallest at least
// that large, else the largest, scaled to it.
static ui::Bitmap pickPixmap(const std::vector<ui::Bitmap> &pixmaps, int side)
{
    const ui::Bitmap *best = nullptr;
    for (const ui::Bitmap &pixmap : pixmaps) {
        if (pixmap.isNull())
            continue;
        const int extent = std::max(pixmap.width(), pixmap.height());
        if (!best) {
            best = &pixmap;
            continue;
        }
        const int bestExtent = std::max(best->width(), best->height());
        if ((extent >= side && (bestExtent < side || extent < bestExtent)) || (bestExtent < side && extent > bestExtent))
            best = &pixmap;
    }
    if (!best)
        return {};
    if (best->width() == side && best->height() == side)
        return *best;
    return best->scaled(side, side, true);
}

// Unpremultiplied 0xAARRGGBB, as Painter::drawImage takes it.
static ui::Image toImage(const ui::Bitmap &bitmap)
{
    if (bitmap.isNull())
        return {};
    ui::Image image(bitmap.width(), bitmap.height());
    for (int y = 0; y < bitmap.height(); ++y) {
        for (int x = 0; x < bitmap.width(); ++x) {
            const uint32_t pixel = bitmap.pixel(x, y);
            const uint32_t a = pixel >> 24;
            const auto channel = [a](uint32_t value) { return a ? std::min<uint32_t>(255, (value * 255 + a / 2) / a) : 0; };
            image.bits()[size_t(y) * size_t(bitmap.width()) + size_t(x)] = a << 24
                    | channel((pixel >> 16) & 0xff) << 16 | channel((pixel >> 8) & 0xff) << 8 | channel(pixel & 0xff);
        }
    }
    return image;
}

SystemTrayView::SystemTrayView(BarContext *bar, SystemTrayModel *model, Host host)
    : m_bar(bar)
    , m_model(model)
    , m_host(std::move(host))
    , m_menuWait(core::mainLoop())
{
    setClipsChildren(true);
    m_menuWait.setSingleShot(true);
    if (m_model) {
        const auto reset = [this] { rebuild(); };
        m_subscriptions.push_back(m_model->rowsInserted.connect([reset](int, int) { reset(); }));
        m_subscriptions.push_back(m_model->rowsRemoved.connect([reset](int, int) { reset(); }));
        m_subscriptions.push_back(m_model->rowMoved.connect([this](int, int) {
            m_animateMoves = true;
            rebuild();
        }));
        m_subscriptions.push_back(m_model->dataChanged.connect([reset](int, int) { reset(); }));
        m_subscriptions.push_back(m_model->modelReset.connect(reset));
    }
    rebuild();
}

SystemTrayView::~SystemTrayView()
{
    *m_alive = false;
    m_subscriptions.clear();
    m_menuLoaded.reset();
    m_popup.reset();
    closeOverflow();
}

double SystemTrayView::preferredWidth() const
{
    const int visible = visibleCount();
    return kItemWidth * (visible + (visible < count() ? 1 : 0));
}

int SystemTrayView::visibleCount() const
{
    double width = 480;
    if (m_host.availableGeometry) {
        const ui::RectF area = m_host.availableGeometry();
        if (area.width > 0)
            width = std::max(160.0, area.width * 0.24);
    }
    return std::min(count(), std::max(1, int(std::floor(width / kItemWidth)) - 1));
}

TrayItemView *SystemTrayView::item(int row) const
{
    if (row < 0 || row >= count())
        return nullptr;
    return m_items[size_t(row)];
}

void SystemTrayView::rebuild()
{
    std::vector<TrayItemView *> items;
    const int rows = m_model ? m_model->count() : 0;
    for (int row = 0; row < rows; ++row) {
        StatusNotifierItem *source = m_model->at(row);
        auto it = std::find_if(m_items.begin(), m_items.end(),
                               [&](TrayItemView *view) { return view->itemId() == source->id(); });
        TrayItemView *view;
        if (it != m_items.end()) {
            view = *it;
            m_items.erase(it);
        } else {
            view = addChild<TrayItemView>(m_bar, this, source->id());
        }
        view->setIcon(source->iconName(), source->pixmaps(), source->applicationIcon());
        items.push_back(view);
    }
    // Items that left: their menu goes with them.
    for (TrayItemView *gone : m_items) {
        // The item's DBusMenu is already gone with it.
        if (gone == m_popupItem && m_popup) {
            m_popup->detach();
            closeMenu();
            m_popup.reset();
            m_popupItem = nullptr;
        }
        if (gone == m_menuItem)
            closeMenu();
        gone->setVisible(false);
        std::shared_ptr<ui::View> retired(takeChild(gone));
        core::mainLoop()->post([retired] {});
    }
    m_items = std::move(items);
    for (TrayItemView *view : m_items)
        view->updateLook();
    if (!m_overflowButton)
        m_overflowButton = addChild<OverflowButton>(m_bar, [this] { toggleOverflow(); });
    requestLayout();
    if (onCountChanged)
        onCountChanged();
}

void SystemTrayView::toggleOverflow()
{
    if (m_overflowWindow) {
        closeOverflow();
        return;
    }
    if (m_host.closePopups)
        m_host.closePopups();
    const int visible = visibleCount();
    auto root = std::make_unique<OverflowRoot>();
    OverflowRoot *rawRoot = root.get();
    m_overflowItems.clear();
    for (int row = visible; row < count(); ++row) {
        TrayItemView *item = m_items[size_t(row)];
        item->setVisible(true);
        std::unique_ptr<ui::View> child = takeChild(item);
        if (!child)
            continue;
        TrayItemView *rawItem = static_cast<TrayItemView *>(child.release());
        m_overflowItems.push_back(rawItem);
        rawRoot->add(std::unique_ptr<TrayItemView>(rawItem));
    }
    ui::PopupPlacement placement;
    const ui::RectF area = m_host.availableGeometry ? m_host.availableGeometry() : ui::RectF();
    const bool bottomBar = area.height > 0 && area.y < 0;
    placement.anchorRect = {geometry().width - kItemWidth, bottomBar ? 0 : geometry().height, kItemWidth, 1};
    placement.anchor = bottomBar ? ui::PopupAnchor::BottomRight : ui::PopupAnchor::TopRight;
    placement.gravity = bottomBar ? ui::PopupAnchor::BottomRight : ui::PopupAnchor::TopRight;
    placement.flipX = true;
    placement.flipY = false;
    placement.slide = true;
    m_overflowWindow = ui::Window::createPopup(window(), placement, true);
    m_overflowRoot = rawRoot;
    m_overflowWindow->setContentView(std::move(root));
    m_overflowWindow->resize({rawRoot->popupWidth(), rawRoot->popupHeight()});
    m_overflowWindow->setBackgroundBlur({0, 0, rawRoot->popupWidth(), rawRoot->popupHeight()}, 16);
    m_overflowWindow->onCloseRequested = [this] { closeOverflow(); };
    m_overflowWindow->show();
    m_overflowButton->setOpen(true, window());
}

void SystemTrayView::closeOverflow()
{
    if (!m_overflowWindow)
        return;
    m_overflowButton->setOpen(false, window());
    for (TrayItemView *item : m_overflowItems) {
        std::unique_ptr<ui::View> child = m_overflowRoot->takeChild(item);
        if (child)
            addChild(std::move(child));
    }
    m_overflowItems.clear();
    m_overflowRoot = nullptr;
    m_overflowWindow->hide();
    m_overflowWindow.reset();
    requestLayout();
}

double SystemTrayView::slotX(int row) const
{
    // RightToLeft: the first item at the right end.
    return geometry().width - kItemWidth * (row + 1);
}

void SystemTrayView::layout()
{
    const double height = geometry().height;
    if (m_animateMoves && window()) {
        m_animateMoves = false;
        m_moves.clear();
        for (int row = 0; row < count(); ++row) {
            TrayItemView *item = m_items[size_t(row)];
            const double from = item->geometry().width > 0 ? item->geometry().x : slotX(row);
            if (from != slotX(row))
                m_moves[item] = {from, slotX(row)};
        }
        if (!m_moves.empty()) {
            if (!m_moveAnimation) {
                m_moveAnimation = std::make_unique<ui::Animation>(window());
                m_moveAnimation->setDuration(std::chrono::milliseconds(200));
                m_moveAnimation->setEasing(ui::Animation::Easing::InOutQuad);
                m_moveAnimation->onUpdate = [this](double value) {
                    m_moveProgress = value;
                    requestLayout();
                };
                m_moveAnimation->onFinished = [this] {
                    m_moveProgress = 1;
                    m_moves.clear();
                    requestLayout();
                };
            }
            m_moveProgress = 0;
            m_moveAnimation->start();
        }
    }
    const int visible = visibleCount();
    for (int row = 0; row < count(); ++row) {
        TrayItemView *item = m_items[size_t(row)];
        if (row >= visible) {
            item->setVisible(false);
            continue;
        }
        item->setVisible(true);
        double x = slotX(row + (visible < count() ? 1 : 0));
        if (const auto it = m_moves.find(item); it != m_moves.end())
            x = it->second.first + (it->second.second - it->second.first) * m_moveProgress;
        item->setGeometry({x, 0, kItemWidth, height});
    }
    if (m_overflowButton) {
        m_overflowButton->setVisible(visible < count());
        m_overflowButton->setGeometry({0, 0, kItemWidth, height});
    }
}

void SystemTrayView::startItemDrag(TrayItemView *item)
{
    if (!m_dragged.empty())
        return;
    const ui::Window *w = window();
    const double scale = w ? w->scale() : 1;
    ui::DragSource source;
    source.mimeTypes = {kTrayItemMimeType};
    source.data = [id = item->itemId()](const std::string &) { return id; };
    source.actions = ui::MoveAction;
    source.icon = item->dragImage(scale);
    source.iconScale = scale;
    source.hotSpot = {BarContext::kIconSize / 2, BarContext::kIconSize / 2};
    const std::string id = item->itemId();
    source.onFinished = [this, id, alive = m_alive](ui::DropAction) {
        if (!*alive)
            return;
        m_dragged.clear();
        for (TrayItemView *view : m_items) {
            if (view->itemId() == id)
                view->setIconHidden(false);
        }
    };
    m_dragged = id;
    item->setIconHidden(true);
    // The drag takes the pointer: no release comes to end the press.
    item->cancelPress();
    if (!item->startDrag(std::move(source))) {
        m_dragged.clear();
        item->setIconHidden(false);
    }
}

void SystemTrayView::dragEntered(TrayItemView *target)
{
    // The order changes as the drag enters an item, and only in memory.
    const int from = m_model->indexOf(m_dragged);
    const int to = m_model->indexOf(target->itemId());
    if (from >= 0 && to >= 0 && from != to)
        m_model->move(from, to);
}

void SystemTrayView::barChanged()
{
    for (TrayItemView *view : m_items)
        view->updateLook();
}

ui::Menu *SystemTrayView::openMenu() const
{
    return m_popup && m_popup->menu().isOpen() ? &m_popup->menu() : nullptr;
}

void SystemTrayView::closeMenu()
{
    m_menuWait.stop();
    m_menuLoaded.reset();
    if (m_popup && m_popup->menu().isOpen())
        m_popup->menu().close();
    if (m_menuItem) {
        m_menuItem->setContextMenuPressed(false);
        m_menuItem = nullptr;
    }
}

void SystemTrayView::itemClicked(TrayItemView *item, const ui::PointerEvent &event)
{
    // Where the click was on the screen: only a hint for the application.
    const ui::PointF inWindow = item->mapToWindow(event.position);
    const ui::PointF origin = m_host.origin ? m_host.origin() : ui::PointF {};
    const int x = int(std::lround(origin.x + inWindow.x));
    const int y = int(std::lround(origin.y + inWindow.y));
    const StatusNotifierItem *source = m_model->find(item->itemId());
    switch (event.button) {
    case ui::MouseButton::Left:
        if (source && source->itemIsMenu()) {
            toggleMenu(item, {double(x), double(y)});
            break;
        }
        if (m_host.closePopups)
            m_host.closePopups();
        m_model->leftButtonClick(item->itemId(), x, y);
        break;
    case ui::MouseButton::Middle:
        if (m_host.closePopups)
            m_host.closePopups();
        m_model->middleButtonClick(item->itemId(), x, y);
        break;
    case ui::MouseButton::Right:
        toggleMenu(item, {double(x), double(y)});
        break;
    }
}

void SystemTrayView::toggleMenu(TrayItemView *item, const ui::PointF &position)
{
    if (m_menuItem == item && openMenu())
        closeMenu();
    else
        openContextMenu(item, position);
}

void SystemTrayView::openContextMenu(TrayItemView *item, const ui::PointF &position)
{
    if (m_host.closePanels)
        m_host.closePanels();
    if (m_host.closeMenuBar)
        m_host.closeMenuBar();

    DBusMenu *menu = m_model->contextMenu(item->itemId(), int(position.x), int(position.y));
    closeMenu();
    // An item without an exported menu was asked to show its own.
    if (!menu)
        return;
    m_menuItem = item;
    item->setContextMenuPressed(true);
    // The menu opens with the rows it has once AboutToShow's answer and any
    // layout it asked for arrived, or after a moment without them.
    m_menuLoaded = menu->menuUpdated.connect([this, item, menu](int32_t id) {
        if (id != 0 || m_menuItem != item)
            return;
        m_menuWait.stop();
        core::mainLoop()->post([this, item, menu, alive = m_alive] {
            if (*alive && m_menuItem == item)
                showMenu(item, menu);
        });
    });
    m_menuWait.onTimeout = [this, item, menu] {
        if (m_menuItem == item)
            showMenu(item, menu);
    };
    m_menuWait.start(kMenuWait);
}

void SystemTrayView::showMenu(TrayItemView *item, DBusMenu *menu)
{
    m_menuWait.stop();
    m_menuLoaded.reset();
    if (m_popup && m_popup->menu().isOpen())
        return;
    m_popup = std::make_unique<DBusMenuPopup>(menu, 0);
    m_popup->setPrepared();
    m_popupItem = item;
    m_popup->onClosed = [this] { menuClosed(); };
    ui::Menu &popup = m_popup->menu();
    if (m_host.availableGeometry)
        popup.setAvailableGeometry(m_host.availableGeometry());
    m_menuItem = item;
    item->setContextMenuPressed(true);
    popup.popupAt(window(), item->mapToWindow({item->leftInset(), item->geometry().height + kMenuGap}));
}

void SystemTrayView::menuClosed()
{
    if (m_menuItem) {
        m_menuItem->setContextMenuPressed(false);
        m_menuItem = nullptr;
    }
}

TrayItemView::TrayItemView(BarContext *bar, SystemTrayView *tray, std::string id)
    : BarItem(bar)
    , m_tray(tray)
    , m_id(std::move(id))
{
    setName("tray:" + m_id);
    setInsets(BarContext::kHighlightGap / 2, BarContext::kHighlightGap / 2);
    setGlyphWidth(BarContext::kIconSize);
    onClicked = [this](const ui::PointerEvent &event) { m_tray->itemClicked(this, event); };
    onPositionChanged = [this](const ui::PointerEvent &event) {
        if (!isPressed() || event.button != ui::MouseButton::Left)
            return;
        const ui::PointF delta {event.position.x - pressPosition().x, event.position.y - pressPosition().y};
        if (std::abs(delta.x) > kDragThreshold || std::abs(delta.y) > kDragThreshold)
            m_tray->startItemDrag(this);
    };
}

TrayItemView::~TrayItemView() = default;

void TrayItemView::setIcon(const std::string &name, const std::vector<ui::Bitmap> &pixmaps,
                           const std::string &fallback)
{
    std::string themeName;
    std::string file;
    std::vector<ui::Bitmap> pictures;
    if (!name.empty()) {
        themeName = name;
    } else if (!pixmaps.empty()) {
        pictures = pixmaps;
    } else if (!fallback.empty() && fallback.front() == '/') {
        file = fallback;
    } else {
        themeName = fallback.empty() ? "application-x-desktop" : fallback;
    }

    const bool samePictures = pictures.size() == m_pixmaps.size()
            && std::equal(pictures.begin(), pictures.end(), m_pixmaps.begin());
    if (themeName == m_name && file == m_file && samePictures)
        return;
    m_name = themeName;
    m_file = file;
    m_pixmaps = std::move(pictures);
    m_image = ui::Image();
    m_tinted = ui::Image();

    if (!m_name.empty()) {
        if (!m_themeIcon)
            m_themeIcon = addChild<ui::ThemeIcon>(m_name, BarContext::kIconSize);
        m_themeIcon->setIconName(m_name);
        m_themeIcon->setVisible(!m_iconHidden);
    } else if (m_themeIcon) {
        m_themeIcon->setVisible(false);
    }

    // The single-colour verdict, on a 32 px rendering made on a worker.
    const std::string key = !m_name.empty() ? "name:" + m_name : !m_file.empty() ? "file:" + m_file : std::string();
    m_probed.clear();
    if (!m_pixmaps.empty()) {
        setMonochrome(isMonochromeBitmap(pickPixmap(m_pixmaps, 32)));
        updatePixmapImage();
        return;
    }
    const std::string path = !m_name.empty() ? ui::themeIconPath(m_name, 32, 1) : m_file;
    m_probe = base::ThreadPool::global().run(
            [path, file = m_file] {
                std::pair<bool, ui::Bitmap> result;
                const ui::Bitmap probe = ui::Bitmap::decodeFile(path, {32, 32}, ui::Bitmap::Scaling::Fit);
                result.first = isMonochromeBitmap(probe);
                if (!file.empty())
                    result.second = ui::Bitmap::decodeFile(file, {64, 64}, ui::Bitmap::Scaling::Fit);
                return result;
            },
            [this, key](std::pair<bool, ui::Bitmap> result) {
                m_probed = key;
                if (!result.second.isNull())
                    m_pixmaps = {result.second};
                setMonochrome(result.first);
                updatePixmapImage();
            });
    requestPaint();
}

void TrayItemView::setMonochrome(bool monochrome)
{
    m_monochrome = monochrome;
    updateLook();
}

void TrayItemView::updateLook()
{
    const ui::Color color = bar()->textColor();
    if (m_themeIcon)
        m_themeIcon->setColor(m_monochrome ? std::optional<ui::Color>(color) : std::nullopt);
    if (m_monochrome && m_tintedColor != color && !m_image.isNull()) {
        m_tintedColor = color;
        m_tinted = ui::Image(m_image.width(), m_image.height());
        for (int y = 0; y < m_image.height(); ++y) {
            for (int x = 0; x < m_image.width(); ++x) {
                const ui::Color pixel = m_image.pixel(x, y);
                m_tinted.setPixel(x, y, color.withAlpha(pixel.a / 255.0 * color.a / 255.0));
            }
        }
    }
    barChanged();
}

void TrayItemView::updatePixmapImage()
{
    const ui::Window *w = window();
    const double scale = w ? w->scale() : 1;
    m_image = toImage(pickPixmap(m_pixmaps, int(std::lround(BarContext::kIconSize * scale))));
    m_tintedColor = {};
    m_tinted = ui::Image();
    updateLook();
}

void TrayItemView::layout()
{
    // A picture is picked for the device size, which follows the scale.
    const ui::Window *w = window();
    const int side = int(std::lround(BarContext::kIconSize * (w ? w->scale() : 1)));
    if (!m_pixmaps.empty() && m_image.width() != side)
        updatePixmapImage();
    if (m_themeIcon)
        m_themeIcon->setGeometry(centred({BarContext::kIconSize, BarContext::kIconSize}));
}

void TrayItemView::setIconHidden(bool hidden)
{
    m_iconHidden = hidden;
    if (m_themeIcon)
        m_themeIcon->setVisible(!hidden && !m_name.empty());
    requestPaint();
}

ui::Bitmap TrayItemView::dragImage(double scale) const
{
    const int side = int(std::lround(BarContext::kIconSize * scale));
    ui::Bitmap bitmap;
    const ui::Image &image = m_monochrome && !m_tinted.isNull() ? m_tinted : m_image;
    if (!image.isNull()) {
        bitmap = ui::Bitmap(image.width(), image.height(), ui::Bitmap::Format::Argb32Premultiplied);
        for (int y = 0; y < image.height(); ++y) {
            for (int x = 0; x < image.width(); ++x) {
                const ui::Color c = image.pixel(x, y);
                const auto premultiply = [&c](uint8_t value) { return uint32_t((value * c.a + 127) / 255); };
                bitmap.setPixel(x, y, uint32_t(c.a) << 24 | premultiply(c.r) << 16 | premultiply(c.g) << 8
                                        | premultiply(c.b));
            }
        }
        return bitmap.scaled(side, side, true);
    }
    // A theme icon's file, read here: a small one, once per drag.
    const std::string path = m_name.empty() ? std::string() : ui::themeIconPath(m_name, 16, scale);
    bitmap = path.empty() ? ui::Bitmap() : ui::Bitmap::decodeFile(path, {side, side}, ui::Bitmap::Scaling::Fit);
    if (!bitmap.isNull() && m_monochrome) {
        const ui::Color color = bar()->textColor();
        for (int y = 0; y < bitmap.height(); ++y) {
            for (int x = 0; x < bitmap.width(); ++x) {
                const uint32_t alpha = bitmap.alpha(x, y);
                const auto channel = [alpha](uint8_t value) { return uint32_t((value * alpha + 127) / 255); };
                bitmap.setPixel(x, y, alpha << 24 | channel(color.r) << 16 | channel(color.g) << 8 | channel(color.b));
            }
        }
    }
    return bitmap;
}

bool TrayItemView::canDrop(const ui::DropOffer &offer) const
{
    return offer.hasMimeType(kTrayItemMimeType) && !m_tray->draggedItem().empty();
}

void TrayItemView::dragEnter(const ui::DragEvent &)
{
    m_tray->dragEntered(this);
}

uint32_t TrayItemView::dragUpdate(const ui::DragEvent &)
{
    return ui::MoveAction;
}

uint32_t TrayItemView::drop(const ui::DragEvent &)
{
    return ui::MoveAction;
}

void TrayItemView::paint(ui::Painter &painter)
{
    BarItem::paint(painter);
    if (m_image.isNull() || m_iconHidden)
        return;
    const ui::RectF rect = centred({BarContext::kIconSize, BarContext::kIconSize});
    painter.drawImage(m_monochrome && !m_tinted.isNull() ? m_tinted : m_image, rect);
}

} // namespace Lingmo::Shell
