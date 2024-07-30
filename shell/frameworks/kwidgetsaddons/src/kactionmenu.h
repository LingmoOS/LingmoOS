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

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KACTIONMENU_H
#define KACTIONMENU_H

#include <QToolButton>
#include <QWidgetAction>
#include <memory>

#include <kwidgetsaddons_export.h>

class QMenu;

/**
 * @class KActionMenu kactionmenu.h KActionMenu
 *
 * A KActionMenu is an action that provides a sub-menu of other actions.
 *
 * Plugged in a popupmenu, it will create a submenu.
 * Plugged in a toolbar, it will create a button with a popup menu.
 *
 * This is the action used by the XMLGUI since it holds other actions.
 * If you want a submenu for selecting one tool among many (without icons), see KSelectAction.
 */
class KWIDGETSADDONS_EXPORT KActionMenu : public QWidgetAction
{
    Q_OBJECT
    Q_PROPERTY(QToolButton::ToolButtonPopupMode popupMode READ popupMode WRITE setPopupMode)

public:
    explicit KActionMenu(QObject *parent);
    KActionMenu(const QString &text, QObject *parent);
    KActionMenu(const QIcon &icon, const QString &text, QObject *parent);
    ~KActionMenu() override;

    /**
     * Adds @p action to this KActionMenu.
     * The KActionMenu does not take ownership of @p action.
     */
    void addAction(QAction *action);
    QAction *addSeparator();
    void insertAction(QAction *before, QAction *action);
    QAction *insertSeparator(QAction *before);
    void removeAction(QAction *action);

    /**
     * The currently used popup mode when plugged in a KToolBar.
     *
     * @see setPopupMode()
     *
     * @since 5.77
     */
    QToolButton::ToolButtonPopupMode popupMode() const;

    /**
     * Determines the popup mode when plugged in a KToolBar.
     *
     * Options are:
     *  - QToolButton::InstantPopup
     *    Clicking anywhere on the toolbar button opens the popup menu.
     *  - QToolButton::DelayedPopup (Default)
     *    Clicking anywhere on the toolbar button triggers the default action.
     *    Clicking and holding the toolbar button opens the popup menu instead.
     *  - QToolButton::MenuButtonPopup
     *    The toolbar button is split in a main button (triggers default action)
     *    and an arrow button (opens the popup menu).
     *
     * @see QToolButton
     *
     * @since 5.77
     */
    void setPopupMode(QToolButton::ToolButtonPopupMode popupMode);

    QWidget *createWidget(QWidget *parent) override;

private:
    std::unique_ptr<class KActionMenuPrivate> const d;
};

#endif
