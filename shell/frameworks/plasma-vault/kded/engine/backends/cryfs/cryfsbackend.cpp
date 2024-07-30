/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "cryfsbackend.h"

#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>

#include <singleton_p.h>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMountPoint>
#include <KSharedConfig>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>
#include <asynqt/wrappers/process.h>

using namespace AsynQt;

namespace PlasmaVault
{
// see: https://github.com/cryfs/cryfs/blob/develop/src/cryfs/ErrorCodes.h
enum class ExitCode : int {
    Success = 0,

    // An error happened that doesn't have an error code associated with it
    UnspecifiedError = 1,

    // The command line arguments are invalid.
    InvalidArguments = 10,

    // Couldn't load config file. Probably the password is wrong
    WrongPassword = 11,

    // Password cannot be empty
    EmptyPassword = 12,

    // The file system format is too new for this CryFS version. Please update your CryFS version.
    TooNewFilesystemFormat = 13,

    // The file system format is too old for this CryFS version. Run with --allow-filesystem-upgrade to upgrade it.
    TooOldFilesystemFormat = 14,

    // The file system uses a different cipher than the one specified on the command line using the --cipher argument.
    WrongCipher = 15,

    // Base directory doesn't exist or is inaccessible (i.e. not read or writable or not a directory)
    InaccessibleBaseDir = 16,

    // Mount directory doesn't exist or is inaccessible (i.e. not read or writable or not a directory)
    InaccessibleMountDir = 17,

    // Base directory can't be a subdirectory of the mount directory
    BaseDirInsideMountDir = 18,

    // Something's wrong with the file system.
    InvalidFilesystem = 19,
};

CryFsBackend::CryFsBackend()
{
}

CryFsBackend::~CryFsBackend()
{
}

Backend::Ptr CryFsBackend::instance()
{
    return singleton::instance<CryFsBackend>();
}

FutureResult<> CryFsBackend::mount(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload)
{
    QDir dir;

    const auto password = payload[KEY_PASSWORD].toString();
    const auto cypher = payload["cryfs-cipher"].toString();
    const auto shouldUpgrade = payload["cryfs-fs-upgrade"].toBool();

    if (!dir.mkpath(device.data()) || !dir.mkpath(mountPoint.data())) {
        return errorResult(Error::BackendError, i18n("Failed to create directories, check your permissions"));
    }
    removeDotDirectory(mountPoint);

    auto process =
        // Cypher is specified, use it to create the device
        (!cypher.isEmpty()) ? cryfs({
            "--cipher",
            cypher,
            device.data(), // source directory to initialize cryfs in
            mountPoint.data() // where to mount the file system
        })

        // Cypher is not specified, use the default, whatever it is
        : shouldUpgrade ? cryfs({device.data(), // source directory to initialize cryfs in
                                 mountPoint.data(), // where to mount the file system
                                 "--allow-filesystem-upgrade"})

                        : cryfs({
                            device.data(), // source directory to initialize cryfs in
                            mountPoint.data() // where to mount the file system
                        })

        ;

    auto result = makeFuture(process, [this, device, mountPoint, payload](QProcess *process) {
        const auto out = process->readAllStandardOutput();
        const auto err = process->readAllStandardError();

        qDebug() << "OUT: " << out;
        qDebug() << "ERR: " << err;

        const auto exitCode = (ExitCode)process->exitCode();

        auto upgradeFileSystem = [this, device, mountPoint, payload] {
            const auto upgrade =
                QMessageBox::Yes
                == QMessageBox::question(
                    nullptr,
                    i18n("Upgrade the vault?"),
                    i18n("This vault was created with an older version of cryfs and needs to be upgraded.\n\nMind that this process is irreversible and the "
                         "vault will no longer work with older versions of cryfs.\n\nDo you want to perform the upgrade now?"));

            if (!upgrade) {
                return Result<>::error(Error::BackendError, i18n("The vault needs to be upgraded before it can be opened with this version of cryfs"));
            }

            auto new_payload = payload;
            new_payload["cryfs-fs-upgrade"] = true;

            return AsynQt::await(mount(device, mountPoint, new_payload));
        };

        // clang-format off
        return
            // If we tried to mount into a non-empty location, report
            err.contains("'nonempty'") ?
                Result<>::error(Error::CommandError,
                                i18n("The mount point directory is not empty, refusing to open the vault")) :

            // If all went well, just return success
            (process->exitStatus() == QProcess::NormalExit && exitCode == ExitCode::Success) ?
                Result<>::success() :

            exitCode == ExitCode::WrongPassword ?
                Result<>::error(Error::BackendError,
                                i18n("You entered the wrong password")) :

            exitCode == ExitCode::TooNewFilesystemFormat ?
                Result<>::error(Error::BackendError,
                                i18n("The installed version of cryfs is too old to open this vault.")) :

            exitCode == ExitCode::TooOldFilesystemFormat ?
                upgradeFileSystem() :

            // otherwise just report that we failed
                Result<>::error(Error::CommandError,
                                i18n("Unable to perform the operation (error code %1).", QString::number((int)exitCode)),
                                out, err);


        });
    // clang-format on

    // Writing the password
    process->write(password.toUtf8());
    process->write("\n");

    return result;
}

FutureResult<> CryFsBackend::validateBackend()
{
    using namespace AsynQt::operators;

    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    return collect(checkVersion(cryfs({"--version"}), std::make_tuple(0, 9, 9)), checkVersion(fusermount({"--version"}), std::make_tuple(2, 9, 7)))

        | transform([this](const QPair<bool, QString> &cryfs, const QPair<bool, QString> &fusermount) {
               bool success = cryfs.first && fusermount.first;
               QString message = formatMessageLine("cryfs", cryfs) + formatMessageLine("fusermount", fusermount);

               return success ? Result<>::success() : Result<>::error(Error::BackendError, message);
           });
}

bool CryFsBackend::isInitialized(const Device &device) const
{
    QFile cryFsConfig(device.data() + QStringLiteral("/cryfs.config"));
    return cryFsConfig.exists();
}

QProcess *CryFsBackend::cryfs(const QStringList &arguments) const
{
    auto config = KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE);
    KConfigGroup backendConfig(config, "CryfsBackend");

    return process("cryfs", arguments + backendConfig.readEntry("extraMountOptions", QStringList{}), {{"CRYFS_FRONTEND", "noninteractive"}});
}

} // namespace PlasmaVault
