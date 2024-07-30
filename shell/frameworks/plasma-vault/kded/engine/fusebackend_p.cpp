/*
 *   SPDX-FileCopyrightText: 2017 Ivan Cukic <ivan.cukic (at) kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "fusebackend_p.h"

#include <QDir>
#include <QRegularExpression>
#include <QUrl>

#include <KIO/DeleteJob>
#include <KLocalizedString>
#include <KMountPoint>

#include <algorithm>

#include <asynqt/basic/all.h>
#include <asynqt/operations/collect.h>
#include <asynqt/operations/transform.h>
#include <asynqt/wrappers/kjob.h>
#include <asynqt/wrappers/process.h>

#include "singleton_p.h"

using namespace AsynQt;

namespace PlasmaVault
{
Result<> FuseBackend::hasProcessFinishedSuccessfully(QProcess *process)
{
    const auto out = process->readAllStandardOutput();
    const auto err = process->readAllStandardError();

    return
        // If all went well, just return success
        (process->exitStatus() == QProcess::NormalExit && process->exitCode() == 0) ? Result<>::success() :

                                                                                    // If we tried to mount into a non-empty location, report
        (err.contains("'nonempty'") || err.contains("non empty"))
        ? Result<>::error(Error::CommandError, i18n("The mount point directory is not empty, refusing to open the vault"))
        :

        // If we have a message for the user, report it
        // !out.isEmpty() ?
        //     Result<>::error(Error::CommandError,
        //                     out) :

        // otherwise just report that we failed
        Result<>::error(Error::CommandError, i18n("Unable to perform the operation"), out, err);
}

void FuseBackend::removeDotDirectory(const MountPoint &mountPoint)
{
    QDir dir(mountPoint.data());
    const auto dirContents = dir.entryList(QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden | QDir::Dirs);
    if (dirContents.length() == 1 && dirContents.first() == QStringLiteral(".directory"))
        dir.remove(QStringLiteral(".directory"));
}

FuseBackend::FuseBackend()
{
}

FuseBackend::~FuseBackend()
{
}

QProcess *FuseBackend::process(const QString &executable, const QStringList &arguments, const QHash<QString, QString> &environment) const
{
    auto result = new QProcess();
    result->setProgram(executable);
    result->setArguments(arguments);

    if (environment.count() > 0) {
        auto env = result->processEnvironment();
        for (const auto &key : environment.keys()) {
            env.insert(key, environment[key]);
        }
        result->setProcessEnvironment(env);
    }

    return result;
}

QProcess *FuseBackend::fusermount(const QStringList &arguments) const
{
    return process("fusermount", arguments, {});
}

FutureResult<> FuseBackend::initialize(const QString &name, const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload)
{
    Q_UNUSED(name);

    return isInitialized(device) ? errorResult(Error::BackendError, i18n("This directory already contains encrypted data")) :

        directoryExists(device.data()) || directoryExists(mountPoint.data())
        ? errorResult(Error::BackendError, i18n("You need to select empty directories for the encrypted storage and for the mount point"))
        :

        // otherwise
        mount(device, mountPoint, payload);
}

FutureResult<> FuseBackend::import(const QString &name, const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload)
{
    Q_UNUSED(name);

    // clang-format off
    return
        !isInitialized(device) ?
            errorResult(Error::BackendError,
                        i18n("This directory doesn't contain encrypted data")) :

        !directoryExists(device.data()) || directoryExists(mountPoint.data()) ?
            errorResult(Error::BackendError,
                        i18n("You need to select an empty directory for the mount point")) :

        // otherwise
            mount(device, mountPoint, payload);
    // clang-format on
}

FutureResult<> FuseBackend::open(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload)
{
    return isOpened(mountPoint) //
        ? errorResult(Error::BackendError, i18n("Device is already open"))
        : mount(device, mountPoint, payload);
}

FutureResult<> FuseBackend::close(const Device &device, const MountPoint &mountPoint)
{
    Q_UNUSED(device);

    return !isOpened(mountPoint) ? errorResult(Error::BackendError, i18n("Device is not open")) :

                                 // otherwise
        makeFuture(fusermount({"-u", mountPoint.data()}), hasProcessFinishedSuccessfully);
}

FutureResult<> FuseBackend::dismantle(const Device &device, const MountPoint &mountPoint, const Vault::Payload &payload)
{
    // TODO:
    // mount
    // unmount
    // remove the directories
    // return Fuse::dismantle(device, mountPoint, password);

    Q_UNUSED(payload)

    // Removing the data and the mount point
    return transform(makeFuture<KJob *>(KIO::del({QUrl::fromLocalFile(device.data()), QUrl::fromLocalFile(mountPoint.data())})), [](KJob *job) {
        job->deleteLater();
        return job->error() == 0 ? Result<>::success() : Result<>::error(Error::DeletionError, job->errorString());
    });
}

QFuture<QPair<bool, QString>> FuseBackend::checkVersion(QProcess *process, const std::tuple<int, int, int> &requiredVersion) const
{
    using namespace AsynQt::operators;

    return makeFuture(process, [=](QProcess *process) {
        if (process->exitStatus() != QProcess::NormalExit) {
            return qMakePair(false, i18n("Failed to execute"));
        }

        QRegularExpression versionMatcher("([0-9]+)[.]([0-9]+)[.]([0-9]+)");

        const auto out = process->readAllStandardOutput();
        const auto err = process->readAllStandardError();
        const auto all = out + err;

        const auto matches = versionMatcher.match(all);

        if (!matches.hasMatch()) {
            return qMakePair(false, i18n("Unable to detect the version"));
        }

        const auto matchedVersion = std::make_tuple(matches.captured(1).toInt(), matches.captured(2).toInt(), matches.captured(3).toInt());

        if (matchedVersion < requiredVersion) {
            // Bad version, we need to notify the world
            return qMakePair(false,
                             i18n("Wrong version installed. The required version is %1.%2.%3",
                                  std::get<0>(requiredVersion),
                                  std::get<1>(requiredVersion),
                                  std::get<2>(requiredVersion)));
        }

        return qMakePair(true, i18n("Correct version found"));
    });
}

bool FuseBackend::isOpened(const MountPoint &mountPoint) const
{
    // warning: KMountPoint depends on /etc/mtab according to the documentation.
    KMountPoint::Ptr ptr = KMountPoint::currentMountPoints().findByPath(mountPoint.data());

    // we can not rely on ptr->realDeviceName() since it is empty,
    // KMountPoint can not get the source

    return ptr && ptr->mountPoint() == mountPoint.data();
}

} // namespace PlasmaVault
