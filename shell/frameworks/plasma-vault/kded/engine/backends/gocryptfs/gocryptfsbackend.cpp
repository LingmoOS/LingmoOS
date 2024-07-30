/*
 *   SPDX-FileCopyrightText: 2020 Martino Pilia <martino.pilia (at) gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "gocryptfsbackend.h"

#include <QDir>
#include <QProcess>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KMountPoint>
#include <KSharedConfig>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>
#include <asynqt/wrappers/process.h>

#include <qregularexpression.h>
#include <singleton_p.h>

using namespace AsynQt;

namespace PlasmaVault
{
// See `man gocryptfs`, section EXIT CODES.
enum class ExitCode : int {
    Success = 0,

    // CIPHERDIR is not an emtpy directory (on "-init")
    NonEmptyCipherDir = 6,

    // MOUNTPOINT is not an empty directory
    NonEmptyMountPoint = 10,

    // Password incorrect
    WrongPassword = 12,

    // Password is empty (on "-init")
    EmptyPassword = 22,

    // Could not read gocryptfs.conf
    CannotReadConfig = 23,

    // Could not write gocryptfs.conf (on "-init" or "-password")
    CannotWriteConfig = 24,

    // fsck found errors
    FsckError = 26,
};

GocryptfsBackend::GocryptfsBackend()
{
}

GocryptfsBackend::~GocryptfsBackend()
{
}

Backend::Ptr GocryptfsBackend::instance()
{
    return singleton::instance<GocryptfsBackend>();
}

FutureResult<> GocryptfsBackend::mount(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload)
{
    QDir dir;

    const auto password = payload[KEY_PASSWORD].toString();

    if (!dir.mkpath(device.data()) || !dir.mkpath(mountPoint.data())) {
        return errorResult(Error::BackendError, i18n("Failed to create directories, check your permissions"));
    }
    removeDotDirectory(mountPoint);

    if (isInitialized(device)) {
        auto mountProcess = gocryptfs({
            device.data(), // cypher data directory
            mountPoint.data() // mount point
        });

        auto mountResult = makeFuture(mountProcess, hasProcessFinishedSuccessfully);

        // Write password
        mountProcess->write(password.toUtf8() + "\n");

        return mountResult;
    } else {
        // Initialise cipherdir
        auto initProcess = gocryptfs({
            "-init",
            device.data(),
        });

        auto initResult = makeFuture(initProcess, [this, device, mountPoint, payload](QProcess *process) {
            auto const exitCode = static_cast<ExitCode>(process->exitCode());

            switch (exitCode) {
            case ExitCode::Success:
                return AsynQt::await(mount(device, mountPoint, payload));

            case ExitCode::NonEmptyCipherDir:
                return Result<>::error(Error::BackendError, i18n("The cipher directory is not empty, cannot initialise the vault."));

            case ExitCode::EmptyPassword:
                return Result<>::error(Error::BackendError, i18n("The password is empty, cannot initialise the vault."));

            case ExitCode::CannotWriteConfig:
                return Result<>::error(Error::BackendError, i18n("Cannot write gocryptfs.conf inside cipher directory, check your permissions."));

            default:
                return Result<>::error(Error::CommandError,
                                       i18n("Unable to perform the operation (error code %1).", QString::number((int)exitCode)),
                                       process->readAllStandardOutput(),
                                       process->readAllStandardError());
            }
        });

        // Write password twice (insert and confirm)
        for (int i = 0; i < 2; ++i) {
            initProcess->write(password.toUtf8() + "\n");
        }

        return initResult;
    }
}

FutureResult<> GocryptfsBackend::validateBackend()
{
    using namespace AsynQt::operators;

    auto customCheckVersion = [](QProcess *process, const std::tuple<int, int> &requiredVersion) {
        using namespace AsynQt::operators;

        return makeFuture(process, [=](QProcess *process) {
            if (process->exitStatus() != QProcess::NormalExit) {
                return qMakePair(false, i18n("Failed to execute"));
            }

            // We don't care about the minor version for gocryptfs
            QRegularExpression versionMatcher("([0-9]+)[.]([0-9]+)");

            const auto out = process->readAllStandardOutput();
            const auto err = process->readAllStandardError();

            if (out.isEmpty() && err.isEmpty()) {
                return qMakePair(false, i18n("Unable to detect the version"));
            }

            // gocryptfs prints out several versions separated by semicolons
            // -- of itself, of go-fuse and of go
            // We just need the first
            const auto gocryptfsVersionString = (out + err).split(';').at(0);

            if (!gocryptfsVersionString.startsWith("gocryptfs")) {
                return qMakePair(false, i18n("Unable to detect the version, the version string is invalid"));
            }

            const auto matches = versionMatcher.match(gocryptfsVersionString);

            if (!matches.hasMatch()) {
                return qMakePair(false, i18n("Unable to detect the version"));
            }

            const auto matchedVersion = std::make_tuple(matches.captured(1).toInt(), matches.captured(2).toInt());

            if (matchedVersion < requiredVersion) {
                // Bad version, we need to notify the world
                return qMakePair(false,
                                 i18n("Wrong version installed. The required version is %1.%2", std::get<0>(requiredVersion), std::get<1>(requiredVersion)));
            }

            return qMakePair(true, i18n("Correct version found"));
        });
    };

    // We need to check whether all the commands are installed
    // and whether the user has permissions to run them
    return collect(customCheckVersion(gocryptfs({"--version"}), std::make_tuple(1, 8)), checkVersion(fusermount({"--version"}), std::make_tuple(2, 9, 7)))

        | transform([this](const QPair<bool, QString> &gocryptfs, const QPair<bool, QString> &fusermount) {
               bool success = gocryptfs.first && fusermount.first;
               QString message = formatMessageLine("gocryptfs", gocryptfs) + formatMessageLine("fusermount", fusermount);

               return success ? Result<>::success() : Result<>::error(Error::BackendError, message);
           });
}

bool GocryptfsBackend::isInitialized(const Device &device) const
{
    QFile gocryptfsConfig(getConfigFilePath(device));
    return gocryptfsConfig.exists();
}

QProcess *GocryptfsBackend::gocryptfs(const QStringList &arguments) const
{
    auto config = KSharedConfig::openConfig(PLASMAVAULT_CONFIG_FILE);
    KConfigGroup backendConfig(config, "GocryptfsBackend");

    return process("gocryptfs", arguments + backendConfig.readEntry("extraMountOptions", QStringList{}), {});
}

QString GocryptfsBackend::getConfigFilePath(const Device &device) const
{
    return device.data() + QStringLiteral("/gocryptfs.conf");
}

} // namespace PlasmaVault
