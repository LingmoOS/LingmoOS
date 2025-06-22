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

#ifndef KPASTETEXTACTION_H
#define KPASTETEXTACTION_H

#include <QAction>
#include <memory>

#include <kconfigwidgets_export.h>

class KPasteTextActionPrivate;

#if KCONFIGWIDGETS_ENABLE_DEPRECATED_SINCE(5, 39)
/**
 * @class KPasteTextAction kpastetextaction.h KPasteTextAction
 *
 * An action for pasting text from the clipboard.
 * It's useful for text handling applications as
 * when plugged into a toolbar it provides a menu
 * with the clipboard history if klipper is running.
 * If klipper is not running, the menu has only one
 * item: the current clipboard content.
 * @deprecated since 5.39
 */
class KCONFIGWIDGETS_EXPORT KPasteTextAction : public QAction
{
    Q_OBJECT
public:
    /**
     * Constructs an action with the specified parent.
     *
     * @param parent The parent of this action.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 39, "No longer use this class")
    explicit KPasteTextAction(QObject *parent);

    /**
     * Constructs an action with text; a shortcut may be specified by
     * the ampersand character (e.g. \"&amp;Option\" creates a shortcut with key \e O )
     *
     * This is the most common KAction used when you do not have a
     * corresponding icon (note that it won't appear in the current version
     * of the "Edit ToolBar" dialog, because an action needs an icon to be
     * plugged in a toolbar...).
     *
     * @param text The text that will be displayed.
     * @param parent The parent of this action.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 39, "No longer use this class")
    KPasteTextAction(const QString &text, QObject *parent);

    /**
     * Constructs an action with text and an icon; a shortcut may be specified by
     * the ampersand character (e.g. \"&amp;Option\" creates a shortcut with key \e O )
     *
     * This is the other common KAction used.  Use it when you
     * \e do have a corresponding icon.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent The parent of this action.
     */
    KCONFIGWIDGETS_DEPRECATED_VERSION(5, 39, "No longer use this class")
    KPasteTextAction(const QIcon &icon, const QString &text, QObject *parent);

    ~KPasteTextAction() override;

    /**
     * Controls the behavior of the clipboard history menu popup.
     *
     * @param mode If false and the clipboard contains a non-text object
     *             the popup menu with the clipboard history will appear
     *             immediately as the user clicks the toolbar action; if
     *             true, the action works like the standard paste action
     *             even if the current clipboard object is not text.
     *             Default value is true.
     */
    void setMixedMode(bool mode);

private:
    std::unique_ptr<KPasteTextActionPrivate> const d;
};
#endif

#endif
