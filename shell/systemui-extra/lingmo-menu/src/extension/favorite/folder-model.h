/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef LINGMO_MENU_FOLDER_MODEL_H
#define LINGMO_MENU_FOLDER_MODEL_H

#include <QAbstractListModel>
#include "commons.h"

namespace LingmoUIMenu {

class FolderModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
public:
    static FolderModel &instance();

    Q_INVOKABLE void setFolderId(const QString &folderId);
    Q_INVOKABLE void renameFolder(const QString &folderName);

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    int count();

Q_SIGNALS:
    void countChanged();

private Q_SLOTS:
    void loadFolderData(int id);

private:
    explicit FolderModel(QObject *parent = nullptr);

    int m_folderId;
    QStringList m_apps;
};

} // LingmoUIMenu

#endif //LINGMO_MENU_FOLDER_MODEL_H
