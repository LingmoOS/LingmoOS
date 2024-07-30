/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "khamburgermenu.h"
#include "khamburgermenu_p.h"

#include "khamburgermenuhelpers_p.h"

#include <KLocalizedString>

#include <QMenu>
#include <QMenuBar>
#include <QStyle>
#include <QToolBar>
#include <QToolButton>

#include <algorithm>
#include <forward_list>
#include <unordered_set>

KHamburgerMenu::KHamburgerMenu(QObject *parent)
    : QWidgetAction{parent}
    , d_ptr{new KHamburgerMenuPrivate(this)}
{
}

KHamburgerMenuPrivate::KHamburgerMenuPrivate(KHamburgerMenu *qq)
    : q_ptr{qq}
    , m_listeners{new ListenerContainer(this)}
{
    q_ptr->setPriority(QAction::LowPriority);
    connect(q_ptr, &QAction::changed, this, &KHamburgerMenuPrivate::slotActionChanged);
    connect(q_ptr, &QAction::triggered, this, &KHamburgerMenuPrivate::slotActionTriggered);
}

KHamburgerMenu::~KHamburgerMenu() = default;

KHamburgerMenuPrivate::~KHamburgerMenuPrivate() = default;

void KHamburgerMenu::setMenuBar(QMenuBar *menuBar)
{
    Q_D(KHamburgerMenu);
    d->setMenuBar(menuBar);
}

void KHamburgerMenuPrivate::setMenuBar(QMenuBar *menuBar)
{
    if (m_menuBar) {
        m_menuBar->removeEventFilter(m_listeners->get<VisibilityChangesListener>());
        m_menuBar->removeEventFilter(m_listeners->get<AddOrRemoveActionListener>());
    }
    m_menuBar = menuBar;
    updateVisibility();
    if (m_menuBar) {
        m_menuBar->installEventFilter(m_listeners->get<VisibilityChangesListener>());
        m_menuBar->installEventFilter(m_listeners->get<AddOrRemoveActionListener>());
    }
}

QMenuBar *KHamburgerMenu::menuBar() const
{
    Q_D(const KHamburgerMenu);
    return d->menuBar();
}

QMenuBar *KHamburgerMenuPrivate::menuBar() const
{
    return m_menuBar;
}

void KHamburgerMenu::setMenuBarAdvertised(bool advertise)
{
    Q_D(KHamburgerMenu);
    d->setMenuBarAdvertised(advertise);
}

void KHamburgerMenuPrivate::setMenuBarAdvertised(bool advertise)
{
    m_advertiseMenuBar = advertise;
}

bool KHamburgerMenu::menuBarAdvertised() const
{
    Q_D(const KHamburgerMenu);
    return d->menuBarAdvertised();
}

bool KHamburgerMenuPrivate::menuBarAdvertised() const
{
    return m_advertiseMenuBar;
}

void KHamburgerMenu::setShowMenuBarAction(QAction *showMenuBarAction)
{
    Q_D(KHamburgerMenu);
    d->setShowMenuBarAction(showMenuBarAction);
}

void KHamburgerMenuPrivate::setShowMenuBarAction(QAction *showMenuBarAction)
{
    m_showMenuBarAction = showMenuBarAction;
}

void KHamburgerMenu::addToMenu(QMenu *menu)
{
    Q_D(KHamburgerMenu);
    d->insertIntoMenuBefore(menu, nullptr);
}

void KHamburgerMenu::insertIntoMenuBefore(QMenu *menu, QAction *before)
{
    Q_D(KHamburgerMenu);
    d->insertIntoMenuBefore(menu, before);
}

void KHamburgerMenuPrivate::insertIntoMenuBefore(QMenu *menu, QAction *before)
{
    Q_CHECK_PTR(menu);
    Q_Q(KHamburgerMenu);
    if (!m_menuAction) {
        m_menuAction = new QAction(this);
        m_menuAction->setText(i18nc("@action:inmenu General purpose menu", "&Menu"));
        m_menuAction->setIcon(q->icon());
        m_menuAction->setMenu(m_actualMenu.get());
    }
    updateVisibility(); // Sets the appropriate visibility of m_menuAction.

    menu->insertAction(before, m_menuAction);
    connect(menu, &QMenu::aboutToShow, this, [this, menu, q]() {
        if (m_menuAction->isVisible()) {
            Q_EMIT q->aboutToShowMenu();
            hideActionsOf(menu);
            resetMenu();
        }
    });
}

void KHamburgerMenu::hideActionsOf(QWidget *widget)
{
    Q_D(KHamburgerMenu);
    d->hideActionsOf(widget);
}

void KHamburgerMenuPrivate::hideActionsOf(QWidget *widget)
{
    Q_CHECK_PTR(widget);
    m_widgetsWithActionsToBeHidden.remove(nullptr);
    if (listContainsWidget(m_widgetsWithActionsToBeHidden, widget)) {
        return;
    }
    m_widgetsWithActionsToBeHidden.emplace_front(QPointer<const QWidget>(widget));
    if (QMenu *menu = qobject_cast<QMenu *>(widget)) {
        // QMenus are normally hidden. This will avoid redundancy with their actions anyways.
        menu->installEventFilter(m_listeners->get<AddOrRemoveActionListener>());
        notifyMenuResetNeeded();
    } else {
        // Only avoid redundancy when the widget is visible.
        widget->installEventFilter(m_listeners->get<VisibleActionsChangeListener>());
        if (widget->isVisible()) {
            notifyMenuResetNeeded();
        }
    }
}

void KHamburgerMenu::showActionsOf(QWidget *widget)
{
    Q_D(KHamburgerMenu);
    d->showActionsOf(widget);
}

void KHamburgerMenuPrivate::showActionsOf(QWidget *widget)
{
    Q_CHECK_PTR(widget);
    m_widgetsWithActionsToBeHidden.remove(widget);
    widget->removeEventFilter(m_listeners->get<AddOrRemoveActionListener>());
    widget->removeEventFilter(m_listeners->get<VisibleActionsChangeListener>());
    if (isWidgetActuallyVisible(widget)) {
        notifyMenuResetNeeded();
    }
}

QWidget *KHamburgerMenu::createWidget(QWidget *parent)
{
    Q_D(KHamburgerMenu);
    return d->createWidget(parent);
}

QWidget *KHamburgerMenuPrivate::createWidget(QWidget *parent)
{
    if (qobject_cast<QMenu *>(parent)) {
        qDebug(
            "Adding a KHamburgerMenu directly to a QMenu. "
            "This will look odd. Use addToMenu() instead.");
    }
    Q_Q(KHamburgerMenu);

    auto toolButton = new QToolButton(parent);
    // Set appearance
    toolButton->setDefaultAction(q);
    toolButton->setMenu(m_actualMenu.get());
    toolButton->setAttribute(Qt::WidgetAttribute::WA_CustomWhatsThis);
    toolButton->setPopupMode(QToolButton::InstantPopup);
    updateButtonStyle(toolButton);
    if (const QToolBar *toolbar = qobject_cast<QToolBar *>(parent)) {
        connect(toolbar, &QToolBar::toolButtonStyleChanged, toolButton, &QToolButton::setToolButtonStyle);
    }

    setToolButtonVisible(toolButton, !isMenuBarVisible(m_menuBar));

    // Make sure the menu will be ready in time
    toolButton->installEventFilter(m_listeners->get<ButtonPressListener>());

    hideActionsOf(parent);
    return toolButton;
}

QAction *KHamburgerMenuPrivate::actionWithExclusivesFrom(QAction *from, QWidget *parent, std::unordered_set<const QAction *> &nonExclusives) const
{
    Q_CHECK_PTR(from);
    if (nonExclusives.count(from) > 0) {
        return nullptr; // The action is non-exclusive/already visible elsewhere.
    }
    if (!from->menu() || from->menu()->isEmpty()) {
        return from; // The action is exclusive and doesn't have a menu.
    }
    std::unique_ptr<QAction> menuActionWithExclusives(new QAction(from->icon(), from->text(), parent));
    std::unique_ptr<QMenu> menuWithExclusives(new QMenu(parent));
    const auto fromMenuActions = from->menu()->actions();
    for (QAction *action : fromMenuActions) {
        QAction *actionWithExclusives = actionWithExclusivesFrom(action, menuWithExclusives.get(), nonExclusives);
        if (actionWithExclusives) {
            menuWithExclusives->addAction(actionWithExclusives);
        }
    }
    if (menuWithExclusives->isEmpty()) {
        return nullptr; // "from" has a menu that contains zero exclusive actions.
        // There is a chance that "from" is an exclusive action itself and should
        // therefore be returned instead but that is unlikely for an action that has a menu().
        // This fringe case is the only one that can't be correctly covered because we can
        // not know or assume that activating the action does something or if it is nothing
        // but a container for a menu.
    }
    menuActionWithExclusives->setMenu(menuWithExclusives.release());
    return menuActionWithExclusives.release();
}

std::unique_ptr<QMenu> KHamburgerMenuPrivate::newMenu()
{
    std::unique_ptr<QMenu> menu(new QMenu());
    Q_Q(const KHamburgerMenu);

    // Make sure we notice if the q->menu() is changed or replaced in the future.
    if (q->menu() != m_lastUsedMenu) {
        q->menu()->installEventFilter(m_listeners->get<AddOrRemoveActionListener>());

        if (m_lastUsedMenu && !listContainsWidget(m_widgetsWithActionsToBeHidden, m_lastUsedMenu)) {
            m_lastUsedMenu->removeEventFilter(m_listeners->get<AddOrRemoveActionListener>());
        }
        m_lastUsedMenu = q->menu();
    }

    if (!q->menu() && !m_menuBar) {
        return menu; // empty menu
    }

    if (!q->menu()) {
        // We have nothing else to work with so let's just add the menuBar contents.
        const auto menuBarActions = m_menuBar->actions();
        for (QAction *menuAction : menuBarActions) {
            menu->addAction(menuAction);
        }
        return menu;
    }

    // Collect actions which shouldn't be added to the menu
    std::unordered_set<const QAction *> visibleActions;
    m_widgetsWithActionsToBeHidden.remove(nullptr);
    for (const QWidget *widget : m_widgetsWithActionsToBeHidden) {
        if (qobject_cast<const QMenu *>(widget) || isWidgetActuallyVisible(widget)) {
            // avoid redundancy with menus even when they are not actually visible.
            visibleActions.reserve(visibleActions.size() + widget->actions().size());
            const auto widgetActions = widget->actions();
            for (QAction *action : widgetActions) {
                visibleActions.insert(action);
            }
        }
    }
    // Populate the menu
    const auto menuActions = q->menu()->actions();
    for (QAction *action : menuActions) {
        if (visibleActions.count(action) == 0) {
            menu->addAction(action);
            visibleActions.insert(action);
        }
    }
    // Add the last two menu actions
    if (m_menuBar) {
        connect(menu.get(), &QMenu::aboutToShow, this, [this]() {
            if (m_menuBar->actions().last()->icon().isNull()) {
                m_helpIconIsSet = false;
                m_menuBar->actions().last()->setIcon(QIcon::fromTheme(QStringLiteral("help-contents"))); // set "Help" menu icon
            } else {
                m_helpIconIsSet = true; // if the "Help" icon was set by the application, we want to leave it untouched
            }
        });
        connect(menu.get(), &QMenu::aboutToHide, this, [this]() {
            if (m_menuBar->actions().last()->icon().name() == QStringLiteral("help-contents") && !m_helpIconIsSet) {
                m_menuBar->actions().last()->setIcon(QIcon());
            }
        });
        menu->addAction(m_menuBar->actions().last()); // add "Help" menu
        visibleActions.insert(m_menuBar->actions().last());
        if (m_advertiseMenuBar) {
            menu->addSeparator();
            m_menuBarAdvertisementMenu = newMenuBarAdvertisementMenu(visibleActions);
            menu->addAction(m_menuBarAdvertisementMenu->menuAction());
        }
    }
    return menu;
}

std::unique_ptr<QMenu> KHamburgerMenuPrivate::newMenuBarAdvertisementMenu(std::unordered_set<const QAction *> &visibleActions)
{
    std::unique_ptr<QMenu> advertiseMenuBarMenu(new QMenu());
    m_showMenuBarWithAllActionsText = i18nc("@action:inmenu A menu item that advertises and enables the menubar", "Show &Menubar with All Actions");
    connect(advertiseMenuBarMenu.get(), &QMenu::aboutToShow, this, [this]() {
        if (m_showMenuBarAction) {
            m_showMenuBarText = m_showMenuBarAction->text();
            m_showMenuBarAction->setText(m_showMenuBarWithAllActionsText);
        }
    });
    connect(advertiseMenuBarMenu.get(), &QMenu::aboutToHide, this, [this]() {
        if (m_showMenuBarAction && m_showMenuBarAction->text() == m_showMenuBarWithAllActionsText) {
            m_showMenuBarAction->setText(m_showMenuBarText);
        }
    });
    if (m_showMenuBarAction) {
        advertiseMenuBarMenu->addAction(m_showMenuBarAction);
        visibleActions.insert(m_showMenuBarAction);
    }
    QAction *section = advertiseMenuBarMenu->addSeparator();

    const auto menuBarActions = m_menuBar->actions();
    for (QAction *menuAction : menuBarActions) {
        QAction *menuActionWithExclusives = actionWithExclusivesFrom(menuAction, advertiseMenuBarMenu.get(), visibleActions);
        if (menuActionWithExclusives) {
            advertiseMenuBarMenu->addAction(menuActionWithExclusives);
        }
    }
    advertiseMenuBarMenu->setIcon(QIcon::fromTheme(QStringLiteral("view-more-symbolic")));
    advertiseMenuBarMenu->setTitle(i18nc("@action:inmenu A menu text advertising its contents (more features).", "More"));
    section->setText(i18nc("@action:inmenu A section heading advertising the contents of the menu bar", "More Actions"));
    return advertiseMenuBarMenu;
}

void KHamburgerMenuPrivate::resetMenu()
{
    Q_Q(KHamburgerMenu);
    if (!m_menuResetNeeded && m_actualMenu && m_lastUsedMenu == q->menu()) {
        return;
    }
    m_menuResetNeeded = false;

    m_actualMenu = newMenu();

    const auto createdWidgets = q->createdWidgets();
    for (auto widget : createdWidgets) {
        static_cast<QToolButton *>(widget)->setMenu(m_actualMenu.get());
    }
    if (m_menuAction) {
        m_menuAction->setMenu(m_actualMenu.get());
    }
}

void KHamburgerMenuPrivate::updateVisibility()
{
    Q_Q(KHamburgerMenu);
    /** The visibility of KHamburgerMenu should be opposite to the visibility of m_menuBar.
     * Exception: We only consider a visible m_menuBar as actually visible if it is not a native
     * menu bar because native menu bars can come in many shapes and sizes which don't necessarily
     * have the same usability benefits as a traditional in-window menu bar.
     * KDE applications normally allow the user to remove any actions from their toolbar(s) anyway. */
    const bool menuBarVisible = isMenuBarVisible(m_menuBar);

    const auto createdWidgets = q->createdWidgets();
    for (auto widget : createdWidgets) {
        setToolButtonVisible(widget, !menuBarVisible);
    }

    if (!m_menuAction) {
        if (menuBarVisible && m_actualMenu) {
            m_actualMenu.release()->deleteLater(); // might as well free up some memory
        }
        return;
    }

    // The m_menuAction acts as a fallback if both the m_menuBar and all createdWidgets() on the UI
    // are currently hidden. Only then should the m_menuAction ever be visible in a QMenu.
    if (menuBarVisible || (m_menuBar && m_menuBar->isNativeMenuBar()) // See [1] below.
        || std::any_of(createdWidgets.cbegin(), createdWidgets.cend(), isWidgetActuallyVisible)) {
        m_menuAction->setVisible(false);
        return;
    }
    m_menuAction->setVisible(true);

    // [1] While the m_menuAction can be used as a normal menu by users that don't mind invoking a
    //     QMenu to access any menu actions, its primary use really is that of a fallback.
    //     Therefore the existence of a native menu bar (no matter what shape or size it might have)
    //     is enough reason for us to hide m_menuAction.
}

void KHamburgerMenuPrivate::slotActionChanged()
{
    Q_Q(KHamburgerMenu);
    const auto createdWidgets = q->createdWidgets();
    for (auto widget : createdWidgets) {
        auto toolButton = static_cast<QToolButton *>(widget);
        updateButtonStyle(toolButton);
    }
}

void KHamburgerMenuPrivate::slotActionTriggered()
{
    if (isMenuBarVisible(m_menuBar)) {
        const auto menuBarActions = m_menuBar->actions();
        for (const auto action : menuBarActions) {
            if (action->isEnabled() && !action->isSeparator()) {
                m_menuBar->setActiveAction(m_menuBar->actions().constFirst());
                return;
            }
        }
    }

    Q_Q(KHamburgerMenu);
    const auto createdWidgets = q->createdWidgets();
    for (auto widget : createdWidgets) {
        if (isWidgetActuallyVisible(widget) && widget->isActiveWindow()) {
            auto toolButton = static_cast<QToolButton *>(widget);
            m_listeners->get<ButtonPressListener>()->prepareHamburgerButtonForPress(toolButton);
            toolButton->pressed();
            return;
        }
    }

    Q_EMIT q->aboutToShowMenu();
    resetMenu();
    prepareParentlessMenuForShowing(m_actualMenu.get(), nullptr);
    m_actualMenu->popup(QCursor::pos());
}

void KHamburgerMenuPrivate::updateButtonStyle(QToolButton *hamburgerMenuButton) const
{
    Q_Q(const KHamburgerMenu);
    Qt::ToolButtonStyle buttonStyle = Qt::ToolButtonFollowStyle;
    if (QToolBar *toolbar = qobject_cast<QToolBar *>(hamburgerMenuButton->parent())) {
        buttonStyle = toolbar->toolButtonStyle();
    }
    if (buttonStyle == Qt::ToolButtonFollowStyle) {
        buttonStyle = static_cast<Qt::ToolButtonStyle>(hamburgerMenuButton->style()->styleHint(QStyle::SH_ToolButtonStyle));
    }
    if (buttonStyle == Qt::ToolButtonTextBesideIcon && q->priority() < QAction::NormalPriority) {
        hamburgerMenuButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    } else {
        hamburgerMenuButton->setToolButtonStyle(buttonStyle);
    }
}

#include "moc_khamburgermenu.cpp"
#include "moc_khamburgermenu_p.cpp"
