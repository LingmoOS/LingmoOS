/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef PLASMAVAULT_KDED_ENGINE_BACKEND_P_H
#define PLASMAVAULT_KDED_ENGINE_BACKEND_P_H

#include <QDir>
#include <QList>
#include <QString>
#include <QStringList>

#include <memory>

#include "commandresult.h"
#include "vault.h"

namespace PlasmaVault
{
class Backend
{
public:
    typedef std::shared_ptr<Backend> Ptr;

    Backend();
    virtual ~Backend();

    virtual bool isInitialized(const Device &device) const = 0;
    virtual bool isOpened(const MountPoint &mountPoint) const = 0;

    virtual FutureResult<> initialize(const QString &name, const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) = 0;

    virtual FutureResult<> import(const QString &name, const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) = 0;

    virtual FutureResult<> open(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) = 0;

    virtual FutureResult<> close(const Device &device, const MountPoint &mountPoint) = 0;

    virtual FutureResult<> dismantle(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload) = 0;

    virtual FutureResult<> validateBackend() = 0;

    virtual QString name() const = 0;

    static QStringList availableBackends();

    static Ptr instance(const QString &backend);

    QString formatMessageLine(const QString &command, const QPair<bool, QString> &result) const;

    static inline bool directoryExists(const QString &path)
    {
        QDir dir(path);

        if (!dir.exists())
            return false;

        return !dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
    }
};

} // namespace PlasmaVault

#endif // include guard
