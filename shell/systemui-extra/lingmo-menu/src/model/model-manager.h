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

#ifndef LINGMO_MENU_MODEL_MANAGER_H
#define LINGMO_MENU_MODEL_MANAGER_H

#include <QObject>

namespace LingmoUIMenu {

class AppModel;
class LabelModel;
class AppGroupModel;
class FolderModel;

class ModelManager : public QObject
{
    Q_OBJECT
public:
    static ModelManager *instance();
    static void registerMetaTypes();
    ~ModelManager() override = default;

    Q_INVOKABLE AppModel *getAppModel();
    Q_INVOKABLE LabelModel *getLabelModel();
    Q_INVOKABLE FolderModel *getFolderModel();
    Q_INVOKABLE AppGroupModel *getLabelGroupModel();

private:
    explicit ModelManager(QObject *parent = nullptr);

private:
    AppModel *appModel{nullptr};
    LabelModel *labelModel{nullptr};
    FolderModel *folderModel{nullptr};
    AppGroupModel *labelGroupModel{nullptr};
};

} // LingmoUIMenu

#endif //LINGMO_MENU_MODEL_MANAGER_H
