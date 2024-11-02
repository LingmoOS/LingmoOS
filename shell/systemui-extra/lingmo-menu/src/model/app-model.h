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

#ifndef LINGMO_MENU_APP_MODEL_H
#define LINGMO_MENU_APP_MODEL_H

#include <QVariantList>
#include <QAbstractListModel>

#include "commons.h"
#include "data-provider-manager.h"
#include "settings.h"
#include "context-menu-extension.h"

namespace LingmoUIMenu {

class AppModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit AppModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariantList getApps(int start = 0, int end = 0);

    Q_INVOKABLE int getLabelIndex(const QString &id);
    Q_INVOKABLE QVariantList folderApps(const QString &folderName);
    Q_INVOKABLE void appClicked(const int &index);
    Q_INVOKABLE void openMenu(const int &index, MenuInfo::Location location);
    Q_INVOKABLE void renameFolder(const QString &index, const QString &folderName);
    Q_INVOKABLE void addAppsToFolder(const QString &appIdA, const QString &appIdB, const QString &folderName);
    Q_INVOKABLE void addAppToFolder(const QString &appId, const QString &folderId);

public Q_SLOTS:
    void toRenameFolder(QString id);

private Q_SLOTS:
    void onPluginDataChanged(QVector<DataEntity> data, DataUpdateMode::Mode mode, quint32 index);
    void reloadPluginData();
    void onStyleChanged(const GlobalSetting::Key& key);

private:
    void resetModel(QVector<DataEntity> &data);
    void appendData(QVector<DataEntity> &data);
    void prependData(QVector<DataEntity> &data);
    void insertData(QVector<DataEntity> &data, int index);
    void updateData(QVector<DataEntity> &data);

private:
    QVector<DataEntity> m_apps;

Q_SIGNALS:
    void renameText(QString id);
};

} // LingmoUIMenu

#endif //LINGMO_MENU_APP_MODEL_H
