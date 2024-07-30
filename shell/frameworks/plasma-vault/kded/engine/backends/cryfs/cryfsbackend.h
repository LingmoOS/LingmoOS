/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_ENGINE_BACKENDS_CRYFS_BACKEND_H
#define PLASMAVAULT_KDED_ENGINE_BACKENDS_CRYFS_BACKEND_H

#include "../../fusebackend_p.h"

namespace PlasmaVault
{
class CryFsBackend : public FuseBackend
{
public:
    CryFsBackend();
    ~CryFsBackend() override;

    static Backend::Ptr instance();

    bool isInitialized(const Device &device) const override;

    FutureResult<> validateBackend() override;

    QString name() const override
    {
        return QStringLiteral("cryfs");
    }

protected:
    FutureResult<> mount(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) override;

private:
    QProcess *cryfs(const QStringList &arguments) const;
};

} // namespace PlasmaVault

#endif // include guard
