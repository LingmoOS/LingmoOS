// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIOVOLUME_H
#define DGIOVOLUME_H

#include <QObject>
#include <QSharedData>

namespace Gio {
class Volume;
}

enum DGioVolumeIdentifierType {
    VOLUME_IDENTIFIER_TYPE_LABEL,
    VOLUME_IDENTIFIER_TYPE_NFS_MOUNT,
    VOLUME_IDENTIFIER_TYPE_UNIX_DEVICE,
    VOLUME_IDENTIFIER_TYPE_UUID,
    VOLUME_IDENTIFIER_TYPE_CLASS
};
Q_ENUMS(DGioVolumeIdentifierType);

class DGioMount;
class DGioVolumePrivate;
class DGioVolume : public QObject, public QSharedData
{
    Q_OBJECT
public:
    explicit DGioVolume(Gio::Volume *gmmVolumePtr, QObject *parent = nullptr);
    ~DGioVolume();

    QString name() const;
    QString volumeClass() const;
    QString volumeMonitorName() const;
    bool canMount() const;
    bool canEject() const;
    bool shouldAutoMount() const;
    void mount() const;

    QExplicitlySharedDataPointer<DGioMount> getMount();

    QString identifier(DGioVolumeIdentifierType id) const;

private:
    QScopedPointer<DGioVolumePrivate> d_ptr;

    Q_DECLARE_PRIVATE(DGioVolume)
};

#endif // DGIOVOLUME_H
