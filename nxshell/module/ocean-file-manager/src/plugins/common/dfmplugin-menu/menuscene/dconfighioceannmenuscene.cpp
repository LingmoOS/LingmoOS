// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dconfighioceannmenuscene.h"
#include "private/dconfighioceannmenuscene_p.h"
#include "utils/menuhelper.h"

#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QMenu>
#include <QStringList>
#include <QApplication>
#include <QDebug>

DPMENU_USE_NAMESPACE
using namespace GlobalDConfDefines::ConfigPath;

DConfigHioceannMenuScenePrivate::DConfigHioceannMenuScenePrivate(DConfigHioceannMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
}

DConfigHioceannMenuScene::DConfigHioceannMenuScene(QObject *parent)
    : AbstractMenuScene(parent)
{
}

QString dfmplugin_menu::DConfigHioceannMenuScene::name() const
{
    return DConfigHioceannMenuCreator::name();
}

bool DConfigHioceannMenuScene::initialize(const QVariantHash &params)
{
    auto currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    if (currentDir.isValid()) {
        // extend menu scene
        if (Helper::isHioceannExtMenu(currentDir))
            disableScene();
    }

    return true;
}

void dfmplugin_menu::DConfigHioceannMenuScene::updateState(QMenu *parent)
{
    updateActionHioceann(parent);
    AbstractMenuScene::updateState(parent);
}

void DConfigHioceannMenuScene::disableScene()
{
    fmDebug() << "disable extend menu scene..";
    static const QSet<QString> extendScenes { "OemMenu", "ExtendMenu" };
    // this scene must be the sibling of extendScenes.
    if (auto parent = dynamic_cast<AbstractMenuScene *>(this->parent())) {
        auto subs = parent->subscene();
        for (auto sub : subs) {
            if (extendScenes.contains(sub->name())) {
                parent->removeSubscene(sub);
                delete sub;
            }
        }
    }
}

void DConfigHioceannMenuScene::updateActionHioceann(QMenu *parent)
{
    static const QMap<QString, QString> appKeyMap {
        { "ocean-file-manager", "dfm.menu.action.hioceann" },
        { "org.lingmo.ocean-shell", "dd.menu.action.hioceann" },
        { "ocean-select-dialog-x11", "dfd.menu.action.hioceann" },
        { "ocean-select-dialog-wayland", "dfd.menu.action.hioceann" },
        { "ocean-file-dialog", "dfd.menu.action.hioceann" },
    };

    auto hioceannActions = DConfigManager::instance()->value(kDefaultCfgPath, appKeyMap.value(qApp->applicationName())).toStringList();
    if (hioceannActions.isEmpty())
        return;

    fmDebug() << "menu: hioceann actions: " << hioceannActions;

    QList<QMenu *> menus { parent };
    do {
        auto menu = menus.takeFirst();
        auto actions = menu->actions();
        for (int i = actions.count() - 1; i >= 0; --i) {
            auto action = actions.at(i);
            const QString &id = action->property(ActionPropertyKey::kActionID).toString();
            if (!id.isEmpty() && hioceannActions.contains(id))
                action->setVisible(false);

            if (auto subMenu = action->menu())
                menus.append(subMenu);
        }
    } while (menus.count() > 0);
}

AbstractMenuScene *DConfigHioceannMenuCreator::create()
{
    return new DConfigHioceannMenuScene();
}
