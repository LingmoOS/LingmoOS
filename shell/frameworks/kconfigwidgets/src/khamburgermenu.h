/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KHamburgerMenu_H
#define KHamburgerMenu_H

#include <kconfigwidgets_export.h>

#include <QWidgetAction>

#include <memory>

class KHamburgerMenuPrivate;

class QMenuBar;

/**
 * @class KHamburgerMenu khamburgermenu.h KHamburgerMenu
 *
 * @short A menu that substitutes a menu bar when necessary
 *
 * Allowing users to toggle the visibility of the menu bar and/or toolbars,
 * while pretty/"simple by default", can lead to various grave usability issues.
 * This class makes it easy to prevent all of them.
 *
 * Simply add a KHamburgerMenu to your UI (typically to a QToolBar) and make
 * it aware of a QMenuBar like this:
 *
 * \code
 * auto hamburgerMenu = KStandardAction::hamburgerMenu(nullptr, nullptr, actionCollection());
 * toolBar()->addAction(hamburgerMenu);
 * hamburgerMenu->hideActionsOf(toolBar());
 * // after the QMenuBar has been initialised
 * hamburgerMenu->setMenuBar(menuBar()); // line not needed if there is no QMenuBar
 * \endcode
 *
 * The added menu button will only be visible when the QMenuBar is hidden.
 * With this minimal initialisation it will contain the contents of the menu bar.
 * If a user (also) hides the container the KHamburgerMenu was added to they
 * might find themselves without a way to get a menu back. To prevent this, it is
 * recommended to add the hamburgerMenu to prominent context menus like the one
 * of your central widget preferably at the first position. Simply write:
 *
 * \code
 * hamburgerMenu->addActionToMenu(contextMenu);
 * \endcode
 *
 * The added menu will only be visible if the QMenuBar is hidden and the
 * hamburgerMenu->createdWidgets() are all invisible to the user.
 *
 * **Populating the KHamburgerMenu**
 *
 * This is easy:
 *
 * \code
 * auto menu = new QMenu(this);
 * menu->addAction(action);
 * // Add actions, separators, etc. like usual.
 * hamburgerMenu->setMenu(menu);
 * \endcode
 *
 * You probably do not want this to happen on startup. Therefore KHamburgerMenu
 * provides the signal aboutToShowMenu that you can connect to a function containing
 * the previous statements.
 *
 * \code
 * connect(hamburgerMenu, &KHamburgerMenu::aboutToShowMenu,
 *         this, &MainWindow::updateHamburgerMenu);
 * // You might want to disconnect the signal after initial creation if the contents never change.
 * \endcode
 *
 * **Deciding what to put on the hamburger menu**
 *
 * 1. Be sure to add all of the most important actions. Actions which are already
 *    visible on QToolBars, etc. will not show up in the hamburgerMenu. To manage
 *    which containers KHamburgerMenu should watch for redundancy use
 *    hideActionsOf(QWidget *) and showActionsOf(QWidget *).
 *    When a KHamburgerMenu is added to a widget, hideActionsOf(that widget)
 *    will automatically be called.
 * 2. Do not worry about adding all actions the application has to offer.
 *    The KHamburgerMenu will automatically have a section advertising excluded
 *    actions which can be found in the QMenuBar. There will also be the
 *    showMenuBarAction if you set it with setShowMenuBarAction().
 * 3. Do not worry about the help menu. KHamburgerMenu will automatically contain
 *    a help menu as the second to last item (if you set a QMenuBar which is
 *    expected to have the help menu as the last action).
 *
 * **Open menu by shortcut**
 *
 * For visually impaired users it is important to have a consistent way to open a general-purpose
 * menu. Triggering the keyboard shortcut bound to KHamburgerMenu will always open a menu.
 * - If setMenuBar() was called and that menu bar is visible, the shortcut will open the first menu
 *   of that menu bar.
 * - Otherwise, if there is a visible KHamburgerMenu button in the user interface, that menu will
 *   open.
 * - Otherwise, KHamburgerMenu's menu will open at the mouse cursor position.
 *
 * @since 5.81
 */
class KCONFIGWIDGETS_EXPORT KHamburgerMenu : public QWidgetAction
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KHamburgerMenu)

public:
    explicit KHamburgerMenu(QObject *parent);

    ~KHamburgerMenu() override;

    /**
     * Associates this KHamburgerMenu with @p menuBar. The KHamburgerMenu will from now
     * on only be visible when @p menuBar is hidden.
     * (Menu bars with QMenuBar::isNativeMenuBar() == true are considered hidden.)
     *
     * Furthermore the KHamburgerMenu will have the help menu from the @p menuBar added
     * at the end. There will also be a special sub-menu advertising actions which are
     * only available in the menu bar unless advertiseMenuBar(false) was called.
     *
     * @param menuBar   The QMenuBar the KHamburgerMenu should be associated with.
     *                  This can be set to nullptr.
     */
    void setMenuBar(QMenuBar *menuBar);

    /** @see setMenuBar() */
    QMenuBar *menuBar() const;

    /**
     * By default the KHamburgerMenu contains a special sub-menu that advertises actions
     * of the menu bar which would otherwise not be visible or discoverable for the user.
     * This method removes or re-adds that sub-menu.
     *
     * @param advertise sets whether the special sub-menu that advertises menu bar only
     *                  actions should exist.
     */
    void setMenuBarAdvertised(bool advertise);

    /** @see setMenuBarAdvertised() */
    bool menuBarAdvertised() const;

    /**
     * Adds the @p showMenuBarAction as the first item of the sub-menu which advertises actions
     * from the menu bar.
     * @see setMenuBarAdvertised()
     */
    void setShowMenuBarAction(QAction *showMenuBarAction);

    /**
     * Adds this KHamburgerMenu to @p menu.
     * It will only be visible in the menu if both the menu bar and all of this
     * QWidgetAction's createdWidgets() are invisible.
     * If it is visible in the menu, then opening the menu emits the aboutToShowMenu
     * signal.
     *
     * @param menu The menu this KHamburgerMenu is supposed to appear in.
     */
    void addToMenu(QMenu *menu);

    /**
     * Inserts this KHamburgerMenu to @p menu's list of actions, before the action @p before.
     * It will only be visible in the menu if both the menu bar and all of this
     * QWidgetAction's createdWidgets() are invisible.
     * If it is visible in the menu, then opening the menu emits the aboutToShowMenu
     * signal.
     *
     * @param before The action before which KHamburgerMenu should be inserted.
     * @param menu The menu this KHamburgerMenu is supposed to appear in.
     *
     * @see QWidget::insertAction(), QMenu::insertMenu()
     *
     * @since 5.99
     */
    void insertIntoMenuBefore(QMenu *menu, QAction *before);

    /**
     * Adds @p widget to a list of widgets that should be monitored for their actions().
     * If the widget is a QMenu, its actions will be treated as known to the user.
     * If the widget isn't a QMenu, its actions will only be treated as known to the user
     * when the widget is actually visible.
     * @param widget A widget that contains actions which should not show up in the
     *               KHamburgerMenu redundantly.
     */
    void hideActionsOf(QWidget *widget);

    /**
     * Reverses a hideActionsOf(widget) method call.
     * @see hideActionsOf()
     */
    void showActionsOf(QWidget *widget);

Q_SIGNALS:
    /**
     * This signal is emitted when a hamburger menu button is about to be pressed down.
     * It is also emitted when a QMenu that contains a visible KHamburgerMenu emits
     * QMenu::aboutToShow.
     */
    void aboutToShowMenu();

protected:
    /**
     * @see QWidgetAction::createWidget
     */
    virtual QWidget *createWidget(QWidget *parent) override;

private:
    std::unique_ptr<KHamburgerMenuPrivate> const d_ptr;
};

#endif // KHamburgerMenu_H
