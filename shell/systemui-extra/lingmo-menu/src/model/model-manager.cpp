/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#include "model-manager.h"
#include "label-model.h"
#include "app-model.h"
#include "app-group-model.h"
#include "favorite/folder-model.h"
#include <QQmlEngine>

namespace LingmoUIMenu {

ModelManager *ModelManager::instance()
{
    static ModelManager modelManager;
    return &modelManager;
}

void ModelManager::registerMetaTypes()
{
    qRegisterMetaType<AppModel*>("AppModel*");
    qRegisterMetaType<LabelModel*>("LabelModel*");
    qRegisterMetaType<FolderModel*>("FolderModel*");
    qRegisterMetaType<AppGroupModel*>("AppGroupModel*");
}

ModelManager::ModelManager(QObject *parent) : QObject(parent)
{
    ModelManager::registerMetaTypes();

    appModel = new AppModel(this);
    labelModel = new LabelModel(this);
    folderModel = new FolderModel(this);
    labelGroupModel = new AppGroupModel(appModel, this);

    QQmlEngine::setObjectOwnership(appModel, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(labelModel, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(folderModel, QQmlEngine::CppOwnership);
    QQmlEngine::setObjectOwnership(labelGroupModel, QQmlEngine::CppOwnership);
}

AppModel *ModelManager::getAppModel()
{
    return appModel;
}

LabelModel *ModelManager::getLabelModel()
{
    return labelModel;
}

AppGroupModel *ModelManager::getLabelGroupModel()
{
    return labelGroupModel;
}

FolderModel *ModelManager::getFolderModel()
{
    return folderModel;
}

} // LingmoUIMenu
