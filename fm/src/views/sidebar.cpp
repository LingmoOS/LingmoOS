// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/sidebar.h"
#include "dialogs/serverdialogs.h"
#include "dialogs/unlockdialog.h"
#include "model/places.h"
#include "views/style.h"

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/url.h"
#include "ui/clipboard.h"
#include "ui/color_provider.h"
#include "ui/dialog_window.h"
#include "ui/drag.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/menu.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/sidebar.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

using Lingmo::i18n::format;
using Lingmo::i18n::tr;

namespace fm {

static constexpr double kSmallSpacing = 6;

// Where a place is now: disks come and go, and favourites change in other
// windows, while a menu opened on one stays up.
static int favoriteRow(const PlacesModel& places, const std::string& url)
{
    for (int row = 0; row < places.favoriteCount(); ++row) {
        if (places.at(row).url == url)
            return row;
    }
    return -1;
}

static int deviceRow(const PlacesModel& places, const std::string& udi)
{
    for (int row = 0; row < places.count(); ++row) {
        if (places.at(row).udi == udi)
            return row;
    }
    return -1;
}

// The places: every length on the
// device grid, so no row lands on half a pixel at a fractional scale.
class SidebarList : public ui::View
{
public:
    explicit SidebarList(SideBar* sidebar)
        : m_sidebar(sidebar)
    {
        setName("places");
        m_favorites = addChild<ui::Label>(tr("Favorites"));
        m_computer = addChild<ui::SidebarItem>("computer", tr("Computer"), "computer-symbolic");
        m_computer->onActivated = [this] {
            if (m_sidebar->onClicked)
                m_sidebar->onClicked("computer:///");
        };
        m_computer->onContextMenu = [this](ui::PointF position) {
            openComputerMenu(m_computer->mapToWindow(position));
        };
    }

    bool acceptsPointer() const override { return true; }
    ui::SizeF sizeHint() const override { return {0, heightForWidth(0)}; }
    double heightForWidth(double) const override
    {
        PlacesModel& places = m_sidebar->places();
        const double rowHeight = snap(std::max(34.0, style::fontHeight() + 12));
        const double spacing = snap(3);
        double y = snap(32);
        std::string category = "Favorites";
        for (int i = 0; i < places.count(); ++i) {
            if (places.at(i).category != category) {
                category = places.at(i).category;
                // An unnamed section takes no room.
                if (!category.empty())
                    y += snap(36);
            }
            y += rowHeight + spacing;
            if (i == 0)
                y += rowHeight + spacing;
        }
        if (places.count() > 0)
            y -= spacing;
        return y + snap(kSmallSpacing);
    }

    void rebuild()
    {
        PlacesModel& places = m_sidebar->places();
        while (m_rows.size() > size_t(places.count())) {
            removeRow(m_rows.back());
            m_rows.pop_back();
        }
        while (m_rows.size() < size_t(places.count())) {
            Row row;
            row.icon = addChild<ui::ThemeIcon>(std::string(), 20);
            row.icon->setThemeSize(20);
            row.label = addChild<ui::Label>();
            row.label->setElide(ui::Label::Elide::Right);
            row.eject = addChild<ui::ThemeIcon>("media-eject-symbolic", 16);
            row.eject->setThemeSize(16);
            row.section = addChild<ui::Label>();
            m_rows.push_back(row);
        }
        requestLayout();
        requestPaint();
    }

    void retranslate()
    {
        m_favorites->setText(tr("Favorites"));
        requestLayout();
    }

    void setSelected(int row)
    {
        m_selected = row;
        requestLayout();
        requestPaint();
    }

    void setComputerSelected(bool selected)
    {
        m_computer->setCurrent(selected);
        requestLayout();
        requestPaint();
    }

protected:
    struct Row
    {
        ui::ThemeIcon* icon = nullptr;
        ui::Label* label = nullptr;
        ui::ThemeIcon* eject = nullptr;
        ui::Label* section = nullptr;
        ui::RectF rect;
        ui::RectF ejectRect;
        bool ejectVisible = false;
    };

    double snap(double value) const
    {
        const ui::Window* window = this->window();
        const double scale = window ? window->scale() : 1;
        return std::round(value * scale) / scale;
    }

    double sideInset() const { return snap(kSmallSpacing * 1.5); }

    void removeRow(const Row& row)
    {
        for (View* view : {static_cast<View*>(row.icon), static_cast<View*>(row.label),
                           static_cast<View*>(row.eject), static_cast<View*>(row.section)}) {
            std::shared_ptr<View> gone = takeChild(view);
            base::EventLoop::current()->post([gone] {});
        }
    }

    static void styleSection(ui::Label* label)
    {
        label->setFontSize(9 * 4.0 / 3);
        label->setWeight(700);
        label->setColor(ui::Theme::current().disabledText);
        label->setVerticallyCentered(true);
    }

    void place(double width)
    {
        PlacesModel& places = m_sidebar->places();
        const ui::Theme& theme = ui::Theme::current();
        const ui::Window* window = this->window();
        const bool active = !window || window->isActive();
        const double inset = sideInset();
        const double rowHeight = snap(std::max(34.0, style::fontHeight() + 12));
        const double spacing = snap(3);

        styleSection(m_favorites);
        const double headerHeight = snap(32);
        m_favorites->setGeometry({inset + kSmallSpacing, 0, std::max(0.0, width - inset - kSmallSpacing), headerHeight});

        double y = headerHeight;
        std::string category = "Favorites";
        for (int i = 0; i < places.count() && size_t(i) < m_rows.size(); ++i) {
            const PlacesModel::Place& place = places.at(i);
            Row& row = m_rows[size_t(i)];
            if (place.category != category && !place.category.empty()) {
                category = place.category;
                styleSection(row.section);
                row.section->setText(tr(category.c_str()));
                row.section->setVisible(true);
                const double height = snap(36);
                const double left = snap(inset + kSmallSpacing);
                row.section->setGeometry({left, y, std::max(0.0, width - left - kSmallSpacing), height});
                y += height;
            } else {
                category = place.category;
                row.section->setVisible(false);
            }

            row.rect = {0, y, width, rowHeight};
            const bool checked = i == m_selected;
            const ui::Color color = !active ? theme.disabledText : checked ? theme.highlight : theme.text;
            row.icon->setIconName(place.iconName);
            row.icon->setColor(color);
            const double iconX = snap(inset + kSmallSpacing);
            const double iconSize = snap(20);
            row.icon->setGeometry({iconX, y + snap((rowHeight - iconSize) / 2), iconSize, iconSize});

            row.ejectVisible = (place.isDevice() && place.canEject)
                    || (place.isDevice() && !place.setupNeeded() && !place.path().empty() && place.path() != "/");
            const double ejectSize = snap(28);
            row.ejectRect = {snap(width - inset - kSmallSpacing - ejectSize), y + snap((rowHeight - ejectSize) / 2),
                             ejectSize, ejectSize};
            row.eject->setVisible(row.ejectVisible);
            const double glyph = snap(14);
            row.eject->setGeometry({row.ejectRect.x + snap((ejectSize - glyph) / 2),
                                    row.ejectRect.y + snap((ejectSize - glyph) / 2), glyph, glyph});
            row.eject->setColor(color);
            row.eject->setOpacity(m_ejectHovered == i ? 0.85 : 0.45);

            row.label->setText(place.displayName());
            row.label->setColor(color);
            const double labelX = snap(iconX + iconSize + kSmallSpacing);
            const double labelRight = (row.ejectVisible ? row.ejectRect.x : width - inset) - snap(kSmallSpacing);
            const double labelHeight = row.label->sizeHint().height;
            row.label->setGeometry({labelX, y + snap((rowHeight - labelHeight) / 2),
                                    std::max(0.0, labelRight - labelX), labelHeight});
            row.label->setOpacity(1);
            row.icon->setOpacity(m_dragRow == i ? 0.45 : 1);
            row.label->setOpacity(m_dragRow == i ? 0.45 : 1);

            y += rowHeight + spacing;
            if (i == 0) {
                m_computerRect = {inset, y, std::max(0.0, width - inset * 2), rowHeight};
                m_computer->setGeometry(m_computerRect);
                y += rowHeight + spacing;
            }
        }
    }

    void layout() override { place(geometry().width); }

    void paint(ui::Painter& painter) override
    {
        PlacesModel& places = m_sidebar->places();
        const ui::Theme& theme = ui::Theme::current();
        const double inset = sideInset();
        for (int i = 0; i < places.count() && size_t(i) < m_rows.size(); ++i) {
            const Row& row = m_rows[size_t(i)];
            const bool checked = i == m_selected;
            const bool dropTarget = i == m_dropTargetRow;
            const bool rejected = i == m_dropRejectedRow;
            const bool menu = i == m_menuRow;
            const bool pressed = i == m_pressedRow;
            ui::Color fill {0, 0, 0, 0};
            if (rejected)
                fill = style::withAlpha(style::kDanger, 0.16);
            else if (pressed || dropTarget || menu)
                fill = ui::color(ui::ColorId::ItemPressed);
            else if (checked)
                fill = style::textAlpha(0.05);
            const double border = dropTarget || rejected || menu ? 2 : 0;
            if (fill.a == 0 && border == 0)
                continue;
            painter.drawRoundedRect({inset, row.rect.y, row.rect.width - inset * 2, row.rect.height},
                                    {theme.mediumRadius, 0, ui::Corners::Circular}, fill, border,
                                    rejected ? style::kDanger : theme.highlight);
        }

        // Where a dragged folder would be pinned among the favourites.
        if (m_dropIndex >= 0) {
            const double x = inset + kSmallSpacing;
            const double width = geometry().width - x - inset;
            painter.drawRoundedRect({x, m_dropY - 1, width, 2}, {1, 0, ui::Corners::Circular}, theme.highlight);
            painter.drawRoundedRect({x - 3, m_dropY - 3, 6, 6}, {3, 0, ui::Corners::Circular}, theme.highlight);
        }
    }

    int rowAt(ui::PointF point) const
    {
        if (m_computerRect.contains(point))
            return -2;
        for (size_t i = 0; i < m_rows.size(); ++i) {
            const ui::RectF& rect = m_rows[i].rect;
            if (point.y >= rect.y && point.y < rect.y + rect.height)
                return int(i);
        }
        return -1;
    }

    void pointerMove(const ui::PointerEvent& event) override
    {
        const int row = rowAt(event.position);
        const int eject = row >= 0 && m_rows[size_t(row)].ejectVisible
                        && m_rows[size_t(row)].ejectRect.contains(event.position)
                ? row
                : -1;
        if (eject != m_ejectHovered) {
            m_ejectHovered = eject;
            requestLayout();
        }

        // A favourite dragged up or down the list is pinned elsewhere.
        if (m_pressedRow >= 0 && m_press && m_pressedRow < m_sidebar->places().favoriteCount()
            && std::abs(event.position.y - m_press->y) > 10)
            m_dragRow = m_pressedRow;
        if (m_dragRow >= 0) {
            positionDrop(event.position.y, true, {});
            requestLayout();
        }
    }

    void hoverMove(const ui::PointerEvent& event) override { pointerMove(event); }

    void pointerLeave() override
    {
        if (m_ejectHovered >= 0) {
            m_ejectHovered = -1;
            requestLayout();
        }
    }

    void pointerPress(const ui::PointerEvent& event) override
    {
        m_pressedRow = rowAt(event.position);
        m_press = event.position;
        m_pressedButton = event.button;
        requestPaint();
    }

    void pointerRelease(const ui::PointerEvent& event) override
    {
        const int row = m_pressedRow;
        m_pressedRow = -1;
        requestPaint();
        PlacesModel& places = m_sidebar->places();

        if (m_dragRow >= 0) {
            if (m_dropIndex >= 0)
                places.moveFavorite(m_dragRow, m_dropIndex);
            m_dragRow = -1;
            clearDrop();
            requestLayout();
            return;
        }

        if (row == -2)
            return;
        if (row < 0 || row != rowAt(event.position) || row >= places.count())
            return;
        const PlacesModel::Place place = places.at(row);

        if (m_pressedButton == ui::MouseButton::Right) {
            openMenu(row, event.position);
            return;
        }
        if (m_pressedButton != ui::MouseButton::Left)
            return;

        if (m_rows[size_t(row)].ejectVisible && m_rows[size_t(row)].ejectRect.contains(event.position)) {
            if (place.canEject)
                places.requestEject(row);
            else
                places.requestTeardown(row);
            return;
        }

        if (place.isDevice() && place.setupNeeded())
            places.requestSetup(row);
        else if (m_sidebar->onClicked)
            m_sidebar->onClicked(!place.path().empty() ? place.path() : place.url);
    }

    void openMenu(int row, ui::PointF position)
    {
        PlacesModel& places = m_sidebar->places();
        const PlacesModel::Place place = places.at(row);
        const std::string target = !place.path().empty() ? place.path() : place.url;
        const bool favorite = row < places.favoriteCount();
        const bool eject = place.isDevice() && place.canEject;
        const bool unmount = place.isDevice() && !place.setupNeeded() && !place.path().empty() && place.path() != "/";

        m_menu = std::make_unique<ui::Menu>();
        const size_t newTab = m_menu->addItem(tr("Open In New Tab"), [this, target] {
            if (m_sidebar->onOpenInNewTab)
                m_sidebar->onOpenInNewTab(target);
        });
        m_menu->setItemEnabled(newTab, !place.isDevice() || !place.setupNeeded());
        m_menu->addItem(tr("Open In New Window"), [this, target] {
            if (m_sidebar->onOpenInNewWindow)
                m_sidebar->onOpenInNewWindow(target);
        });
        if (favorite) {
            m_menu->addSeparator();
            m_menu->addItem(tr("Remove from Favorites"), [this, url = place.url] {
                PlacesModel& places = m_sidebar->places();
                if (const int current = favoriteRow(places, url); current >= 0)
                    places.removeFavorite(current);
            });
        }
        if (eject || unmount)
            m_menu->addSeparator();
        if (eject) {
            m_menu->addItem(tr("Eject"), [this, udi = place.udi] {
                PlacesModel& places = m_sidebar->places();
                if (const int current = deviceRow(places, udi); current >= 0)
                    places.requestEject(current);
            });
        }
        if (unmount) {
            m_menu->addItem(tr("Unmount"), [this, udi = place.udi] {
                PlacesModel& places = m_sidebar->places();
                if (const int current = deviceRow(places, udi); current >= 0)
                    places.requestTeardown(current);
            });
        }
        m_menuRow = row;
        m_menu->onClosed = [this] {
            m_menuRow = -1;
            requestPaint();
        };
        requestPaint();
        m_menu->popup(window(), mapToWindow(position));
    }

    void openComputerMenu(ui::PointF windowPosition)
    {
        m_menu = std::make_unique<ui::Menu>();
        m_menu->addItem(tr("Open"), [this] {
            if (m_sidebar->onClicked)
                m_sidebar->onClicked("computer:///");
        });
        m_menu->addItem(tr("Open In New Tab"), [this] {
            if (m_sidebar->onOpenInNewTab)
                m_sidebar->onOpenInNewTab("computer:///");
        });
        m_menu->addItem(tr("Open In New Window"), [this] {
            if (m_sidebar->onOpenInNewWindow)
                m_sidebar->onOpenInNewWindow("computer:///");
        });
        m_menu->popup(window(), windowPosition);
    }

    // Drops: files go into a place, folders may also be pinned between favourites.
    bool canDrop(const ui::DropOffer& offer) const override { return offer.hasMimeType("text/uri-list"); }

    void dragEnter(const ui::DragEvent& event) override
    {
        m_dropUrls.clear();
        event.offer->read("text/uri-list", [this, alive = m_alive](std::string data) {
            if (!*alive)
                return;
            ui::MimeData mime;
            mime.setData("text/uri-list", std::move(data));
            m_dropUrls = mime.urls();
        });
    }

    uint32_t dragUpdate(const ui::DragEvent& event) override
    {
        m_dropAction = (event.modifiers & ui::ControlModifier) ? ui::CopyAction : ui::MoveAction;
        positionDrop(event.position.y, false, m_dropUrls);
        requestPaint();
        return m_dropTargetRow >= 0 || m_dropIndex >= 0 ? m_dropAction : ui::NoAction;
    }

    void dragLeave() override
    {
        clearDrop();
        requestPaint();
    }

    uint32_t drop(const ui::DragEvent& event) override
    {
        PlacesModel& places = m_sidebar->places();
        const uint32_t action = (event.modifiers & ui::ControlModifier) ? ui::CopyAction : ui::MoveAction;
        bool accepted = false;
        if (m_dropTargetRow >= 0)
            accepted = places.dropInto(m_dropUrls, m_dropTargetRow, action);
        else if (m_dropIndex >= 0)
            accepted = places.addFavorites(m_dropUrls, m_dropIndex);
        const int target = m_dropTargetRow;
        clearDrop();
        requestPaint();
        if (!accepted)
            return ui::NoAction;
        return target >= 0 ? action : ui::CopyAction;
    }

private:
    void clearDrop()
    {
        m_dropIndex = -1;
        m_dropTargetRow = -1;
        m_dropRejectedRow = -1;
    }

    void positionDrop(double y, bool internal, const std::vector<std::string>& urls)
    {
        PlacesModel& places = m_sidebar->places();
        const int count = places.favoriteCount();
        const bool canPin = !urls.empty() && places.canAddFavorites(urls);
        m_dropTargetRow = -1;
        m_dropRejectedRow = -1;

        if (!internal) {
            const int row = rowAt({geometry().width / 2, y});
            if (row >= 0) {
                const ui::RectF& rect = m_rows[size_t(row)].rect;
                // Favourite rows keep their edges for pinning between rows.
                const double edge = canPin && row < count ? rect.height / 4 : 0;
                if (y >= rect.y + edge && y <= rect.y + rect.height - edge) {
                    m_dropIndex = -1;
                    if (places.canDropInto(urls, row, m_dropAction))
                        m_dropTargetRow = row;
                    else
                        m_dropRejectedRow = row;
                    return;
                }
            }
            if (!canPin || row >= count) {
                m_dropIndex = -1;
                return;
            }
        }

        double boundary = m_favorites->geometry().y + m_favorites->geometry().height;
        m_dropIndex = count;
        for (int i = 0; i < count; ++i) {
            const ui::RectF& rect = m_rows[size_t(i)].rect;
            if (y < rect.y + rect.height / 2) {
                m_dropIndex = i;
                boundary = rect.y;
                break;
            }
            boundary = rect.y + rect.height;
        }
        m_dropY = boundary;
        if (y > boundary + 24)
            m_dropIndex = -1;
    }

    SideBar* m_sidebar;
    ui::Label* m_favorites;
    ui::SidebarItem* m_computer = nullptr;
    ui::RectF m_computerRect;
    std::vector<Row> m_rows;
    int m_selected = -1;
    int m_pressedRow = -1;
    ui::MouseButton m_pressedButton = ui::MouseButton::Left;
    std::optional<ui::PointF> m_press;
    int m_ejectHovered = -1;
    int m_menuRow = -1;
    int m_dragRow = -1;
    int m_dropIndex = -1;
    int m_dropTargetRow = -1;
    int m_dropRejectedRow = -1;
    double m_dropY = 0;
    uint32_t m_dropAction = ui::MoveAction;
    std::vector<std::string> m_dropUrls;
    std::unique_ptr<ui::Menu> m_menu;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

SideBar::SideBar(double headerHeight)
    : m_places(std::make_unique<PlacesModel>())
    , m_headerHeight(headerHeight)
{
    setName("sideBar");
    m_title = addChild<ui::Label>();
    m_title->setFontSize(12 * 4.0 / 3);
    m_title->setElide(ui::Label::Elide::Right);
    m_scroll = addChild<ui::ScrollView>();
    m_list = m_scroll->setContent<SidebarList>(this);
    m_scroll->setVerticalBarPadding(1, kSmallSpacing);

    m_subscriptions.push_back(m_places->changed.connect([this] {
        m_list->rebuild();
        updateSelection(m_selectedPath);
        m_scroll->requestLayout();
    }));
    m_subscriptions.push_back(m_places->favoritesChanged.connect([this] { updateSelection(m_selectedPath); }));
    m_subscriptions.push_back(m_places->deviceEjected.connect([this](const std::string& name) {
        if (onNotification)
            onNotification(format(tr("You can safely remove {name}."), {{"name", name}}));
    }));
    m_subscriptions.push_back(m_places->deviceSetupDone.connect([this](const std::string& path) {
        // Once the device is mounted, as if it had been clicked.
        if (onClicked)
            onClicked(path);
    }));
    m_subscriptions.push_back(m_places->passwordRequested.connect(
            [this](std::string address, std::string user, std::string domain, bool needsDomain,
                   bool anonymousAllowed, bool retry) {
                askPassword(std::move(address), std::move(user), std::move(domain), needsDomain,
                            anonymousAllowed, retry);
            }));
    m_subscriptions.push_back(m_places->hostKeyRequested.connect(
            [this](const std::string& address, const std::string& fingerprint, bool changed) {
                askHostKey(address, fingerprint, changed);
            }));
    m_subscriptions.push_back(m_places->passphraseRequested.connect(
            [this](std::string udi, std::string name, bool retry) {
                askPassphrase(std::move(udi), std::move(name), retry);
            }));
    m_subscriptions.push_back(m_places->deviceOperationFailed.connect(
            [this](const std::string& message) { showDeviceError(message); }));
    m_list->rebuild();
}

SideBar::~SideBar() = default;

void SideBar::connectToServer()
{
    if (!m_connectDialog) {
        m_connectDialog = std::make_unique<ConnectServerDialog>(window(), *m_places);
        m_connectDialog->onDone = [this] {
            // Made anew each time: a closed dialog window is gone.
            std::shared_ptr<ConnectServerDialog> gone(std::move(m_connectDialog));
            base::EventLoop::current()->post([gone] {});
        };
    }
    m_connectDialog->show();
}

void SideBar::askPassword(std::string address, std::string user, std::string domain, bool needsDomain,
                          bool anonymousAllowed, bool retry)
{
    ServerPasswordDialog::Request request;
    request.address = std::move(address);
    request.user = std::move(user);
    request.domain = std::move(domain);
    request.needsDomain = needsDomain;
    request.anonymousAllowed = anonymousAllowed;
    request.retry = retry;
    auto dialog = std::make_unique<ServerPasswordDialog>(window(), *m_places, std::move(request));
    ServerPasswordDialog* raw = dialog.get();
    raw->onDone = [this, raw] {
        auto it = std::find_if(m_passwordDialogs.begin(), m_passwordDialogs.end(),
                               [raw](const auto& dialog) { return dialog.get() == raw; });
        if (it == m_passwordDialogs.end())
            return;
        std::shared_ptr<ServerPasswordDialog> gone(std::move(*it));
        m_passwordDialogs.erase(it);
        base::EventLoop::current()->post([gone] {});
    };
    m_passwordDialogs.push_back(std::move(dialog));
    raw->show();
}

void SideBar::askPassphrase(std::string udi, std::string name, bool retry)
{
    auto dialog = std::make_unique<UnlockVolumeDialog>(window(), *m_places, std::move(udi), std::move(name), retry);
    UnlockVolumeDialog* raw = dialog.get();
    raw->onDone = [this, raw] {
        auto it = std::find_if(m_unlockDialogs.begin(), m_unlockDialogs.end(),
                               [raw](const auto& dialog) { return dialog.get() == raw; });
        if (it == m_unlockDialogs.end())
            return;
        std::shared_ptr<UnlockVolumeDialog> gone(std::move(*it));
        m_unlockDialogs.erase(it);
        base::EventLoop::current()->post([gone] {});
    };
    m_unlockDialogs.push_back(std::move(dialog));
    raw->show();
}

void SideBar::askHostKey(const std::string& address, const std::string& fingerprint, bool changed)
{
    auto dialog = std::make_unique<ui::ConfirmDialog>(window(), std::string());
    ui::ConfirmDialog* raw = dialog.get();
    const auto texts = [raw, address, fingerprint, changed] {
        raw->setTitle(changed ? tr("Server Identity Changed") : tr("Unknown Server"));
        raw->setText(format(changed ? tr("The identity of “{address}” is not the one it had before. Someone "
                                         "may be impersonating it. Connect only if its administrator changed "
                                         "its key.")
                                    : tr("The identity of “{address}” cannot be verified. Check that its key "
                                         "fingerprint is the one you expect before you connect."),
                            {{"address", address}})
                     + "\n\n" + fingerprint);
        raw->setConfirmText(changed ? tr("Connect Anyway") : tr("Connect"));
    };
    texts();
    raw->setDestructive(changed);
    raw->onAccepted = [this, raw, address, fingerprint] {
        m_places->acceptHostKey(address, fingerprint);
        removeConfirm(raw);
    };
    raw->onRejected = [this, raw, address] {
        m_places->cancelConnection(address);
        removeConfirm(raw);
    };
    m_confirmTexts[raw] = texts;
    m_confirmDialogs.push_back(std::move(dialog));
    raw->show();
}

void SideBar::showDeviceError(const std::string& message)
{
    auto dialog = std::make_unique<ui::ConfirmDialog>(window(), std::string(), message);
    ui::ConfirmDialog* raw = dialog.get();
    const auto texts = [raw] {
        raw->setTitle(tr("Device operation failed"));
        raw->setConfirmText(tr("Close"));
    };
    texts();
    raw->setCancelVisible(false);
    raw->onAccepted = [this, raw] { removeConfirm(raw); };
    raw->onRejected = [this, raw] { removeConfirm(raw); };
    m_confirmTexts[raw] = texts;
    m_confirmDialogs.push_back(std::move(dialog));
    raw->show();
}

void SideBar::removeConfirm(ui::ConfirmDialog* dialog)
{
    m_confirmTexts.erase(dialog);
    auto it = std::find_if(m_confirmDialogs.begin(), m_confirmDialogs.end(),
                           [dialog](const auto& entry) { return entry.get() == dialog; });
    if (it == m_confirmDialogs.end())
        return;
    std::shared_ptr<ui::ConfirmDialog> gone(std::move(*it));
    m_confirmDialogs.erase(it);
    base::EventLoop::current()->post([gone] {});
}

void SideBar::retranslate()
{
    m_list->retranslate();
    m_places->retranslate();
    if (m_connectDialog)
        m_connectDialog->retranslate();
    for (const auto& dialog : m_passwordDialogs)
        dialog->retranslate();
    for (const auto& dialog : m_unlockDialogs)
        dialog->retranslate();
    // A question about a server's key keeps its connection waiting for the answer.
    for (const auto& [dialog, texts] : m_confirmTexts)
        texts();
}

void SideBar::setTitle(std::string title)
{
    m_title->setText(std::move(title));
}

void SideBar::updateSelection(const std::string& path)
{
    m_selectedPath = path;
    std::string selected = path;
    if (const auto local = base::url::toLocalFile(selected))
        selected = *local;
    if (selected.starts_with("trash:/"))
        selected = "trash:/";

    int row = -1;
    for (int i = 0; i < m_places->count(); ++i) {
        const PlacesModel::Place& place = m_places->at(i);
        std::string placeUrl = place.url;
        if (placeUrl.starts_with("trash:/"))
            placeUrl = "trash:/";
        if (selected == place.path() || selected == placeUrl) {
            row = i;
            break;
        }
    }
    m_list->setSelected(row);
    m_list->setComputerSelected(selected.starts_with("computer:"));
}

void SideBar::layout()
{
    const ui::Theme& theme = ui::Theme::current();
    const ui::Window* window = this->window();
    const double scale = window ? window->scale() : 1;
    const double inset = std::round(kSmallSpacing * 1.5 * scale) / scale;
    const double width = geometry().width;

    m_title->setColor(!window || window->isActive() ? theme.text : theme.disabledText);
    const double left = inset + kSmallSpacing;
    const double right = theme.largeSpacing + kSmallSpacing;
    m_title->setVerticallyCentered(true);
    m_title->setGeometry({left, kSmallSpacing, std::max(0.0, width - left - right), m_headerHeight - kSmallSpacing});

    const double top = m_headerHeight + kSmallSpacing;
    m_scroll->setGeometry({0, top, width, std::max(0.0, geometry().height - top)});
}

void SideBar::paint(ui::Painter& painter)
{
    // A step below the folder view's colour in both modes.
    painter.fillRect(localRect(), style::withAlpha(ui::Theme::current().background, 0.7));
}

} // namespace fm
