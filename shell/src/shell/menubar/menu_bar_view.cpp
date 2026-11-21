// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/menubar/menu_bar_view.h"

#include "core/main_loop.h"
#include "lingmo/accounts/user_account.h"
#include "shell/shell.h"
#include "shell/statusbar/bar_item.h"
#include "shell/statusbar/status_bar.h"
#include "ui/icon.h"
#include "ui/image_view.h"
#include "ui/menu.h"
#include "ui/window.h"

namespace Lingmo::Shell {

// One entry of the bar: the user's avatar, or a title in the bar's text.
// Spaced like the status items on the right; the user's entry reaches the
// screen edge and mirrors the clock there.
class MenuBarView::Entry final : public BarItem
{
public:
    Entry(BarContext *bar, const MenuBarModel::Row &row)
        : BarItem(bar)
        , m_row(row)
    {
        if (row.kind == MenuBarModel::Kind::User) {
            setInsets(BarContext::kEdgeMargin - (BarContext::kItemGap - BarContext::kHighlightGap) / 2,
                      BarContext::kHighlightGap / 2);
            setGlyphWidth(BarContext::kIconSize);
            // The system logo shown at its original shape, not a circular
            // avatar mask. The logo entry replaces the user's picture; the
            // user menu still opens below it.
            m_logo = addChild<ui::ThemeIcon>("systemlogo", BarContext::kIconSize);
            m_logo->setThemeSize(int(BarContext::kIconSize));
        } else {
            setInsets(BarContext::kHighlightGap / 2, BarContext::kHighlightGap / 2);
            m_label = addChild<BarLabel>();
            m_label->setText(row.title);
        }
        updateLook();
    }

    const MenuBarModel::Row &row() const { return m_row; }

    double preferredWidth() const
    {
        if (m_logo)
            return BarContext::kItemGap / 2 + BarContext::kEdgeMargin + BarContext::kIconSize
                    + BarContext::kSmallSpacing * 2;
        return BarContext::kItemGap + m_label->sizeHint().width + BarContext::kSmallSpacing * 2;
    }

    // No-op: the logo is always systemlogo; user picture is not displayed.
    void setAvatar(const std::string &) {}

    void updateLook()
    {
        if (m_label) {
            // The application's name stands out from its menus, a step
            // lighter in black, which carries more weight than white.
            if (m_row.kind == MenuBarModel::Kind::Application)
                m_label->setWeight(bar()->lightContents() ? 600 : 500);
            m_label->setTextColor(bar()->menuTextColor());
            m_label->setDropShadow(bar()->lightContents());
        }
        barChanged();
    }

protected:
    void layout() override
    {
        if (m_logo)
            m_logo->setGeometry(centred({BarContext::kIconSize, BarContext::kIconSize}));
        if (m_label)
            m_label->setGeometry(centred(m_label->sizeHint()));
    }

private:
    const MenuBarModel::Row m_row;
    BarLabel *m_label = nullptr;
    ui::ThemeIcon *m_logo = nullptr;
};

MenuBarView::MenuBarView(Shell *shell, StatusBarMenuBarHost host)
    : m_host(std::move(host))
{
    setName("menubar-view");
    StatusBar *statusBar = m_host.statusBar;
    m_model = std::make_unique<MenuBarModel>(shell, statusBar ? &statusBar->appMenuRegistrar() : nullptr,
                                             statusBar ? &statusBar->powerActions() : nullptr);

    m_subscriptions.push_back(m_model->switched.connect([this] {
        // The user's menu is not the application's; it stays.
        if (m_currentIndex > 0)
            close();
    }));
    m_subscriptions.push_back(m_model->rowsChanged.connect([this] { rebuildEntries(); }));
    // How a menu's shortcut arrives.
    m_subscriptions.push_back(
            m_model->activationRequested.connect([this](size_t row) { open(int(row), true); }));
    m_subscriptions.push_back(m_model->menuClosed.connect([this] { onMenuClosed(); }));

    setUserIcon({});
    if (shell->bus()) {
        if (base::dbus::Connection *system = base::dbus::Connection::system(core::mainLoop())) {
            m_account = std::make_unique<accounts::UserAccount>(system);
            m_subscriptions.push_back(m_account->changed.connect([this] { setUserIcon(m_account->iconFileName()); }));
            setUserIcon(m_account->iconFileName());
        }
    }
    rebuildEntries();
}

MenuBarView::~MenuBarView()
{
    m_subscriptions.clear();
    m_switching = true;
    if (m_openMenu)
        m_openMenu->close();
    m_openMenu = nullptr;
    m_model.reset();
}

BarItem *MenuBarView::entry(size_t index) const
{
    return index < m_entries.size() ? m_entries[index] : nullptr;
}

void MenuBarView::setUserIcon(const std::string &file)
{
    // The system logo stands in for an account without a picture, as FishUI's
    // image://icontheme/systemlogo did; the theme's default-user is the last
    // resort.
    m_userIcon = file;
    if (m_userIcon.empty())
        m_userIcon = ui::themeIconPath("systemlogo", int(BarContext::kIconSize));
    if (m_userIcon.empty())
        m_userIcon = ui::themeIconPath("default-user", int(BarContext::kIconSize));
    if (!m_entries.empty())
        m_entries.front()->setAvatar(m_userIcon);
}

void MenuBarView::barChanged()
{
    for (Entry *entry : m_entries)
        entry->updateLook();
    // The application's name changes weight, and with it width.
    requestLayout();
}

void MenuBarView::rebuildEntries()
{
    const std::vector<MenuBarModel::Row> &rows = m_model->rows();
    // The user's entry stays put; the application's are swapped out and in.
    size_t keep = 0;
    while (keep < m_entries.size() && keep < rows.size() && m_entries[keep]->row() == rows[keep]
           && rows[keep].kind == MenuBarModel::Kind::User)
        ++keep;
    while (m_entries.size() > keep) {
        Entry *entry = m_entries.back();
        m_entries.pop_back();
        std::shared_ptr<ui::View> retired(takeChild(entry));
        // Its own pointer handling may be what is running.
        core::mainLoop()->post([retired] {});
    }
    for (size_t i = keep; i < rows.size(); ++i) {
        Entry *entry = addChild<Entry>(m_host.bar, rows[i]);
        entry->setName(rows[i].kind == MenuBarModel::Kind::User ? "user" : "entry-" + rows[i].title);
        const int index = int(i);
        entry->onClicked = [this, index](const ui::PointerEvent &event) {
            if (event.button != ui::MouseButton::Left)
                return;
            if (m_currentIndex == index)
                close();
            else
                open(index);
        };
        // While one menu is open, moving onto another entry switches to it.
        entry->onEntered = [this, index] {
            if (m_currentIndex != -1 && m_currentIndex != index)
                open(index);
        };
        m_entries.push_back(entry);
    }
    if (!m_entries.empty())
        m_entries.front()->setAvatar(m_userIcon);

    // The open menu stays while its entry does.
    if (m_currentIndex >= 0) {
        if (size_t(m_currentIndex) >= rows.size() || m_model->menuAt(size_t(m_currentIndex)) != m_openMenu
            || !m_openMenu || !m_openMenu->isOpen())
            close();
    }
    for (size_t i = 0; i < m_entries.size(); ++i)
        m_entries[i]->setChecked(int(i) == m_currentIndex);
    requestLayout();
}

void MenuBarView::layout()
{
    double x = 0;
    const double height = geometry().height;
    for (Entry *entry : m_entries) {
        const double width = entry->preferredWidth();
        entry->setGeometry({x, 0, width, height});
        x += width;
    }
}

void MenuBarView::open(int index, bool keyboard)
{
    if (index < 0 || size_t(index) >= m_entries.size())
        return;
    Entry *entry = m_entries[size_t(index)];
    ui::Window *window = this->window();
    if (!window)
        return;

    if (m_host.closePanels)
        m_host.closePanels();
    if (m_host.closeTrayMenu)
        m_host.closeTrayMenu();

    ui::Menu *menu = m_model->menuAt(size_t(index));
    // An entry can be a plain action instead of a menu.
    if (!menu) {
        close();
        m_model->trigger(size_t(index));
        return;
    }

    // Closing the previous menu here is part of the switch, not a dismissal.
    m_switching = true;
    if (m_openMenu && m_openMenu != menu)
        m_openMenu->close();
    m_switching = false;

    m_currentIndex = index;
    m_openMenu = menu;
    for (size_t i = 0; i < m_entries.size(); ++i)
        m_entries[i]->setChecked(int(i) == m_currentIndex);

    // Its top-left at the plate's left edge, the popup margin under the bar.
    const ui::PointF at = entry->mapToWindow({entry->leftInset(), entry->geometry().height + kPopupMargin});
    menu->setAvailableGeometry(m_host.availableGeometry ? std::optional<ui::RectF>(m_host.availableGeometry())
                                                        : std::nullopt);
    // Left in the menu and Right on a row without a submenu walk the bar.
    menu->onNavigateLeft = [this] { step(-1); };
    menu->onNavigateRight = [this] { step(1); };
    m_switching = true;
    // A menu opened again starts afresh.
    menu->close();
    m_switching = false;
    // From the keyboard or not, no row is lit until a key or the pointer
    // picks one: the pointer is on the bar.
    (void)keyboard;
    menu->popupAt(window, at);
    if (!menu->isOpen())
        onMenuClosed();
}

void MenuBarView::close()
{
    ui::Menu *menu = m_openMenu;
    m_currentIndex = -1;
    m_openMenu = nullptr;
    for (Entry *entry : m_entries)
        entry->setChecked(false);
    if (menu) {
        m_switching = true;
        menu->close();
        m_switching = false;
    }
}

void MenuBarView::onMenuClosed()
{
    if (m_switching)
        return;
    if (m_openMenu && m_openMenu->isOpen())
        return;
    close();
}

void MenuBarView::step(int step)
{
    const int count = int(m_entries.size());
    if (count == 0)
        return;
    open((m_currentIndex + step + count) % count, true);
}

} // namespace Lingmo::Shell
