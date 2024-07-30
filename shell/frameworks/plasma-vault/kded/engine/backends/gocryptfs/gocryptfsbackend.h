/*
 *   SPDX-FileCopyrightText: 2020 Martino Pilia <martino.pilia (at) gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_ENGINE_BACKENDS_GOCRYPTFS_BACKEND_H
#define PLASMAVAULT_KDED_ENGINE_BACKENDS_GOCRYPTFS_BACKEND_H

#include "../../fusebackend_p.h"

namespace PlasmaVault
{
class GocryptfsBackend : public FuseBackend
{
public:
    GocryptfsBackend();
    ~GocryptfsBackend() override;

    static Backend::Ptr instance();

    bool isInitialized(const Device &device) const override;

    FutureResult<> validateBackend() override;

    QString name() const override
    {
        return QStringLiteral("gocryptfs");
    }

protected:
    FutureResult<> mount(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) override;

private:
    QProcess *gocryptfs(const QStringList &arguments) const;
    QString getConfigFilePath(const Device &device) const;
};

} // namespace PlasmaVault

#endif // include guard
