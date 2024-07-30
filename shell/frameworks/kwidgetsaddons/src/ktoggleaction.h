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

#ifndef KTOGGLEACTION_H
#define KTOGGLEACTION_H

#include <QAction>
#include <memory>

#include <kwidgetsaddons_export.h>

class KGuiItem;
class KToggleActionPrivate;

/**
 *  @class KToggleAction ktoggleaction.h KToggleAction
 *
 *  @short Checkbox like action.
 *
 *  This action provides two states: checked or not.
 *
 */
class KWIDGETSADDONS_EXPORT KToggleAction : public QAction
{
    Q_OBJECT

public:
    /**
     * Constructs an action with the specified parent.
     *
     * @param parent The action's parent object.
     */
    explicit KToggleAction(QObject *parent);

    /**
     * Constructs an action with text; a shortcut may be specified by
     * the ampersand character (e.g.\ "&Option" creates a shortcut with key \e O )
     *
     * This is the most common QAction used when you do not have a
     * corresponding icon (note that it won't appear in the current version
     * of the "Edit ToolBar" dialog, because an action needs an icon to be
     * plugged in a toolbar...).
     *
     * @param text The text that will be displayed.
     * @param parent The action's parent object.
     */
    KToggleAction(const QString &text, QObject *parent);

    /**
     * Constructs an action with text and an icon; a shortcut may be specified by
     * the ampersand character (e.g.\ "&Option" creates a shortcut with key \e O )
     *
     * This is the other common QAction used.  Use it when you
     * \e do have a corresponding icon.
     *
     * @param icon The icon to display.
     * @param text The text that will be displayed.
     * @param parent The action's parent object.
     */
    KToggleAction(const QIcon &icon, const QString &text, QObject *parent);

    /**
     * Destructor
     */
    ~KToggleAction() override;

    /**
     * Defines the text (and icon, tooltip, whatsthis) that should be displayed
     * instead of the normal text, when the action is checked.
     * Note that this does <em>not</em> replace the check box in front of the
     * menu.  So you should not use it to replace the text "Show <foo>" with
     * "Hide <foo>", for example.
     *
     * If hasIcon(), the icon is kept for the 'checked state', unless
     * @p checkedItem defines an icon explicitly. Same thing for tooltip and whatsthis.
     */
    void setCheckedState(const KGuiItem &checkedItem);

protected Q_SLOTS:
    virtual void slotToggled(bool checked);

protected:
    KWIDGETSADDONS_NO_EXPORT KToggleAction(KToggleActionPrivate &dd, QObject *parent);

protected:
    std::unique_ptr<class KToggleActionPrivate> const d_ptr;

private:
    Q_DECLARE_PRIVATE(KToggleAction)
};

#endif
