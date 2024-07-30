/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "encfsbackend.h"

#include <QDir>
#include <QProcess>
#include <QRegularExpression>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMountPoint>
#include <KSharedConfig>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>
#include <asynqt/wrappers/process.h>

#include <singleton_p.h>

using namespace AsynQt;

namespace PlasmaVault
{
EncFsBackend::EncFsBackend()
{
}

EncFsBackend::~EncFsBackend()
{
}

Backend::Ptr EncFsBackend::instance()
{
    return singleton::instance<EncFsBackend>();
}

FutureResult<> EncFsBackend::mount(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload)
{
    QDir dir;

    const auto password = payload[KEY_PASSWORD].toString();

    if (!dir.mkpath(device.data()) || !dir.mkpath(mountPoint.data())) {
        return errorResult(Error::BackendError, i18n("Failed to create directories, check your permissions"));
    }
    removeDotDirectory(mountPoint);

    auto process = encfs({
        "-S", // read password from stdin
        "--standard", // If creating a file system, use the default options
        device.data(), // source directory to initialize encfs in
        mountPoint.data() // where to mount the file system
    });

    auto result = makeFuture(process, hasProcessFinishedSuccessfully);

    // Writing the password
    process->write(password.toUtf8());
    process->write("\n");

    return result;
}

FutureResult<> EncFsBackend::validateBackend()
{
    using namespace AsynQt::operators;

    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    return collect(checkVersion(encfs({"--version"}), std::make_tuple(1, 9, 1)),
                   checkVersion(encfsctl({"--version"}), std::make_tuple(1, 9, 1)),
                   checkVersion(fusermount({"--version"}), std::make_tuple(2, 9, 7)))

        | transform([this](const QPair<bool, QString> &encfs, const QPair<bool, QString> &encfsctl, const QPair<bool, QString> &fusermount) {
               bool success = encfs.first && encfsctl.first && fusermount.first;
               QString message = formatMessageLine("encfs", encfs) + formatMessageLine("encfsctl", encfsctl) + formatMessageLine("fusermount", fusermount);

               return success ? Result<>::success() : Result<>::error(Error::BackendError, message);
           });
}

bool EncFsBackend::isInitialized(const Device &device) const
{
    auto process = encfsctl({device.data()});

    process->start();
    process->waitForFinished();

    return process->exitCode() == 0;
}

QProcess *EncFsBackend::encfs(const QStringList &arguments) const
{
    auto config = KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE);
    KConfigGroup backendConfig(config, "EncfsBackend");

    return process("encfs", arguments + backendConfig.readEntry("extraMountOptions", QStringList{}), {});
}

QProcess *EncFsBackend::encfsctl(const QStringList &arguments) const
{
    return process("encfsctl", arguments, {});
}

} // namespace PlasmaVault
