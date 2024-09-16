// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIOVOLUMEMANAGER_H
#define DGIOVOLUMEMANAGER_H

#include <QExplicitlySharedDataPointer>
#include <QObject>

class DGioMount;
class DGioVolume;
class DGioDrive;
class DGioVolumeManagerPrivate;
class DGioVolumeManager : public QObject
{
    Q_OBJECT
public:
    explicit DGioVolumeManager(QObject *parent = nullptr);
    ~DGioVolumeManager();

    static const QList<QExplicitlySharedDataPointer<DGioMount> > getMounts();
    static const QList<QExplicitlySharedDataPointer<DGioVolume> > getVolumes();
    static const QList<QExplicitlySharedDataPointer<DGioDrive> > getDrives();

Q_SIGNALS:
    void mountAdded(QExplicitlySharedDataPointer<DGioMount> mount);
    void mountRemoved(QExplicitlySharedDataPointer<DGioMount> mount);
    void mountPreRemoved(QExplicitlySharedDataPointer<DGioMount> mount);
    void mountChanged(QExplicitlySharedDataPointer<DGioMount> mount);
    void volumeAdded(QExplicitlySharedDataPointer<DGioVolume> volume);
    void volumeRemoved(QExplicitlySharedDataPointer<DGioVolume> volume);
    void volumeChanged(QExplicitlySharedDataPointer<DGioVolume> volume);
    void driveConnected(QExplicitlySharedDataPointer<DGioDrive> drive);
    void driveDisconnected(QExplicitlySharedDataPointer<DGioDrive> drive);
    void driveChanged(QExplicitlySharedDataPointer<DGioDrive> drive);

private:
    QScopedPointer<DGioVolumeManagerPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DGioVolumeManager)
};

#endif // DGIOVOLUMEMANAGER_H
