/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2003 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KTOOLBARPOPUPACTION_H
#define KTOOLBARPOPUPACTION_H

#include <QToolButton>
#include <QWidgetAction>
#include <memory>

#include <kwidgetsaddons_export.h>

class QMenu;

/**
 * @class KToolBarPopupAction ktoolbarpopupaction.h KToolBarPopupAction
 *
 * This action is a normal action everywhere, except in a toolbar
 * where it also has a popupmenu (optionally delayed). This action is designed
 * for history actions (back/forward, undo/redo) and for any other action
 * that has more detail in a toolbar than in a menu (e.g. tool chooser
 * with "Other" leading to a dialog...).
 *
 * In contrast to KActionMenu, this action is a \e simple menuitem when plugged
 * into a menu, and has a popup only in a toolbar.
 *
 * Use cases include Back/Forward, and Undo/Redo. Simple click is what's most commonly
 * used, and enough for menus, but in toolbars there is \e also an optional popup
 * to go back N steps or undo N steps.
 *
 * @note Add actions to the popupMenu(), don't use setMenu().
 */
class KWIDGETSADDONS_EXPORT KToolBarPopupAction : public QWidgetAction
{
    Q_OBJECT
    Q_PROPERTY(PopupMode popupMode READ popupMode WRITE setPopupMode)

public:
    /**
     * The menu popup mode.
     *
     * Default is MenuButtonPopup.
     *
     * @sa QToolButton::ToolButtonPopupMode
     *
     * @since 6.0
     */
    enum PopupMode {
        NoPopup = -1, ///< Behave as if the button had no menu.
        DelayedPopup = QToolButton::DelayedPopup, ///< Clicking anywhere on the toolbar button triggers the default action.
                                                  ///< Clicking and holding the toolbar button opens the popup menu instead.
        MenuButtonPopup = QToolButton::MenuButtonPopup, ///< The toolbar button is split in a main button (triggers default action)
                                                        ///< and an arrow button (opens the popup menu).
        InstantPopup = QToolButton::InstantPopup, ///< Clicking anywhere on the toolbar button opens the popup menu.
    };
    Q_ENUM(PopupMode)

    // Not all constructors - because we need an icon, since this action only makes
    // sense when being plugged at least in a toolbar.
    /**
     * Create a KToolBarPopupAction, with a text, an icon, a
     * parent and a name.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent This action's parent.
     */
    KToolBarPopupAction(const QIcon &icon, const QString &text, QObject *parent);

    /**
     * Destroys the toolbar popup action.
     */
    ~KToolBarPopupAction() override;

    /**
     * The popup menu that is shown when clicking (some time) on the toolbar
     * button. You may want to plug items into it on creation, or connect to
     * aboutToShow for a more dynamic menu.
     *
     * @note menu() is null on this action by default and using setMenu()
     * will effectively turn this action into a regular action with a submenu.
     *
     * @since 6.0
     */
    QMenu *popupMenu() const;

    /**
     * The popup mode of the toolbar button.
     *
     * @see setPopupMode()
     *
     * @since 6.0
     */
    PopupMode popupMode() const;

    /**
     * Determines the popup mode of the toolbar button.
     * @see PopupMode
     *
     * @since 6.0
     */
    void setPopupMode(PopupMode popupMode);

    /**
     * Reimplemented from QWidgetAction.
     */
    QWidget *createWidget(QWidget *parent) override;

private:
    std::unique_ptr<class KToolBarPopupActionPrivate> const d;
};

#endif
