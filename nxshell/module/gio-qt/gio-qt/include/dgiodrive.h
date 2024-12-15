// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIODRIVE_H
#define DGIODRIVE_H

#include <QObject>
#include <QSharedData>

#define DGIODRIVE_IDENTIFIER_KIND_UNIX_DEVICE "unix-device"

namespace Gio {
class Drive;
}

class DGioDrivePrivate;
class DGioDrive : public QObject, public QSharedData {
    Q_OBJECT
public:
    explicit DGioDrive(Gio::Drive *gmmDrivePtr, QObject *parent = nullptr);
    ~DGioDrive();

    QString name() const;
    QString identifier(const QString & kind = DGIODRIVE_IDENTIFIER_KIND_UNIX_DEVICE) const;
    bool hasVolumes() const;
    bool canStart() const;
    bool canStop() const;
    bool canEject() const;
    bool isMediaRemovable() const;
    bool isRemovable() const;

private:
    QScopedPointer<DGioDrivePrivate> d_ptr;
    Q_DECLARE_PRIVATE(DGioDrive)
};


#endif // DGIODRIVE_H
