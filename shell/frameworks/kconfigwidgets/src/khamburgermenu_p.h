/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KHamburgerMenu_P_H
#define KHamburgerMenu_P_H

#include "khamburgermenu.h"

#include <QWidgetAction>

#include <QPointer>
#include <QScopedPointer>

#include <forward_list>
#include <unordered_set>

class ListenerContainer;

class QMenu;
class QMenuBar;
class QToolButton;

/**
 * The private class of KHamburgerMenu used for the PIMPL idiom.
 * \internal
 */
class KHamburgerMenuPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KHamburgerMenu)

public:
    explicit KHamburgerMenuPrivate(KHamburgerMenu *qq);

    ~KHamburgerMenuPrivate() override;

    /** @see KHamburgerMenu::setMenuBar() */
    void setMenuBar(QMenuBar *menuBar);

    /** @see KHamburgerMenu::setMenuBar() */
    QMenuBar *menuBar() const;

    /** @see KHamburgerMenu::setMenuBarAdvertised() */
    void setMenuBarAdvertised(bool advertise);

    /** @see KHamburgerMenu::setMenuBarAdvertised() */
    bool menuBarAdvertised() const;

    /** @see KHamburgerMenu::setShowMenuBarAction() */
    void setShowMenuBarAction(QAction *showMenuBarAction);

    /** @see KHamburgerMenu::insertIntoMenuBefore() */
    void insertIntoMenuBefore(QMenu *menu, QAction *before);

    /** @see KHamburgerMenu::hideActionsOf() */
    void hideActionsOf(QWidget *widget);

    /** @see KHamburgerMenu::showActionsOf() */
    void showActionsOf(QWidget *widget);

    /** @see KHamburgerMenu::createWidget() */
    QWidget *createWidget(QWidget *parent);

    /**
     * This method only returns exclusive actions. The idea is to remove any @p nonExclusives
     * from @p from and all of its sub-menus. This means a copy of @p from is created when
     * necessary that has a menu() that only contains the exclusive actions from @p from.
     * @param from          the action this method extracts the exclusive actions from.
     * @param parent        the widget that is to become the parent if a copy of @p from needs
     *                      to be created.
     * @param nonExclusives the actions which will not be anywhere within the returned action.
     * @return either nullptr, @p from unchanged or a copy of @p from without the @p nonExclusives.
     *         In the last case, the caller gets ownership of this new copy with parent @p parent.
     */
    QAction *actionWithExclusivesFrom(QAction *from, QWidget *parent, std::unordered_set<const QAction *> &nonExclusives) const;

    /**
     * @return a new menu with all actions from KHamburgerMenu::menu() which aren't
     * exempted from being displayed (@see hideActionsOf()).
     * Next adds the help menu.
     * At last adds a special sub-menu by calling newMenuBarAdvertisementMenu() if this step
     * was not explicitly set to be skipped (@see KHamburgerMenu::setMenuBarAdvertised()).
     */
    std::unique_ptr<QMenu> newMenu();

    /**
     * @return a special sub-menu that advertises actions of the menu bar which would otherwise
     * not be visible or discoverable for the user
     * @see KHamburgerMenu::setMenuBarAdvertised()
     */
    std::unique_ptr<QMenu> newMenuBarAdvertisementMenu(std::unordered_set<const QAction *> &visibleActions);

    /** @see resetMenu() */
    inline void notifyMenuResetNeeded()
    {
        m_menuResetNeeded = true;
    }

    /**
     * Does nothing if m_menuResetNeeded is false.
     * Otherwise deletes m_actualMenu and creates a newMenu() in its place. This new menu
     * is then set to be used whenever the hamburger menu is opened.
     * @see newMenu()
     */
    void resetMenu();

    /**
     * Sets the correct visibility for KHamburgerMenu buttons based on the visibility of the
     * menu bar (@see setMenuBar()).
     * Also sets the correct visibility of the menu item (@see addToMenu()) based on the visibility
     * of the menu bar and of the KHamburgerMenu buttons.
     */
    void updateVisibility();

protected:
    /**
     * Makes the KHamburgerMenu buttons change style just like other toolbuttons would
     * when their associated action changes.
     */
    void slotActionChanged();

    /**
     * When m_menuBar->isVisible(): Opens the first menu of the menu bar.
     * Otherwise it acts the same way as clicking on a visible KHamburgerMenu button.
     * If no KHamburgerMenu button is visible, open its menu anyway.
     *
     * @note The action triggered signal is not emitted when a normal button that contains a menu
     *       is pressed. So this slot will effectively only be called by keyboard shortcut.
     */
    void slotActionTriggered();

    /**
     * Updates the style of @p hamburgerMenuButton based on its parent's style and q->priority().
     */
    void updateButtonStyle(QToolButton *hamburgerMenuButton) const;

public:
    KHamburgerMenu *const q_ptr;

protected:
    /** @see newMenu(). Do not confuse this menu with QAction::menu(). */
    std::unique_ptr<QMenu> m_actualMenu;
    /** @see KHamburgerMenu::setMenuBarAdvertised() */
    bool m_advertiseMenuBar = true;
    /** @see newMenuBarAdvertisementMenu() */
    std::unique_ptr<QMenu> m_menuBarAdvertisementMenu;
    /** @see KHamburgerMenu::hideActionsOf() */
    std::forward_list<QPointer<const QWidget>> m_widgetsWithActionsToBeHidden;
    /** The menu that was used as a base when newMenu() was last called. With this we
     * make sure to reset the m_actualMenu if the q->menu() has been changed or replaced. */
    QPointer<QMenu> m_lastUsedMenu;
    /** Makes sure there are no redundant event listeners of the same class. */
    std::unique_ptr<ListenerContainer> m_listeners;
    /** The action that is put into QMenus to represent the KHamburgerMenu.
     * @see KHamburgerMenu::addToMenu() */
    QPointer<QAction> m_menuAction;
    /** @see KHamburgerMenu::setMenuBar() */
    QPointer<QMenuBar> m_menuBar;
    /** @see resetMenu() */
    bool m_menuResetNeeded = false;
    /** @see KHamburgerMenu::setShowMenuBarAction */
    QPointer<QAction> m_showMenuBarAction;
    /** Keeps track of changes to the "Show Menubar" button text. */
    QString m_showMenuBarText;
    QString m_showMenuBarWithAllActionsText;
    /** Identifies if the application set an icon for "Help" menu. */
    bool m_helpIconIsSet = false;
};

#endif // KHamburgerMenu_P_H
