/*
    SPDX-FileCopyrightText: 2019 Filip Fila <filipfila.kde@gmail.com>
    SPDX-FileCopyrightText: 2013 Reza Fatahilah Shah <rshah0385@kireihana.com>
    SPDX-FileCopyrightText: 2011, 2012 David Edmundson <kde@davidedmundson.co.uk>

    SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "sddmauthhelper.h"
#include "src/config.h"

#include <unistd.h>

#include <QBuffer>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QSharedPointer>

#include <KArchive>
#include <KCompressionDevice>
#include <KConfig>
#include <KConfigGroup>
#include <KLazyLocalizedString>
#include <KLocalizedString>
#include <KTar>
#include <KUser>
#include <KZip>

static QSharedPointer<KConfig> openConfig(const QString &filePath)
{
    // if the sddm.conf.d folder doesn't exist we fail to set the right permissions for kde_settings.conf
    QFileInfo fileLocation(filePath);
    QDir dir(fileLocation.absolutePath());
    if (!dir.exists()) {
        QDir().mkpath(dir.path());
    }
    QFile file(filePath);
    if (!file.exists()) {
        // If we are creating the config file, ensure it is world-readable: if
        // we don't do that, KConfig will create a file which is only readable
        // by root
        file.open(QIODevice::WriteOnly);
        file.close();
        file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
    }
    // in case the file has already been created with wrong permissions
    else if (!(file.permissions() & QFile::ReadOwner & QFile::WriteOwner & QFile::ReadGroup & QFile::ReadOther)) {
        file.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
    }

    return QSharedPointer<KConfig>(new KConfig(file.fileName(), KConfig::SimpleConfig));
}

static QString SddmUserCheck()
{
    // check for sddm user; return empty string if user not present
    // we have to check with QString and isEmpty() instead of QDir and exists() because
    // QDir returns "." and true for exists() in the case of a non-existent user;
    const QString sddmHomeDirPath = KUser("sddm").homeDir();
    if (sddmHomeDirPath.isEmpty()) {
        qDebug() << "Cannot proceed, user 'sddm' does not exist. Please check your SDDM install.";
        return QString();
    } else {
        return sddmHomeDirPath;
    }
}

void SddmAuthHelper::copyDirectoryRecursively(const QString &source, const QString &destination, QSet<QString> &done)
{
    if (done.contains(source)) {
        return;
    }
    done.insert(source);

    const QDir sourceDir(source);
    const auto entries = sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    for (const auto &entry : entries) {
        const QString destinationPath = destination + QLatin1Char('/') + entry.fileName();
        if (entry.isFile()) {
            copyFile(entry.absoluteFilePath(), destinationPath);
        } else {
            QDir().mkpath(destinationPath);
            copyDirectoryRecursively(entry.absoluteFilePath(), destinationPath, done);
        }
    }
}

void SddmAuthHelper::copyFile(const QString &source, const QString &destination)
{
    KUser sddmUser(QStringLiteral("sddm"));

    if (QFile::exists(destination)) {
        QFile::remove(destination);
    }

    if (!QFile::copy(source, destination)) {
        qWarning() << "Could not copy" << source << "to" << destination;
    }
    const char *destinationConverted = destination.toLocal8Bit().data();
    if (chown(destinationConverted, sddmUser.userId().nativeId(), sddmUser.groupId().nativeId())) {
        return;
    }
}

ActionReply SddmAuthHelper::sync(const QVariantMap &args)
{
    // abort if user not present
    const QString sddmHomeDirPath = SddmUserCheck();
    if (sddmHomeDirPath.isEmpty()) {
        return ActionReply::HelperErrorReply();
    }

    // In plasma-framework, ThemePrivate::useCache documents the requirement to
    // clear the cache when colors change while the app that uses them isn't running;
    // that condition applies to the SDDM greeter here, so clear the cache if it
    // exists to make sure SDDM has a fresh state
    QDir sddmCacheLocation(sddmHomeDirPath + QStringLiteral("/.cache"));
    if (sddmCacheLocation.exists()) {
        sddmCacheLocation.removeRecursively();
    }

    // create SDDM config directory if it does not exist
    QDir sddmConfigLocation(sddmHomeDirPath + QStringLiteral("/.config"));
    if (!sddmConfigLocation.exists()) {
        QDir().mkpath(sddmConfigLocation.path());
    }

    // copy fontconfig (font, font rendering)
    if (!args[QStringLiteral("fontconfig")].isNull()) {
        QDir fontconfigSource(args[QStringLiteral("fontconfig")].toString());
        QStringList sourceFileEntries = fontconfigSource.entryList(QDir::Files);
        QStringList sourceDirEntries = fontconfigSource.entryList(QDir::AllDirs);
        QDir fontconfigDestination(sddmConfigLocation.path() + QStringLiteral("/fontconfig"));

        if (!fontconfigDestination.exists()) {
            QDir().mkpath(fontconfigDestination.path());
        }

        if (sourceDirEntries.count() != 0) {
            for (int i = 0; i < sourceDirEntries.count(); i++) {
                QString directoriesSource = fontconfigSource.path() + QDir::separator() + sourceDirEntries[i];
                QString directoriesDestination = fontconfigDestination.path() + QDir::separator() + sourceDirEntries[i];
                fontconfigSource.mkpath(directoriesDestination);
                copyFile(directoriesSource, directoriesDestination);
            }
        }

        if (sourceFileEntries.count() != 0) {
            for (int i = 0; i < sourceFileEntries.count(); i++) {
                QString filesSource = fontconfigSource.path() + QDir::separator() + sourceFileEntries[i];
                QString filesDestination = fontconfigDestination.path() + QDir::separator() + sourceFileEntries[i];
                copyFile(filesSource, filesDestination);
            }
        }
    }

    // copy kdeglobals (color scheme)
    if (!args[QStringLiteral("kdeglobals")].isNull()) {
        QDir kdeglobalsSource(args[QStringLiteral("kdeglobals")].toString());
        QDir kdeglobalsDestination(sddmConfigLocation.path() + QStringLiteral("/kdeglobals"));
        copyFile(kdeglobalsSource.path(), kdeglobalsDestination.path());
    }

    // copy plasmarc (icons, UI style)
    if (!args[QStringLiteral("plasmarc")].isNull()) {
        QDir plasmarcSource(args[QStringLiteral("plasmarc")].toString());
        QDir plasmarcDestination(sddmConfigLocation.path() + QStringLiteral("/plasmarc"));
        copyFile(plasmarcSource.path(), plasmarcDestination.path());
    }

    // If SDDM is set up to use Wayland with KWin, NumLock is controlled by this
    if (!args[QStringLiteral("kcminputrc")].isNull()) {
        QDir kcminputrcSource(args[QStringLiteral("kcminputrc")].toString());
        QDir kcminputrcDestination(sddmConfigLocation.path() + QStringLiteral("/kcminputrc"));
        copyFile(kcminputrcSource.path(), kcminputrcDestination.path());
    }

    // copy kscreen config
    if (!args[QStringLiteral("kscreen-config")].isNull()) {
        const QString destinationDir = sddmHomeDirPath + QStringLiteral("/.local/share/kscreen/");
        QSet<QString> done;
        copyDirectoryRecursively(args[QStringLiteral("kscreen-config")].toString(), destinationDir, done);
    }

    // copy KWin config
    if (!args[QStringLiteral("kwinoutputconfig")].isNull()) {
        QDir source(args[QStringLiteral("kwinoutputconfig")].toString());
        QDir destination(sddmConfigLocation.path() + QStringLiteral("/kwinoutputconfig.json"));
        copyFile(source.path(), destination.path());
    }

    // write cursor theme, NumLock preference, and scaling DPI to config file
    ActionReply reply = ActionReply::HelperErrorReply();
    QSharedPointer<KConfig> sddmConfig = openConfig(args[QStringLiteral("kde_settings.conf")].toString());
    QSharedPointer<KConfig> sddmOldConfig = openConfig(args[QStringLiteral("sddm.conf")].toString());

    QMap<QString, QVariant>::const_iterator iterator;

    for (iterator = args.constBegin(); iterator != args.constEnd(); ++iterator) {
        if (iterator.key() == QLatin1String("kde_settings.conf")) {
            continue;
        }

        QStringList configFields = iterator.key().split(QLatin1Char('/'));
        if (configFields.size() != 3) {
            continue;
        }

        QSharedPointer<KConfig> config;
        QString fileName = configFields[0];
        QString groupName = configFields[1];
        QString keyName = configFields[2];

        if (fileName == QLatin1String("kde_settings.conf")) {
            if (iterator.value().isValid()) {
                sddmConfig->group(groupName).writeEntry(keyName, iterator.value());
            } else {
                sddmConfig->group(groupName).deleteEntry(keyName);
            }
            sddmOldConfig->group(groupName).deleteEntry(keyName);
        }
    }

    sddmOldConfig->sync();
    sddmConfig->sync();

    return ActionReply::SuccessReply();
}

ActionReply SddmAuthHelper::reset(const QVariantMap &args)
{
    // abort if user not present
    const QString sddmHomeDirPath = SddmUserCheck();
    if (sddmHomeDirPath.isEmpty()) {
        return ActionReply::HelperErrorReply();
    }

    QDir sddmConfigLocation(sddmHomeDirPath + QStringLiteral("/.config"));
    QDir fontconfigDir(args[QStringLiteral("sddmUserConfig")].toString() + QStringLiteral("/fontconfig"));

    fontconfigDir.removeRecursively();
    QFile::remove(sddmConfigLocation.path() + QStringLiteral("/kdeglobals"));
    QFile::remove(sddmConfigLocation.path() + QStringLiteral("/plasmarc"));

    QDir(sddmHomeDirPath + QStringLiteral("/.local/share/kscreen/")).removeRecursively();

    // remove cursor theme, NumLock preference, and scaling DPI from config file
    ActionReply reply = ActionReply::HelperErrorReply();
    QSharedPointer<KConfig> sddmConfig = openConfig(args[QStringLiteral("kde_settings.conf")].toString());
    QSharedPointer<KConfig> sddmOldConfig = openConfig(args[QStringLiteral("sddm.conf")].toString());

    QMap<QString, QVariant>::const_iterator iterator;

    for (iterator = args.constBegin(); iterator != args.constEnd(); ++iterator) {
        if (iterator.key() == QLatin1String("kde_settings.conf")) {
            continue;
        }

        QStringList configFields = iterator.key().split(QLatin1Char('/'));
        if (configFields.size() != 3) {
            continue;
        }

        QSharedPointer<KConfig> config;
        QString fileName = configFields[0];
        QString groupName = configFields[1];
        QString keyName = configFields[2];

        if (fileName == QLatin1String("kde_settings.conf")) {
            sddmConfig->group(groupName).deleteEntry(keyName);
            sddmOldConfig->group(groupName).deleteEntry(keyName);
        }
    }

    sddmOldConfig->sync();
    sddmConfig->sync();

    return ActionReply::SuccessReply();
}

ActionReply SddmAuthHelper::save(const QVariantMap &args)
{
    ActionReply reply = ActionReply::HelperErrorReply();
    QSharedPointer<KConfig> sddmConfig = openConfig(QString{QLatin1String(SDDM_CONFIG_DIR "/") + QStringLiteral("kde_settings.conf")});
    QSharedPointer<KConfig> sddmOldConfig = openConfig(QStringLiteral(SDDM_CONFIG_FILE));
    QSharedPointer<KConfig> themeConfig;
    QString themeConfigFile = args[QStringLiteral("theme.conf.user")].toString();

    if (!themeConfigFile.isEmpty()) {
        themeConfig = openConfig(themeConfigFile);
    }

    QMap<QString, QVariant>::const_iterator iterator;

    for (iterator = args.constBegin(); iterator != args.constEnd(); ++iterator) {
        if (iterator.key() == QLatin1String("kde_settings.conf") || iterator.key() == QLatin1String("theme.conf.user")) {
            continue;
        }

        QStringList configFields = iterator.key().split(QLatin1Char('/'));
        if (configFields.size() != 3) {
            continue;
        }

        QString fileName = configFields[0];
        QString groupName = configFields[1];
        QString keyName = configFields[2];

        // if there is an identical keyName in "sddm.conf" we want to delete it so SDDM doesn't read from the old file
        // hierarchically SDDM prefers "etc/sddm.conf" to "/etc/sddm.conf.d/some_file.conf"

        if (fileName == QLatin1String("kde_settings.conf")) {
            sddmConfig->group(groupName).writeEntry(keyName, iterator.value());
            sddmOldConfig->group(groupName).deleteEntry(keyName);
        } else if (fileName == QLatin1String("theme.conf.user") && !themeConfig.isNull()) {
            QFileInfo themeConfigFileInfo(themeConfigFile);
            QDir configRootDirectory = themeConfigFileInfo.absoluteDir();

            if (keyName == QLatin1String("background")) {
                QFileInfo newBackgroundFileInfo(iterator.value().toString());
                QString previousBackground = themeConfig->group(groupName).readEntry(keyName);

                bool backgroundChanged = newBackgroundFileInfo.fileName() != previousBackground;
                if (backgroundChanged) {
                    if (!previousBackground.isEmpty()) {
                        QString previousBackgroundPath = configRootDirectory.filePath(previousBackground);
                        if (QFile::remove(previousBackgroundPath)) {
                            qDebug() << "Removed previous background " << previousBackgroundPath;
                        }
                    }

                    if (newBackgroundFileInfo.exists()) {
                        QString newBackgroundPath = configRootDirectory.filePath(newBackgroundFileInfo.fileName());
                        qDebug() << "Copying background from " << newBackgroundFileInfo.absoluteFilePath() << " to " << newBackgroundPath;
                        if (QFile::copy(newBackgroundFileInfo.absoluteFilePath(), newBackgroundPath)) {
                            QFile::setPermissions(newBackgroundPath, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
                            themeConfig->group(groupName).writeEntry(keyName, newBackgroundFileInfo.fileName());
                        }
                    } else {
                        themeConfig->group(groupName).deleteEntry(keyName);
                    }
                }
            } else {
                themeConfig->group(groupName).writeEntry(keyName, iterator.value());
            }
        }
    }

    sddmOldConfig->sync();
    sddmConfig->sync();

    if (!themeConfig.isNull()) {
        themeConfig->sync();
    }

    return ActionReply::SuccessReply();
}

ActionReply SddmAuthHelper::installtheme(const QVariantMap &args)
{
    const QDBusUnixFileDescriptor themefileFd = args[QStringLiteral("filedescriptor")].value<QDBusUnixFileDescriptor>();
    QFile theme;

    if (!themefileFd.isValid()) {
        return ActionReply::HelperErrorReply();
    }

    if (!theme.open(themefileFd.fileDescriptor(), QIODevice::ReadOnly)) {
        return ActionReply::HelperErrorReply();
    }

    const QString themesBaseDir = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("sddm/themes"), QStandardPaths::LocateDirectory);
    QDir dir(themesBaseDir);
    if (!dir.exists()) {
        return ActionReply::HelperErrorReply();
    }

    QByteArray themeData = theme.readAll();
    QBuffer themeBuffer(&themeData);

    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForData(themeData);
    qWarning() << "Postinstallation: uncompress the file";

    QScopedPointer<QIODevice> filterDev;
    KCompressionDevice::CompressionType compressionType = KCompressionDevice::compressionTypeForMimeType(mimeType.name());

    if (compressionType == KCompressionDevice::None) {
        // KCompressionDevice::None may indicate an uncompressed Tar or any other archive format
        filterDev.reset(new QBuffer(&themeBuffer.buffer()));
    } else {
        filterDev.reset(new KCompressionDevice(&themeBuffer, false, compressionType));
    }

    QStringList installedPaths;

    {
        QScopedPointer<KArchive> archive;

        // there must be a better way to do this? If not, make a static bool KZip::supportsMimeType(const QMimeType &type);?
        // or even a factory class in KArchive
        if (mimeType.inherits(QStringLiteral("application/zip"))) {
            archive.reset(new KZip(filterDev.data()));
        } else if (mimeType.inherits(QStringLiteral("application/tar")) || mimeType.inherits(QStringLiteral("application/x-gzip"))
                   || mimeType.inherits(QStringLiteral("application/x-bzip")) || mimeType.inherits(QStringLiteral("application/x-lzma"))
                   || mimeType.inherits(QStringLiteral("application/x-xz")) || mimeType.inherits(QStringLiteral("application/x-bzip-compressed-tar"))
                   || mimeType.inherits(QStringLiteral("application/x-compressed-tar"))) {
            archive.reset(new KTar(filterDev.data()));
        } else {
            auto e = ActionReply::HelperErrorReply();
            e.setErrorDescription(QString::fromUtf8(kli18n("Invalid theme package").untranslatedText()));
            return e;
        }

        if (!archive->open(QIODevice::ReadOnly)) {
            auto e = ActionReply::HelperErrorReply();
            e.setErrorDescription(QString::fromUtf8(kli18n("Could not open file").untranslatedText()));
            return e;
        }

        auto directory = archive->directory();

        // some basic validation
        // the top level should only have folders, and those folders should contain a valid metadata.desktop file
        // if we get anything else, abort everything before copying
        const auto entries = directory->entries();
        for (const QString &name : entries) {
            auto entry = directory->entry(name);
            if (!entry->isDirectory()) {
                auto e = ActionReply::HelperErrorReply();
                e.setErrorDescription(QString::fromUtf8(kli18n("Invalid theme package").untranslatedText()));
                return e;
            }
            auto subDirectory = static_cast<const KArchiveDirectory *>(entry);
            auto metadataFile = subDirectory->file(QStringLiteral("metadata.desktop"));
            if (!metadataFile || !metadataFile->data().contains("[SddmGreeterTheme]")) {
                auto e = ActionReply::HelperErrorReply();
                e.setErrorDescription(QString::fromUtf8(kli18n("Invalid theme package").untranslatedText()));
                return e;
            }
            installedPaths.append(themesBaseDir + QLatin1Char('/') + name);
        }

        if (!directory->copyTo(themesBaseDir)) {
            auto e = ActionReply::HelperErrorReply();
            e.setErrorDescription(QString::fromUtf8(kli18n("Could not decompress archive").untranslatedText()));
            return e;
        }
    }

    auto rc = ActionReply::SuccessReply();
    rc.addData(QStringLiteral("installedPaths"), installedPaths);
    return rc;
}

ActionReply SddmAuthHelper::uninstalltheme(const QVariantMap &args)
{
    const QString themePath = args[QStringLiteral("filePath")].toString();
    const QString themesBaseDir = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("sddm/themes"), QStandardPaths::LocateDirectory);

    QDir dir(themePath);
    if (!dir.exists()) {
        return ActionReply::HelperErrorReply();
    }

    // validate the themePath is directly inside the themesBaseDir
    QDir baseDir(themesBaseDir);
    if (baseDir.absoluteFilePath(dir.dirName()) != dir.absolutePath()) {
        return ActionReply::HelperErrorReply();
    }

    if (!dir.removeRecursively()) {
        return ActionReply::HelperErrorReply();
    }

    return ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmsddm", SddmAuthHelper)

#include "moc_sddmauthhelper.cpp"
