/*
    This file is part of KNewStuff2.
    SPDX-FileCopyrightText: 2007 Josef Spillner <spillner@kde.org>
    SPDX-FileCopyrightText: 2009 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "installation_p.h"

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTemporaryFile>
#include <QUrlQuery>

#include "karchive.h"
#include "knewstuff_version.h"
#include "qmimedatabase.h"
#include <KRandom>
#include <KShell>
#include <KTar>
#include <KZip>

#include <KPackage/Package>
#include <KPackage/PackageJob>

#include <KLocalizedString>
#include <knewstuffcore_debug.h>
#include <qstandardpaths.h>

#include "jobs/filecopyjob.h"
#include "question.h"
#ifdef Q_OS_WIN
#include <shlobj.h>
#include <windows.h>
#endif

using namespace KNSCore;

Installation::Installation(QObject *parent)
    : QObject(parent)
{
}

bool Installation::readConfig(const KConfigGroup &group, QString &errorMessage)
{
    // FIXME: add support for several categories later on
    const QString uncompression = group.readEntry("Uncompress", QStringLiteral("never"));
    if (uncompression == QLatin1String("always") || uncompression == QLatin1String("true")) {
        uncompressSetting = AlwaysUncompress;
    } else if (uncompression == QLatin1String("archive")) {
        uncompressSetting = UncompressIfArchive;
    } else if (uncompression == QLatin1String("subdir")) {
        uncompressSetting = UncompressIntoSubdir;
    } else if (uncompression == QLatin1String("kpackage")) {
        uncompressSetting = UseKPackageUncompression;
    } else if (uncompression == QLatin1String("subdir-archive")) {
        uncompressSetting = UncompressIntoSubdirIfArchive;
    } else if (uncompression == QLatin1String("never")) {
        uncompressSetting = NeverUncompress;
    } else {
        errorMessage = QStringLiteral("invalid Uncompress setting chosen, must be one of: subdir, always, archive, never, or kpackage");
        qCCritical(KNEWSTUFFCORE) << errorMessage;
        return false;
    }

    kpackageStructure = group.readEntry("KPackageStructure");
    if (uncompressSetting == UseKPackageUncompression && kpackageStructure.isEmpty()) {
        errorMessage = QStringLiteral("kpackage uncompress setting chosen, but no KPackageStructure specified");
        qCCritical(KNEWSTUFFCORE) << errorMessage;
        return false;
    }

    postInstallationCommand = group.readEntry("InstallationCommand");
    uninstallCommand = group.readEntry("UninstallCommand");
    standardResourceDirectory = group.readEntry("StandardResource");
    targetDirectory = group.readEntry("TargetDir");
    xdgTargetDirectory = group.readEntry("XdgTargetDir");

    installPath = group.readEntry("InstallPath");
    absoluteInstallPath = group.readEntry("AbsoluteInstallPath");

    if (standardResourceDirectory.isEmpty() && targetDirectory.isEmpty() && xdgTargetDirectory.isEmpty() && installPath.isEmpty()
        && absoluteInstallPath.isEmpty()) {
        qCCritical(KNEWSTUFFCORE) << "No installation target set";
        return false;
    }
    return true;
}

void Installation::install(const Entry &entry)
{
    downloadPayload(entry);
}

void Installation::downloadPayload(const KNSCore::Entry &entry)
{
    if (!entry.isValid()) {
        Q_EMIT signalInstallationFailed(i18n("Invalid item."), entry);
        return;
    }
    QUrl source = QUrl(entry.payload());

    if (!source.isValid()) {
        qCCritical(KNEWSTUFFCORE) << "The entry doesn't have a payload.";
        Q_EMIT signalInstallationFailed(i18n("Download of item failed: no download URL for \"%1\".", entry.name()), entry);
        return;
    }

    QString fileName(source.fileName());
    QTemporaryFile tempFile(QDir::tempPath() + QStringLiteral("/XXXXXX-") + fileName);
    tempFile.setAutoRemove(false);
    if (!tempFile.open()) {
        return; // ERROR
    }
    QUrl destination = QUrl::fromLocalFile(tempFile.fileName());
    qCDebug(KNEWSTUFFCORE) << "Downloading payload" << source << "to" << destination;
#ifdef Q_OS_WIN // can't write to the file if it's open, on Windows
    tempFile.close();
#endif

    // FIXME: check for validity
    FileCopyJob *job = FileCopyJob::file_copy(source, destination, -1, JobFlag::Overwrite | JobFlag::HideProgressInfo);
    connect(job, &KJob::result, this, &Installation::slotPayloadResult);

    entry_jobs[job] = entry;
}

void Installation::slotPayloadResult(KJob *job)
{
    // for some reason this slot is getting called 3 times on one job error
    if (entry_jobs.contains(job)) {
        Entry entry = entry_jobs[job];
        entry_jobs.remove(job);

        if (job->error()) {
            const QString errorMessage = i18n("Download of \"%1\" failed, error: %2", entry.name(), job->errorString());
            qCWarning(KNEWSTUFFCORE) << errorMessage;
            Q_EMIT signalInstallationFailed(errorMessage, entry);
        } else {
            FileCopyJob *fcjob = static_cast<FileCopyJob *>(job);
            qCDebug(KNEWSTUFFCORE) << "Copied to" << fcjob->destUrl();
            QMimeDatabase db;
            QMimeType mimeType = db.mimeTypeForFile(fcjob->destUrl().toLocalFile());
            if (mimeType.inherits(QStringLiteral("text/html")) || mimeType.inherits(QStringLiteral("application/x-php"))) {
                const auto error = i18n("Cannot install '%1' because it points to a web page. Click <a href='%2'>here</a> to finish the installation.",
                                        entry.name(),
                                        fcjob->srcUrl().toString());
                Q_EMIT signalInstallationFailed(error, entry);
                entry.setStatus(KNSCore::Entry::Invalid);
                Q_EMIT signalEntryChanged(entry);
                return;
            }

            Q_EMIT signalPayloadLoaded(fcjob->destUrl());
            install(entry, fcjob->destUrl().toLocalFile());
        }
    }
}

void KNSCore::Installation::install(KNSCore::Entry entry, const QString &downloadedFile)
{
    qCWarning(KNEWSTUFFCORE) << "Install:" << entry.name() << "from" << downloadedFile;
    Q_ASSERT(QFileInfo::exists(downloadedFile));

    if (entry.payload().isEmpty()) {
        qCDebug(KNEWSTUFFCORE) << "No payload associated with:" << entry.name();
        return;
    }

    // TODO Add async checksum verification

    QString targetPath = targetInstallationPath();
    QStringList installedFiles = installDownloadedFileAndUncompress(entry, downloadedFile, targetPath);

    if (uncompressionSetting() != UseKPackageUncompression) {
        if (installedFiles.isEmpty()) {
            if (entry.status() == KNSCore::Entry::Installing) {
                entry.setStatus(KNSCore::Entry::Downloadable);
            } else if (entry.status() == KNSCore::Entry::Updating) {
                entry.setStatus(KNSCore::Entry::Updateable);
            }
            Q_EMIT signalEntryChanged(entry);
            Q_EMIT signalInstallationFailed(i18n("Could not install \"%1\": file not found.", entry.name()), entry);
            return;
        }

        entry.setInstalledFiles(installedFiles);

        auto installationFinished = [this, entry]() {
            Entry newentry = entry;
            if (!newentry.updateVersion().isEmpty()) {
                newentry.setVersion(newentry.updateVersion());
            }
            if (newentry.updateReleaseDate().isValid()) {
                newentry.setReleaseDate(newentry.updateReleaseDate());
            }
            newentry.setStatus(KNSCore::Entry::Installed);
            Q_EMIT signalEntryChanged(newentry);
            Q_EMIT signalInstallationFinished(newentry);
        };
        if (!postInstallationCommand.isEmpty()) {
            QString scriptArgPath = !installedFiles.isEmpty() ? installedFiles.first() : targetPath;
            if (scriptArgPath.endsWith(QLatin1Char('*'))) {
                scriptArgPath = scriptArgPath.left(scriptArgPath.lastIndexOf(QLatin1Char('*')));
            }
            QProcess *p = runPostInstallationCommand(scriptArgPath, entry);
            connect(p, &QProcess::finished, this, [entry, installationFinished, this](int exitCode) {
                if (exitCode) {
                    Entry newEntry = entry;
                    newEntry.setStatus(KNSCore::Entry::Invalid);
                    Q_EMIT signalEntryChanged(newEntry);
                } else {
                    installationFinished();
                }
            });
        } else {
            installationFinished();
        }
    }
}

QString Installation::targetInstallationPath() const
{
    // installdir is the target directory
    QString installdir;

    const bool userScope = true;
    // installpath also contains the file name if it's a single file, otherwise equal to installdir
    int pathcounter = 0;
    // wallpaper is already managed in the case of !xdgTargetDirectory.isEmpty()
    if (!standardResourceDirectory.isEmpty() && standardResourceDirectory != QLatin1String("wallpaper")) {
        QStandardPaths::StandardLocation location = QStandardPaths::TempLocation;
        // crude translation KStandardDirs names -> QStandardPaths enum
        if (standardResourceDirectory == QLatin1String("tmp")) {
            location = QStandardPaths::TempLocation;
        } else if (standardResourceDirectory == QLatin1String("config")) {
            location = QStandardPaths::ConfigLocation;
        }

        if (userScope) {
            installdir = QStandardPaths::writableLocation(location);
        } else { // system scope
            installdir = QStandardPaths::standardLocations(location).constLast();
        }
        pathcounter++;
    }
    if (!targetDirectory.isEmpty() && targetDirectory != QLatin1String("/")) {
        if (userScope) {
            installdir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + targetDirectory + QLatin1Char('/');
        } else { // system scope
            installdir = QStandardPaths::locate(QStandardPaths::GenericDataLocation, targetDirectory, QStandardPaths::LocateDirectory) + QLatin1Char('/');
        }
        pathcounter++;
    }
    if (!xdgTargetDirectory.isEmpty() && xdgTargetDirectory != QLatin1String("/")) {
        installdir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + xdgTargetDirectory + QLatin1Char('/');
        pathcounter++;
    }
    if (!installPath.isEmpty()) {
#if defined(Q_OS_WIN)
        WCHAR wPath[MAX_PATH + 1];
        if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, wPath) == S_OK) {
            installdir = QString::fromUtf16((const char16_t *)wPath) + QLatin1Char('/') + installPath + QLatin1Char('/');
        } else {
            installdir = QDir::homePath() + QLatin1Char('/') + installPath + QLatin1Char('/');
        }
#else
        installdir = QDir::homePath() + QLatin1Char('/') + installPath + QLatin1Char('/');
#endif
        pathcounter++;
    }
    if (!absoluteInstallPath.isEmpty()) {
        installdir = absoluteInstallPath + QLatin1Char('/');
        pathcounter++;
    }

    if (pathcounter != 1) {
        qCCritical(KNEWSTUFFCORE) << "Wrong number of installation directories given.";
        return QString();
    }

    qCDebug(KNEWSTUFFCORE) << "installdir: " << installdir;

    // create the dir if it doesn't exist (QStandardPaths doesn't create it, unlike KStandardDirs!)
    QDir().mkpath(installdir);

    return installdir;
}

QStringList Installation::installDownloadedFileAndUncompress(const KNSCore::Entry &entry, const QString &payloadfile, const QString installdir)
{
    // Collect all files that were installed
    QStringList installedFiles;
    bool isarchive = true;
    UncompressionOptions uncompressionOpt = uncompressionSetting();

    // respect the uncompress flag in the knsrc
    if (uncompressionOpt == UseKPackageUncompression) {
        qCDebug(KNEWSTUFFCORE) << "Using KPackage for installation";
        auto resetEntryStatus = [this, entry]() {
            KNSCore::Entry changedEntry(entry);
            if (changedEntry.status() == KNSCore::Entry::Installing || changedEntry.status() == KNSCore::Entry::Installed) {
                changedEntry.setStatus(KNSCore::Entry::Downloadable);
            } else if (changedEntry.status() == KNSCore::Entry::Updating) {
                changedEntry.setStatus(KNSCore::Entry::Updateable);
            }
            Q_EMIT signalEntryChanged(changedEntry);
        };

        qCDebug(KNEWSTUFFCORE) << "About to attempt to install" << payloadfile << "as" << kpackageStructure;
        auto job = KPackage::PackageJob::update(kpackageStructure, payloadfile);
        connect(job, &KPackage::PackageJob::finished, this, [this, entry, payloadfile, resetEntryStatus, job]() {
            if (job->error() == KJob::NoError) {
                Entry newentry = entry;
                newentry.setInstalledFiles(QStringList{job->package().path()});
                // update version and release date to the new ones
                if (newentry.status() == KNSCore::Entry::Updating) {
                    if (!newentry.updateVersion().isEmpty()) {
                        newentry.setVersion(newentry.updateVersion());
                    }
                    if (newentry.updateReleaseDate().isValid()) {
                        newentry.setReleaseDate(newentry.updateReleaseDate());
                    }
                }
                newentry.setStatus(KNSCore::Entry::Installed);
                // We can remove the downloaded file, because we don't save its location and don't need it to uninstall the entry
                QFile::remove(payloadfile);
                Q_EMIT signalEntryChanged(newentry);
                Q_EMIT signalInstallationFinished(newentry);
                qCDebug(KNEWSTUFFCORE) << "Install job finished with no error and we now have files" << job->package().path();
            } else {
                if (job->error() == KPackage::PackageJob::JobError::NewerVersionAlreadyInstalledError) {
                    Entry newentry = entry;
                    newentry.setStatus(KNSCore::Entry::Installed);
                    newentry.setInstalledFiles(QStringList{job->package().path()});
                    // update version and release date to the new ones
                    if (!newentry.updateVersion().isEmpty()) {
                        newentry.setVersion(newentry.updateVersion());
                    }
                    if (newentry.updateReleaseDate().isValid()) {
                        newentry.setReleaseDate(newentry.updateReleaseDate());
                    }
                    Q_EMIT signalEntryChanged(newentry);
                    Q_EMIT signalInstallationFinished(newentry);
                    qCDebug(KNEWSTUFFCORE) << "Install job finished telling us this item was already installed with this version, so... let's "
                                              "just make a small fib and say we totally installed that, honest, and we now have files"
                                           << job->package().path();
                } else {
                    Q_EMIT signalInstallationFailed(i18n("Installation of %1 failed: %2", payloadfile, job->errorText()), entry);
                    resetEntryStatus();
                    qCDebug(KNEWSTUFFCORE) << "Install job finished with error state" << job->error() << "and description" << job->error();
                }
            }
        });
    } else {
        if (uncompressionOpt == AlwaysUncompress || uncompressionOpt == UncompressIntoSubdirIfArchive || uncompressionOpt == UncompressIfArchive
            || uncompressionOpt == UncompressIntoSubdir) {
            // this is weird but a decompression is not a single name, so take the path instead
            QMimeDatabase db;
            QMimeType mimeType = db.mimeTypeForFile(payloadfile);
            qCDebug(KNEWSTUFFCORE) << "Postinstallation: uncompress the file";

            // FIXME: check for overwriting, malicious archive entries (../foo) etc.
            // FIXME: KArchive should provide "safe mode" for this!
            QScopedPointer<KArchive> archive;

            if (mimeType.inherits(QStringLiteral("application/zip"))) {
                archive.reset(new KZip(payloadfile));
                // clang-format off
            } else if (mimeType.inherits(QStringLiteral("application/tar"))
                    || mimeType.inherits(QStringLiteral("application/x-tar")) // BUG 450662
                    || mimeType.inherits(QStringLiteral("application/x-gzip"))
                    || mimeType.inherits(QStringLiteral("application/x-bzip"))
                    || mimeType.inherits(QStringLiteral("application/x-lzma"))
                    || mimeType.inherits(QStringLiteral("application/x-xz"))
                    || mimeType.inherits(QStringLiteral("application/x-bzip-compressed-tar"))
                    || mimeType.inherits(QStringLiteral("application/x-compressed-tar"))) {
                // clang-format on
                archive.reset(new KTar(payloadfile));
            } else {
                qCCritical(KNEWSTUFFCORE) << "Could not determine type of archive file" << payloadfile;
                if (uncompressionOpt == AlwaysUncompress) {
                    Q_EMIT signalInstallationError(i18n("Could not determine the type of archive of the downloaded file %1", payloadfile), entry);
                    return QStringList();
                }
                isarchive = false;
            }

            if (isarchive) {
                bool success = archive->open(QIODevice::ReadOnly);
                if (!success) {
                    qCCritical(KNEWSTUFFCORE) << "Cannot open archive file" << payloadfile;
                    if (uncompressionOpt == AlwaysUncompress) {
                        Q_EMIT signalInstallationError(
                            i18n("Failed to open the archive file %1. The reported error was: %2", payloadfile, archive->errorString()),
                            entry);
                        return QStringList();
                    }
                    // otherwise, just copy the file
                    isarchive = false;
                }

                if (isarchive) {
                    const KArchiveDirectory *dir = archive->directory();
                    // if there is more than an item in the file, and we are requested to do so
                    // put contents in a subdirectory with the same name as the file
                    QString installpath;
                    const bool isSubdir =
                        (uncompressionOpt == UncompressIntoSubdir || uncompressionOpt == UncompressIntoSubdirIfArchive) && dir->entries().count() > 1;
                    if (isSubdir) {
                        installpath = installdir + QLatin1Char('/') + QFileInfo(archive->fileName()).baseName();
                    } else {
                        installpath = installdir;
                    }

                    if (dir->copyTo(installpath)) {
                        // If we extracted the subdir we want to save it using the /* notation like we would when using the "archive" option
                        // Also if we use an (un)install command we only call it once with the folder as argument and not for each file
                        if (isSubdir) {
                            installedFiles << QDir(installpath).absolutePath() + QLatin1String("/*");
                        } else {
                            installedFiles << archiveEntries(installpath, dir);
                        }
                    } else {
                        qCWarning(KNEWSTUFFCORE) << "could not install" << entry.name() << "to" << installpath;
                    }

                    archive->close();
                    QFile::remove(payloadfile);
                }
            }
        }

        qCDebug(KNEWSTUFFCORE) << "isarchive:" << isarchive;

        // some wallpapers are compressed, some aren't
        if ((!isarchive && standardResourceDirectory == QLatin1String("wallpaper"))
            || (uncompressionOpt == NeverUncompress || (uncompressionOpt == UncompressIfArchive && !isarchive)
                || (uncompressionOpt == UncompressIntoSubdirIfArchive && !isarchive))) {
            // no decompress but move to target

            /// @todo when using KIO::get the http header can be accessed and it contains a real file name.
            // FIXME: make naming convention configurable through *.knsrc? e.g. for kde-look.org image names
            QUrl source = QUrl(entry.payload());
            qCDebug(KNEWSTUFFCORE) << "installing non-archive from" << source;
            const QString installpath = QDir(installdir).filePath(source.fileName());

            qCDebug(KNEWSTUFFCORE) << "Install to file" << installpath;
            // FIXME: copy goes here (including overwrite checking)
            // FIXME: what must be done now is to update the cache *again*
            //        in order to set the new payload filename (on root tag only)
            //        - this might or might not need to take uncompression into account
            // FIXME: for updates, we might need to force an overwrite (that is, deleting before)
            QFile file(payloadfile);
            bool success = true;
            const bool update = ((entry.status() == KNSCore::Entry::Updateable) || (entry.status() == KNSCore::Entry::Updating));

            if (QFile::exists(installpath) && QDir::tempPath() != installdir) {
                if (!update) {
                    Question question(Question::ContinueCancelQuestion);
                    question.setEntry(entry);
                    question.setQuestion(i18n("This file already exists on disk (possibly due to an earlier failed download attempt). Continuing means "
                                              "overwriting it. Do you wish to overwrite the existing file?")
                                         + QStringLiteral("\n'") + installpath + QLatin1Char('\''));
                    question.setTitle(i18n("Overwrite File"));
                    if (question.ask() != Question::ContinueResponse) {
                        return QStringList();
                    }
                }
                success = QFile::remove(installpath);
            }
            if (success) {
                // remove in case it's already present and in a temporary directory, so we get to actually use the path again
                if (installpath.startsWith(QDir::tempPath())) {
                    QFile::remove(installpath);
                }
                success = file.rename(installpath);
                qCWarning(KNEWSTUFFCORE) << "move:" << file.fileName() << "to" << installpath;
                if (!success) {
                    qCWarning(KNEWSTUFFCORE) << file.errorString();
                }
            }
            if (!success) {
                Q_EMIT signalInstallationError(i18n("Unable to move the file %1 to the intended destination %2", payloadfile, installpath), entry);
                qCCritical(KNEWSTUFFCORE) << "Cannot move file" << payloadfile << "to destination" << installpath;
                return QStringList();
            }
            installedFiles << installpath;
        }
    }

    return installedFiles;
}

QProcess *Installation::runPostInstallationCommand(const QString &installPath, const KNSCore::Entry &entry)
{
    QString command(postInstallationCommand);
    QString fileArg(KShell::quoteArg(installPath));
    command.replace(QLatin1String("%f"), fileArg);

    qCDebug(KNEWSTUFFCORE) << "Run command:" << command;

    QProcess *ret = new QProcess(this);
    auto onProcessFinished = [this, command, ret, entry](int exitcode, QProcess::ExitStatus status) {
        const QString output{QString::fromLocal8Bit(ret->readAllStandardError())};
        if (status == QProcess::CrashExit) {
            QString errorMessage = i18n("The installation failed while attempting to run the command:\n%1\n\nThe returned output was:\n%2", command, output);
            Q_EMIT signalInstallationError(errorMessage, entry);
            qCCritical(KNEWSTUFFCORE) << "Process crashed with command:" << command;
        } else if (exitcode) {
            // 130 means Ctrl+C as an exit code this is interpreted by KNewStuff as cancel operation
            // and no error will be displayed to the user, BUG: 436355
            if (exitcode == 130) {
                qCCritical(KNEWSTUFFCORE) << "Command" << command << "failed was aborted by the user";
                Q_EMIT signalInstallationFinished(entry);
            } else {
                Q_EMIT signalInstallationError(
                    i18n("The installation failed with code %1 while attempting to run the command:\n%2\n\nThe returned output was:\n%3",
                         exitcode,
                         command,
                         output),
                    entry);
                qCCritical(KNEWSTUFFCORE) << "Command" << command << "failed with code" << exitcode;
            }
        }
        sender()->deleteLater();
    };
    connect(ret, &QProcess::finished, this, onProcessFinished);

    QStringList args = KShell::splitArgs(command);
    ret->setProgram(args.takeFirst());
    ret->setArguments(args);
    ret->start();
    return ret;
}

void Installation::uninstall(Entry entry)
{
    const auto deleteFilesAndMarkAsUninstalled = [entry, this]() {
        bool deletionSuccessful = true;
        const auto lst = entry.installedFiles();
        for (const QString &file : lst) {
            // This is used to delete the download location if there are no more entries
            QFileInfo info(file);
            if (info.isDir()) {
                QDir().rmdir(file);
            } else if (file.endsWith(QLatin1String("/*"))) {
                QDir dir(file.left(file.size() - 2));
                bool worked = dir.removeRecursively();
                if (!worked) {
                    qCWarning(KNEWSTUFFCORE) << "Couldn't remove" << dir.path();
                    continue;
                }
            } else {
                if (info.exists() || info.isSymLink()) {
                    bool worked = QFile::remove(file);
                    if (!worked) {
                        qWarning() << "unable to delete file " << file;
                        Q_EMIT signalInstallationFailed(
                            i18n("The removal of %1 failed, as the installed file %2 could not be automatically removed. You can attempt to manually delete "
                                 "this file, if you believe this is an error.",
                                 entry.name(),
                                 file),
                            entry);
                        // Assume that the uninstallation has failed, and reset the entry to an installed state
                        deletionSuccessful = false;
                        break;
                    }
                } else {
                    qWarning() << "unable to delete file " << file << ". file does not exist.";
                }
            }
        }
        Entry newEntry = entry;
        if (deletionSuccessful) {
            newEntry.setEntryDeleted();
        } else {
            newEntry.setStatus(KNSCore::Entry::Installed);
        }

        Q_EMIT signalEntryChanged(newEntry);
    };

    if (uncompressionSetting() == UseKPackageUncompression) {
        const auto lst = entry.installedFiles();
        if (lst.length() == 1) {
            const QString installedFile{lst.first()};

            KJob *job = KPackage::PackageJob::uninstall(kpackageStructure, installedFile);
            connect(job, &KJob::result, this, [this, installedFile, entry, job]() {
                Entry newEntry = entry;
                if (job->error() == KJob::NoError) {
                    newEntry.setEntryDeleted();
                    Q_EMIT signalEntryChanged(newEntry);
                } else {
                    Q_EMIT signalInstallationFailed(i18n("Installation of %1 failed: %2", installedFile, job->errorText()), entry);
                }
            });
        }
        deleteFilesAndMarkAsUninstalled();
    } else {
        const auto lst = entry.installedFiles();
        // If there is an uninstall script, make sure it runs without errors
        if (!uninstallCommand.isEmpty()) {
            bool validFileExisted = false;
            for (const QString &file : lst) {
                QString filePath = file;
                bool validFile = QFileInfo::exists(filePath);
                // If we have uncompressed a subdir we write <path>/* in the config, but when calling a script
                // we want to convert this to a normal path
                if (!validFile && file.endsWith(QLatin1Char('*'))) {
                    filePath = filePath.left(filePath.lastIndexOf(QLatin1Char('*')));
                    validFile = QFileInfo::exists(filePath);
                }
                if (validFile) {
                    validFileExisted = true;
                    QString fileArg(KShell::quoteArg(filePath));
                    QString command(uninstallCommand);
                    command.replace(QLatin1String("%f"), fileArg);

                    QStringList args = KShell::splitArgs(command);
                    const QString program = args.takeFirst();
                    QProcess *process = new QProcess(this);
                    process->start(program, args);
                    auto onProcessFinished = [this, command, process, entry, deleteFilesAndMarkAsUninstalled](int, QProcess::ExitStatus status) {
                        if (status == QProcess::CrashExit) {
                            const QString processOutput = QString::fromLocal8Bit(process->readAllStandardError());
                            const QString err = i18n(
                                "The uninstallation process failed to successfully run the command %1\n"
                                "The output of was: \n%2\n"
                                "If you think this is incorrect, you can continue or cancel the uninstallation process",
                                KShell::quoteArg(command),
                                processOutput);
                            Q_EMIT signalInstallationError(err, entry);
                            // Ask the user if he wants to continue, even though the script failed
                            Question question(Question::ContinueCancelQuestion);
                            question.setEntry(entry);
                            question.setQuestion(err);
                            Question::Response response = question.ask();
                            if (response == Question::CancelResponse) {
                                // Use can delete files manually
                                Entry newEntry = entry;
                                newEntry.setStatus(KNSCore::Entry::Installed);
                                Q_EMIT signalEntryChanged(newEntry);
                                return;
                            }
                        } else {
                            qCDebug(KNEWSTUFFCORE) << "Command executed successfully:" << command;
                        }
                        deleteFilesAndMarkAsUninstalled();
                    };
                    connect(process, &QProcess::finished, this, onProcessFinished);
                }
            }
            // If the entry got deleted, but the RemoveDeadEntries option was not selected this case can happen
            if (!validFileExisted) {
                deleteFilesAndMarkAsUninstalled();
            }
        } else {
            deleteFilesAndMarkAsUninstalled();
        }
    }
}

Installation::UncompressionOptions Installation::uncompressionSetting() const
{
    return uncompressSetting;
}

QStringList Installation::archiveEntries(const QString &path, const KArchiveDirectory *dir)
{
    QStringList files;
    const auto lst = dir->entries();
    for (const QString &entry : lst) {
        const auto currentEntry = dir->entry(entry);

        const QString childPath = QDir(path).filePath(entry);
        if (currentEntry->isFile()) {
            files << childPath;
        } else if (currentEntry->isDirectory()) {
            files << childPath + QStringLiteral("/*");
        }
    }
    return files;
}

#include "moc_installation_p.cpp"
