// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddplugin_videowallpaper_global.h"
#include "videowallpapermenuscene.h"
#include "wallpaperconfig.h"

#include "dfm-base/dfm_menu_defines.h"

#include <QVariantHash>
#include <QMenu>
#include <QDebug>

using namespace ddplugin_videowallpaper;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *VideoWallpaerMenuCreator::create()
{
    return new VideoWallpaperMenuScene();
}

VideoWallpaperMenuScene::VideoWallpaperMenuScene(QObject *parent)
    : AbstractMenuScene(parent)
{
    predicateName[ActionID::kVideoWallpaper] = tr("Video wallpaper");
}

QString VideoWallpaperMenuScene::name() const
{
    return VideoWallpaerMenuCreator::name();
}

bool VideoWallpaperMenuScene::initialize(const QVariantHash &params)
{
    turnOn = WpCfg->enable();
    isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();

    return isEmptyArea && onDesktop;
}

AbstractMenuScene *VideoWallpaperMenuScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    if (predicateAction.values().contains(action))
        return const_cast<VideoWallpaperMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool VideoWallpaperMenuScene::create(QMenu *parent)
{
    QAction *tempAction = parent->addAction(predicateName.value(ActionID::kVideoWallpaper));
    predicateAction[ActionID::kVideoWallpaper] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kVideoWallpaper));
    tempAction->setCheckable(true);
    tempAction->setChecked(turnOn);

    return true;
}

void VideoWallpaperMenuScene::updateState(QMenu *parent)
{
    auto actions = parent->actions();
    auto actionIter = std::find_if(actions.begin(), actions.end(), [](const QAction *ac){
        return ac->property(ActionPropertyKey::kActionID).toString() == QString("wallpaper-settings");
    });

    if (actionIter == actions.end()) {
        fmWarning() << "can not find action:" << "display-settings";
        return ;
    }

    QAction *indexAction = *actionIter;
    parent->insertAction(indexAction, predicateAction[ActionID::kVideoWallpaper]);

    AbstractMenuScene::updateState(parent);
}

bool VideoWallpaperMenuScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (predicateAction.values().contains(action)) {
        if (actionId == ActionID::kVideoWallpaper) {
            emit WpCfg->changeEnableState(action->isChecked());

            //Check for video files
            if (WpCfg->enable())
                emit WpCfg->checkResource();
        }
        return true;
    }
    return AbstractMenuScene::triggered(action);
}
