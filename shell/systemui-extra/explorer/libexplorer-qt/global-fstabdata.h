/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#ifndef GLOBALFSTABDATA_H
#define GLOBALFSTABDATA_H

#include <QObject>
#include <QFileSystemWatcher>
#include "explorer-core_global.h"

namespace Peony {
class fstabData;
class PEONYCORESHARED_EXPORT GlobalFstabData : public QObject
{
    Q_OBJECT
public:
    static GlobalFstabData *getInstance();
    explicit GlobalFstabData(QObject *parent = nullptr);
    bool isMountPoints(const QString& name, const QString& mountPoint);
    bool getUuidState();
    bool isEmptyList();

private:
    const QList<fstabData*> readFstabFile();
    QList<fstabData*> m_fstabDataLists;
    QFileSystemWatcher *m_fstab_data_watcher;
    bool m_isUuid = false;
};

class PEONYCORESHARED_EXPORT fstabData : public QObject{
    Q_OBJECT
public:
    explicit fstabData(QObject *parent = nullptr);

    const QString getName();
    const QString getMountPoint();

    void setName(const QString &name);
    void setMountPoint(const QString &mountPoint);

private:
    QString m_name;
    QString m_mountPoint;
};

}

#endif // GLOBALFSTABDATA_H
