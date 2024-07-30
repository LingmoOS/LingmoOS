/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "private/packagejobthread_p.h"
#include "private/utils.h"

#include "config-package.h"
#include "package.h"

#include <KArchive>
#include <KLocalizedString>
#include <KTar>
#include <kzip.h>

#include "kpackage_debug.h"
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QMimeDatabase>
#include <QMimeType>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>
#include <qtemporarydir.h>

namespace KPackage
{
bool copyFolder(QString sourcePath, QString targetPath)
{
    QDir source(sourcePath);
    if (!source.exists()) {
        return false;
    }

    QDir target(targetPath);
    if (!target.exists()) {
        QString targetName = target.dirName();
        target.cdUp();
        target.mkdir(targetName);
        target = QDir(targetPath);
    }

    const auto lstEntries = source.entryList(QDir::Files);
    for (const QString &fileName : lstEntries) {
        QString sourceFilePath = sourcePath + QDir::separator() + fileName;
        QString targetFilePath = targetPath + QDir::separator() + fileName;

        if (!QFile::copy(sourceFilePath, targetFilePath)) {
            return false;
        }
    }
    const auto lstEntries2 = source.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for (const QString &subFolderName : lstEntries2) {
        QString sourceSubFolderPath = sourcePath + QDir::separator() + subFolderName;
        QString targetSubFolderPath = targetPath + QDir::separator() + subFolderName;

        if (!copyFolder(sourceSubFolderPath, targetSubFolderPath)) {
            return false;
        }
    }

    return true;
}

bool removeFolder(QString folderPath)
{
    QDir folder(folderPath);
    return folder.removeRecursively();
}

class PackageJobThreadPrivate
{
public:
    QString installPath;
    QString errorMessage;
    std::function<void()> run;
    int errorCode;
};

PackageJobThread::PackageJobThread(PackageJob::OperationType type, const QString &src, const QString &dest, const KPackage::Package &package)
    : QObject()
    , QRunnable()
{
    d = new PackageJobThreadPrivate;
    d->errorCode = KJob::NoError;
    if (type == PackageJob::Install) {
        d->run = [this, src, dest, package]() {
            install(src, dest, package);
        };
    } else if (type == PackageJob::Update) {
        d->run = [this, src, dest, package]() {
            update(src, dest, package);
        };
    } else if (type == PackageJob::Uninstall) {
        const QString packagePath = package.path();
        d->run = [this, packagePath]() {
            uninstall(packagePath);
        };

    } else {
        Q_UNREACHABLE();
    }
}

PackageJobThread::~PackageJobThread()
{
    delete d;
}

void PackageJobThread::run()
{
    Q_ASSERT(d->run);
    d->run();
}
bool PackageJobThread::install(const QString &src, const QString &dest, const Package &package)
{
    bool ok = installPackage(src, dest, package, PackageJob::Install);
    Q_EMIT installPathChanged(d->installPath);
    Q_EMIT jobThreadFinished(ok, errorCode(), d->errorMessage);
    return ok;
}

static QString resolveHandler(const QString &scheme)
{
    QString envOverride = qEnvironmentVariable("KPACKAGE_DEP_RESOLVERS_PATH");
    QStringList searchDirs;
    if (!envOverride.isEmpty()) {
        searchDirs.push_back(envOverride);
    }
    searchDirs.append(QStringLiteral(KDE_INSTALL_FULL_LIBEXECDIR_KF "/kpackagehandlers"));
    // We have to use QStandardPaths::findExecutable here to handle the .exe suffix on Windows.
    return QStandardPaths::findExecutable(scheme + QLatin1String("handler"), searchDirs);
}

bool PackageJobThread::installDependency(const QUrl &destUrl)
{
    auto handler = resolveHandler(destUrl.scheme());
    if (handler.isEmpty()) {
        return false;
    }

    QProcess process;
    process.setProgram(handler);
    process.setArguments({destUrl.toString()});
    process.setProcessChannelMode(QProcess::ForwardedChannels);
    process.start();
    process.waitForFinished();

    return process.exitCode() == 0;
}

bool PackageJobThread::installPackage(const QString &src, const QString &dest, const Package &package, PackageJob::OperationType operation)
{
    QDir root(dest);
    if (!root.exists()) {
        QDir().mkpath(dest);
        if (!root.exists()) {
            d->errorMessage = i18n("Could not create package root directory: %1", dest);
            d->errorCode = PackageJob::JobError::RootCreationError;
            // qCWarning(KPACKAGE_LOG) << "Could not create package root directory: " << dest;
            return false;
        }
    }

    QFileInfo fileInfo(src);
    if (!fileInfo.exists()) {
        d->errorMessage = i18n("No such file: %1", src);
        d->errorCode = PackageJob::JobError::PackageFileNotFoundError;
        return false;
    }

    QString path;
    QTemporaryDir tempdir;
    bool archivedPackage = false;

    if (fileInfo.isDir()) {
        // we have a directory, so let's just install what is in there
        path = src;
        // make sure we end in a slash!
        if (!path.endsWith(QLatin1Char('/'))) {
            path.append(QLatin1Char('/'));
        }
    } else {
        KArchive *archive = nullptr;
        QMimeDatabase db;
        QMimeType mimetype = db.mimeTypeForFile(src);
        if (mimetype.inherits(QStringLiteral("application/zip"))) {
            archive = new KZip(src);
        } else if (mimetype.inherits(QStringLiteral("application/x-compressed-tar")) || //
                   mimetype.inherits(QStringLiteral("application/x-tar")) || //
                   mimetype.inherits(QStringLiteral("application/x-bzip-compressed-tar")) || //
                   mimetype.inherits(QStringLiteral("application/x-xz")) || //
                   mimetype.inherits(QStringLiteral("application/x-lzma"))) {
            archive = new KTar(src);
        } else {
            // qCWarning(KPACKAGE_LOG) << "Could not open package file, unsupported archive format:" << src << mimetype.name();
            d->errorMessage = i18n("Could not open package file, unsupported archive format: %1 %2", src, mimetype.name());
            d->errorCode = PackageJob::JobError::UnsupportedArchiveFormatError;
            return false;
        }

        if (!archive->open(QIODevice::ReadOnly)) {
            // qCWarning(KPACKAGE_LOG) << "Could not open package file:" << src;
            delete archive;
            d->errorMessage = i18n("Could not open package file: %1", src);
            d->errorCode = PackageJob::JobError::PackageOpenError;
            return false;
        }

        archivedPackage = true;
        path = tempdir.path() + QLatin1Char('/');

        d->installPath = path;

        const KArchiveDirectory *source = archive->directory();
        source->copyTo(path);

        QStringList entries = source->entries();
        if (entries.count() == 1) {
            const KArchiveEntry *entry = source->entry(entries[0]);
            if (entry->isDirectory()) {
                path = path + entry->name() + QLatin1Char('/');
            }
        }

        delete archive;
    }

    Package copyPackage = package;
    copyPackage.setPath(path);
    if (!copyPackage.isValid()) {
        d->errorMessage = i18n("Package is not considered valid");
        d->errorCode = PackageJob::JobError::InvalidPackageStructure;
        return false;
    }

    KPluginMetaData meta = copyPackage.metadata(); // The packagestructure might have set the metadata, so use that
    QString pluginName = meta.pluginId().isEmpty() ? QFileInfo(src).baseName() : meta.pluginId();
    qCDebug(KPACKAGE_LOG) << "pluginname: " << meta.pluginId();
    if (pluginName == QLatin1String("metadata")) {
        // qCWarning(KPACKAGE_LOG) << "Package plugin id not specified";
        d->errorMessage = i18n("Package plugin id not specified: %1", src);
        d->errorCode = PackageJob::JobError::PluginIdInvalidError;
        return false;
    }

    // Ensure that package names are safe so package uninstall can't inject
    // bad characters into the paths used for removal.
    const QRegularExpression validatePluginName(QStringLiteral("^[\\w\\-\\.]+$")); // Only allow letters, numbers, underscore and period.
    if (!validatePluginName.match(pluginName).hasMatch()) {
        // qCDebug(KPACKAGE_LOG) << "Package plugin id " << pluginName << "contains invalid characters";
        d->errorMessage = i18n("Package plugin id %1 contains invalid characters", pluginName);
        d->errorCode = PackageJob::JobError::PluginIdInvalidError;
        return false;
    }

    QString targetName = dest;
    if (targetName[targetName.size() - 1] != QLatin1Char('/')) {
        targetName.append(QLatin1Char('/'));
    }
    targetName.append(pluginName);

    if (QFile::exists(targetName)) {
        if (operation == PackageJob::Update) {
            KPluginMetaData oldMeta;
            if (QString jsonPath = targetName + QLatin1String("/metadata.json"); QFileInfo::exists(jsonPath)) {
                oldMeta = KPluginMetaData::fromJsonFile(jsonPath);
            }

            if (readKPackageType(oldMeta) != readKPackageType(meta)) {
                d->errorMessage = i18n("The new package has a different type from the old version already installed.");
                d->errorCode = PackageJob::JobError::UpdatePackageTypeMismatchError;
            } else if (isVersionNewer(oldMeta.version(), meta.version())) {
                const bool ok = uninstallPackage(targetName);
                if (!ok) {
                    d->errorMessage = i18n("Impossible to remove the old installation of %1 located at %2. error: %3", pluginName, targetName, d->errorMessage);
                    d->errorCode = PackageJob::JobError::OldVersionRemovalError;
                }
            } else {
                d->errorMessage = i18n("Not installing version %1 of %2. Version %3 already installed.", meta.version(), meta.pluginId(), oldMeta.version());
                d->errorCode = PackageJob::JobError::NewerVersionAlreadyInstalledError;
            }
        } else {
            d->errorMessage = i18n("%1 already exists", targetName);
            d->errorCode = PackageJob::JobError::PackageAlreadyInstalledError;
        }

        if (d->errorCode != KJob::NoError) {
            d->installPath = targetName;
            return false;
        }
    }

    // install dependencies
    const QStringList optionalDependencies{QStringLiteral("sddmtheme.knsrc")};
    const QStringList dependencies = meta.value(QStringLiteral("X-KPackage-Dependencies"), QStringList());
    for (const QString &dep : dependencies) {
        QUrl depUrl(dep);
        const QString knsrcFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("knsrcfiles/") + depUrl.host());
        if (knsrcFilePath.isEmpty() && optionalDependencies.contains(depUrl.host())) {
            qWarning() << "Skipping depdendency due to knsrc files being missing" << depUrl;
            continue;
        }
        if (!installDependency(depUrl)) {
            d->errorMessage = i18n("Could not install dependency: '%1'", dep);
            d->errorCode = PackageJob::JobError::PackageCopyError;
            return false;
        }
    }

    if (archivedPackage) {
        // it's in a temp dir, so just move it over.
        const bool ok = copyFolder(path, targetName);
        removeFolder(path);
        if (!ok) {
            // qCWarning(KPACKAGE_LOG) << "Could not move package to destination:" << targetName;
            d->errorMessage = i18n("Could not move package to destination: %1", targetName);
            d->errorCode = PackageJob::JobError::PackageMoveError;
            return false;
        }
    } else {
        // it's a directory containing the stuff, so copy the contents rather
        // than move them
        const bool ok = copyFolder(path, targetName);
        if (!ok) {
            // qCWarning(KPACKAGE_LOG) << "Could not copy package to destination:" << targetName;
            d->errorMessage = i18n("Could not copy package to destination: %1", targetName);
            d->errorCode = PackageJob::JobError::PackageCopyError;
            return false;
        }
    }

    if (archivedPackage) {
        // no need to remove the temp dir (which has been successfully moved if it's an archive)
        tempdir.setAutoRemove(false);
    }

    d->installPath = targetName;
    return true;
}

bool PackageJobThread::update(const QString &src, const QString &dest, const Package &package)
{
    bool ok = installPackage(src, dest, package, PackageJob::Update);
    Q_EMIT installPathChanged(d->installPath);
    Q_EMIT jobThreadFinished(ok, errorCode(), d->errorMessage);
    return ok;
}

bool PackageJobThread::uninstall(const QString &packagePath)
{
    bool ok = uninstallPackage(packagePath);
    // Do not emit the install path changed, information about the removed package might be useful for consumers
    // qCDebug(KPACKAGE_LOG) << "Thread: installFinished" << ok;
    Q_EMIT jobThreadFinished(ok, errorCode(), d->errorMessage);
    return ok;
}

bool PackageJobThread::uninstallPackage(const QString &packagePath)
{
    if (!QFile::exists(packagePath)) {
        d->errorMessage = packagePath.isEmpty() ? i18n("package path was deleted manually") : i18n("%1 does not exist", packagePath);
        d->errorCode = PackageJob::JobError::PackageFileNotFoundError;
        return false;
    }
    QString pkg;
    QString root;
    {
        // TODO KF6 remove, pass in packageroot, type and pluginName separately?
        QStringList ps = packagePath.split(QLatin1Char('/'));
        int ix = ps.count() - 1;
        if (packagePath.endsWith(QLatin1Char('/'))) {
            ix = ps.count() - 2;
        }
        pkg = ps[ix];
        ps.pop_back();
        root = ps.join(QLatin1Char('/'));
    }

    bool ok = removeFolder(packagePath);
    if (!ok) {
        d->errorMessage = i18n("Could not delete package from: %1", packagePath);
        d->errorCode = PackageJob::JobError::PackageUninstallError;
        return false;
    }

    return true;
}

PackageJob::JobError PackageJobThread::errorCode() const
{
    return static_cast<PackageJob::JobError>(d->errorCode);
}

} // namespace KPackage

#include "moc_packagejobthread_p.cpp"
