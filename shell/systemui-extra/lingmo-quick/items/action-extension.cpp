/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "action-extension.h"

#include <QDebug>

namespace LingmoUIQuick {

class ActionExtensionPrivate : public QObject
{
    Q_OBJECT
public:
    explicit ActionExtensionPrivate(QObject *parent, QAction *action);
    ~ActionExtensionPrivate() override;

    static void appendAction(QQmlListProperty<QAction> *list, QAction *action);
    static int actionCount(QQmlListProperty<QAction> *list);
    static QAction *actionAt(QQmlListProperty<QAction> *list, int index);
    static void clearAction(QQmlListProperty<QAction> *list);
    static void replaceAction(QQmlListProperty<QAction> *list, int index, QAction *action);
    static void removeLastAction(QQmlListProperty<QAction> *list);
    static void resetMenu(ActionExtensionPrivate *aep);

    QString iconName;
    QList<QAction*> subActions;
    QAction *rawAction {nullptr};
    std::unique_ptr<QMenu> subMenu;
};

ActionExtensionPrivate::ActionExtensionPrivate(QObject *parent, QAction *action) : QObject(parent), rawAction(action)
{

}

ActionExtensionPrivate::~ActionExtensionPrivate()
{
    subMenu.reset();
}

void ActionExtensionPrivate::appendAction(QQmlListProperty<QAction> *list, QAction *action)
{
    if (!action) {
        return;
    }

    auto aep = qobject_cast<ActionExtensionPrivate *>(list->object);
    if (!aep->subActions.contains(action)) {
        aep->subActions.append(action);

        if (!aep->subMenu) {
            aep->subMenu.reset(new QMenu);
            aep->subMenu->setAttribute(Qt::WA_DeleteOnClose, false);
            aep->rawAction->setMenu(aep->subMenu.get());
        }

        aep->subMenu->addAction(action);
    }
}

int ActionExtensionPrivate::actionCount(QQmlListProperty<QAction> *list)
{
    auto aep = qobject_cast<ActionExtensionPrivate *>(list->object);
    return aep->subActions.size();
}

QAction *ActionExtensionPrivate::actionAt(QQmlListProperty<QAction> *list, int index)
{
    auto aep = qobject_cast<ActionExtensionPrivate *>(list->object);
    if (index < 0 || index >= aep->subActions.size()) {
        return nullptr;
    }

    return aep->subActions.at(index);
}

void ActionExtensionPrivate::clearAction(QQmlListProperty<QAction> *list)
{
    auto aep = qobject_cast<ActionExtensionPrivate *>(list->object);
    aep->subActions.clear();
    resetMenu(aep);
}

void ActionExtensionPrivate::replaceAction(QQmlListProperty<QAction> *list, int index, QAction *action)
{
    if (!action) {
        return;
    }

    auto aep = qobject_cast<ActionExtensionPrivate *>(list->object);
    if (index < 0 || index >= aep->subActions.size()) {
        return;
    }

    aep->subActions.replace(index, action);
    aep->subMenu->clear();
    aep->subMenu->addActions(aep->subActions);
}

void ActionExtensionPrivate::removeLastAction(QQmlListProperty<QAction> *list)
{
    auto aep = qobject_cast<ActionExtensionPrivate *>(list->object);
    if (aep->subActions.isEmpty()) {
        return;
    }

    aep->subMenu->removeAction(aep->subActions.takeLast());
    if (aep->subActions.isEmpty()) {
        resetMenu(aep);
    }
}

void ActionExtensionPrivate::resetMenu(ActionExtensionPrivate *aep)
{
    aep->rawAction->setMenu(nullptr);
    aep->subMenu.reset();
}

// ====== ActionExtension ====== //
ActionExtension::ActionExtension(QObject *parent)
    : QObject(parent), d(new ActionExtensionPrivate(this, qobject_cast<QAction *>(parent)))
{

}

ActionExtension::~ActionExtension()
{

}

QString ActionExtension::iconName() const
{
    return d->iconName;
}

void ActionExtension::setIconName(const QString &name)
{
    if (name == d->iconName) {
        return;
    }

    d->iconName = name;
    d->rawAction->setIcon(QIcon::fromTheme(name));
    Q_EMIT iconNameChanged();
}

bool ActionExtension::isSeparator() const
{
    return d->rawAction->isSeparator();
}

void ActionExtension::setSeparator(bool isSeparator)
{
    if (isSeparator == d->rawAction->isSeparator()) {
        return;
    }

    d->rawAction->setSeparator(isSeparator);
    Q_EMIT isSeparatorChanged();
}

QMenu *ActionExtension::menu() const
{
    return d->rawAction->menu();
}

void ActionExtension::setMenu(QMenu *menu)
{
    if (menu == d->rawAction->menu()) {
        return;
    }

    d->rawAction->setMenu(menu);
    d->subMenu.reset(menu);
    Q_EMIT menuChanged();
}

QQmlListProperty<QAction> ActionExtension::subActions()
{
    return {d,
            &d->subActions,
            &ActionExtensionPrivate::appendAction,
            &ActionExtensionPrivate::actionCount,
            &ActionExtensionPrivate::actionAt,
            &ActionExtensionPrivate::clearAction,
            &ActionExtensionPrivate::replaceAction,
            &ActionExtensionPrivate::removeLastAction
    };
}

} // LingmoUIQuick

#include "action-extension.moc"
