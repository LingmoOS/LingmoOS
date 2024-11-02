/*
 *
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */
#ifndef VOLUMEMANAGER_H
#define VOLUMEMANAGER_H

#undef slots
#undef signals
#undef emit

#include <gio/gio.h>
#include <gio/gunixmounts.h>
#include <QObject>
#include <QStorageInfo>
#include <QMultiMap>
#include <QMutex>
#include <QVector>

class Volume
{
public:
    Volume();
    Volume(const QString& device);
    QString device();
    QStringList duplicatesMountPoints() const;
    QMap<QString, QString> subVolumes() const;
    QStringList mountPoints() const;

    void setDevice(const QString& device);
    void addMountPoint(const QString& mountPoint, const QString& subVolume = QString());
private:
    QString m_device;
    QMap<QString, QString> m_mountPoints; //挂载点->子卷路径
};

class VolumeManager : public QObject
{
    Q_OBJECT
public:
    static VolumeManager *self();
    ~VolumeManager();
    QVector<QStringList> getDuplicates();
    QVector<Volume> volumesHaveSubVolumes();
    QVector<Volume> volumes();
    void refresh();
Q_SIGNALS:
    void VolumeDataUpdated();

private:
    explicit VolumeManager(QObject *parent = nullptr);
    static void mountAddCallback(GVolumeMonitor *monitor, GMount *gmount, VolumeManager *pThis);
    static void mountRemoveCallback(GVolumeMonitor *monitor, GMount *gmount, VolumeManager *pThis);
    GVolumeMonitor* m_volumeMonitor = nullptr;
    quint64 m_mountAddHandle;
    quint64 m_mountRemoveHandle;
    QMap<QString, Volume> m_volumes; //device -> Volume
    QMutex m_mutex;
};


#endif // VOLUMEMANAGER_H
