/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2023 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorschememenu.h"

#include <KActionMenu>
#include <KColorSchemeManager>
#include <KColorSchemeModel>
#include <KLocalizedString>

#include <QActionGroup>
#include <QIcon>
#include <QMenu>

constexpr int defaultSchemeRow = 0;

KActionMenu *KColorSchemeMenu::createMenu(KColorSchemeManager *manager, QObject *parent)
{
    // Be careful here when connecting to signals. The menu can outlive the manager
    KActionMenu *menu = new KActionMenu(QIcon::fromTheme(QStringLiteral("preferences-desktop-color")), i18n("Color Scheme"), parent);
    QActionGroup *group = new QActionGroup(menu);
    QObject::connect(group, &QActionGroup::triggered, manager, [manager](QAction *action) {
        const QString schemePath = action->data().toString();
        if (schemePath.isEmpty()) {
            // Reset to default
            manager->activateScheme(QModelIndex());
        } else {
            // Software linking KXmlGui gets its static KCheckAccelerators object deploy
            // KAcceleratorManager on the whole UI automatically, which adds accelerators
            // also to the texts of this menu's actions.
            // So they have to be remove here in case
            // (hoping that no original names have them also in case no accelerators were added)
            // See also KColorSchemeManager::saveSchemeToConfigFile(const QString &schemeName) const.
            const QString schemeName = KLocalizedString::removeAcceleratorMarker(action->text());
            manager->activateScheme(manager->indexForScheme(schemeName));
        }
    });
    const auto model = manager->model();
    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex index = model->index(i, 0);
        QAction *action = new QAction(index.data(KColorSchemeModel::NameRole).toString(), menu);
        action->setData(index.data(KColorSchemeModel::PathRole));
        action->setActionGroup(group);
        action->setCheckable(true);
        if (index.data(KColorSchemeModel::IdRole).toString() == manager->activeSchemeId()) {
            action->setChecked(true);
        }
        menu->addAction(action);
        QObject::connect(menu->menu(), &QMenu::aboutToShow, model, [action, index] {
            if (action->icon().isNull()) {
                action->setIcon(index.data(KColorSchemeModel::IconRole).value<QIcon>());
            }
        });
    }
    const auto groupActions = group->actions();
    if (!group->checkedAction()) {
        // If no (valid) color scheme has been selected we select the default one
        groupActions[defaultSchemeRow]->setChecked(true);
    }

    return menu;
}
