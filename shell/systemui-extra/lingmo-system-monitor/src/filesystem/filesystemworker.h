/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntulingmo.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef FILESYSTEMWORKER_H
#define FILESYSTEMWORKER_H

#include <QObject>
#include <QMap>
#include <QDebug>

#include "filesystemdata.h"

class FileSystemWorker : public QObject
{
    Q_OBJECT

public:
    explicit FileSystemWorker(QObject *parent = 0);
    ~FileSystemWorker();

    void removeDiskItem(const QString &devname);

    bool getDiskInfo(const QString &devname, FileSystemData& info);
    QList<FileSystemData> diskInfoList() const;
    QList<QString> diskDevNameList() const;
    void addDiskInfo(const QString &devname, FileSystemData& info);
    void updateDiskInfo(QString devname, FileSystemData& info);
    void removeDiskInfo(const QString &devname);
    bool isDeviceContains(const QString &devname);

public slots:
    void onFileSystemListChanged();

private:
    QMap<QString, FileSystemData> m_diskInfoList;
};

#endif // FILESYSTEMWORKER_H

