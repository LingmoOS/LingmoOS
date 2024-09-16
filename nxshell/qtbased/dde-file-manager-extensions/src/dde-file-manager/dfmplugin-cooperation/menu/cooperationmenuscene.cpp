// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cooperationmenuscene.h"
#include "cooperationmenuscene_p.h"

#include <dfm-base/dfm_menu_defines.h>

#include <QUrl>
#include <QProcess>

inline constexpr char kFileTransfer[] { "file-transfer" };

using namespace dfmplugin_cooperation;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *CooperationMenuCreator::create()
{
    return new CooperationMenuScene();
}

CooperationMenuScenePrivate::CooperationMenuScenePrivate(CooperationMenuScene *qq)
    : q(qq)
{
}

CooperationMenuScene::CooperationMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new CooperationMenuScenePrivate(this))
{
    d->predicateName[kFileTransfer] = tr("File transfer");
}

CooperationMenuScene::~CooperationMenuScene()
{
}

QString CooperationMenuScene::name() const
{
    return CooperationMenuCreator::name();
}

bool CooperationMenuScene::initialize(const QVariantHash &params)
{
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

    if (d->selectFiles.isEmpty() || !d->selectFiles.first().isLocalFile())
        return false;

    auto subScenes = subscene();
    setSubscene(subScenes);

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *CooperationMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<CooperationMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool CooperationMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (!d->isEmptyArea) {
        auto transAct = parent->addAction(d->predicateName.value(kFileTransfer));
        d->predicateAction[kFileTransfer] = transAct;
        transAct->setProperty(ActionPropertyKey::kActionID, kFileTransfer);
    }

    return AbstractMenuScene::create(parent);
}

void CooperationMenuScene::updateState(QMenu *parent)
{
    if (!d->isEmptyArea) {
        auto actions = parent->actions();
        parent->removeAction(d->predicateAction[kFileTransfer]);

        for (auto act : actions) {
            if (act->isSeparator())
                continue;

            auto actId = act->property(ActionPropertyKey::kActionID).toString();
            if (actId == "send-to") {
                auto subMenu = act->menu();
                if (subMenu) {
                    auto subActs = subMenu->actions();
                    subActs.insert(0, d->predicateAction[kFileTransfer]);
                    subMenu->addActions(subActs);
                    act->setVisible(true);
                    break;
                }
            }
        }
    }

    AbstractMenuScene::updateState(parent);
}

bool CooperationMenuScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (!d->predicateAction.contains(actionId))
        return AbstractMenuScene::triggered(action);

    if (actionId == kFileTransfer) {
        QStringList fileList;
        for (auto &url : d->selectFiles)
            fileList << url.toLocalFile();

        QStringList arguments;
        arguments << "-s"
                  << fileList;

        return QProcess::startDetached("dde-cooperation-transfer", arguments);
    }

    return true;
}
