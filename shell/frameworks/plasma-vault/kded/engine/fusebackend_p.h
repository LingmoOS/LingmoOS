/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_ENGINE_BACKENDS_FUSE_BACKEND_P_H
#define PLASMAVAULT_KDED_ENGINE_BACKENDS_FUSE_BACKEND_P_H

#include "backend_p.h"

#include <QProcess>

namespace PlasmaVault
{
class FuseBackend : public Backend
{
public:
    FuseBackend();
    ~FuseBackend() override;

    bool isOpened(const MountPoint &mountPoint) const override;

    FutureResult<> initialize(const QString &name, const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) override;

    FutureResult<> import(const QString &name, const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) override;

    FutureResult<> open(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) override;

    FutureResult<> close(const Device &device, const MountPoint &mountPoint) override;

    FutureResult<> dismantle(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) override;

protected:
    virtual FutureResult<> mount(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) = 0;

    QProcess *process(const QString &command, const QStringList &args, const QHash<QString, QString> &environment) const;

    QFuture<QPair<bool, QString>> checkVersion(QProcess *process, const std::tuple<int, int, int> &requiredVersion) const;

    QProcess *fusermount(const QStringList &arguments = QStringList()) const;

    static Result<> hasProcessFinishedSuccessfully(QProcess *process);

    // dolphin has a tendency to create a .directory file, which would
    // disable our ability to mount there.
    // Check the contents of the dir (should be empty) and if the only item
    // there is the dot-directroy, delete it.
    static void removeDotDirectory(const MountPoint &mountPoint);
};

} // namespace PlasmaVault

#endif // include guard
