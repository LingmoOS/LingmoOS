/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef FMDBUSSERVICE_H
#define FMDBUSSERVICE_H

#include <QObject>

namespace Peony {

class FMDBusService : public QObject
{
    Q_OBJECT
    //Do not modify this row.
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.FileManager1")

public:
    explicit FMDBusService(QObject *parent = nullptr);
    Q_SCRIPTABLE void ShowFolders(const QStringList& uriList, const QString& startUpId);
    Q_SCRIPTABLE void ShowItems(const QStringList& uriList, const QString& startUpId);
    Q_SCRIPTABLE void ShowItemProperties(const QStringList& uriList, const QString& startUpId);

Q_SIGNALS:
    void showFolderRequest(const QStringList& uriList, const QString& startUpId);
    void showItemsRequest(const QStringList& uriList, const QString& startUpId);
    void showItemPropertiesRequest(const QStringList& uriList, const QString& startUpId);
};

}

#endif // FMDBUSSERVICE_H
