/*
 *    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KCOMMANDBAR_H
#define KCOMMANDBAR_H

#include "kconfigwidgets_export.h"

#include <QFrame>
#include <memory>

/**
 * @class KCommandBar kcommandbar.h KCommandBar
 *
 * @short A hud style menu which allows listing and executing actions
 *
 * KCommandBar takes as input a list of QActions and then shows them
 * in a "list-like-view" with a filter line edit. This allows quickly
 * executing an action.
 *
 * Usage is simple. Just create a KCommandBar instance when you need it
 * and throw it away once the user is done with it. You can store it as
 * a class member as well but there is little benefit in that. Example:
 *
 * @code
 * void slotOpenCommandBar()
 * {
 *      // `this` is important, you must pass a parent widget
 *      // here. Ideally it will be your mainWindow
 *     KCommandBar *bar = new KCommandBar(this);
 *
 *     // Load actions into the command bar
 *     // These actions can be from your menus / toolbars etc
 *     QList<ActionGroup> actionGroups = ...;
 *     bar->setActions(actionGroups);
 *
 *     // Show
 *     bar->show();
 * }
 * @endcode
 *
 * @since 5.83
 * @author Waqar Ahmed <waqar.17a@gmail.com>
 */
class KCONFIGWIDGETS_EXPORT KCommandBar : public QFrame
{
    Q_OBJECT

public:
    /**
     * Represents a list of action that belong to the same group.
     * For example:
     * - All actions under the menu "File" or "Tool"
     */
    struct ActionGroup {
        QString name;
        QList<QAction *> actions;
    };

    /**
     * constructor
     *
     * @p parent is used to determine position and size of the
     * command bar. It *must* not be @c nullptr.
     */
    explicit KCommandBar(QWidget *parent);
    ~KCommandBar() override;

    /**
     * @p actions is a list of {GroupName, QAction}. Group name can be the name
     * of the component/menu where a QAction lives, for example in a menu "File -> Open File",
     * "File" should be the GroupName.
     */
    void setActions(const QList<ActionGroup> &actions);

public Q_SLOTS:
    void show();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    std::unique_ptr<class KCommandBarPrivate> const d;
};

Q_DECLARE_TYPEINFO(KCommandBar::ActionGroup, Q_RELOCATABLE_TYPE);

#endif
