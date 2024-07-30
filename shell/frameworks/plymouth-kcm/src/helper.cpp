/*
 *  SPDX-FileCopyrightText: 2021-2022 Harald Sitter <sitter@kde.org>
 *  SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 1998 Luca Montecchiani <m.luca@usa.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "helper.h"
#include "config-kcm.h"

#include <algorithm>
#include <chrono>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMimeDatabase>
#include <QProcess>
#include <QRegularExpression>

#include <KAuth/HelperSupport>

#include "ktar.h"
#include "kzip.h"
#include <KArchive>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

using namespace std::chrono_literals;

static QString updateAlternatives()
{
    return QStringLiteral("update-alternatives");
}

static bool hasUpdateAlternatives()
{
    return !QStandardPaths::findExecutable(updateAlternatives()).isEmpty();
}

static ActionReply updateAlternativesInstall(const QString &installPath)
{
    QProcess process;
    process.start(updateAlternatives(),
                  {QStringLiteral("--install"),
                   QStringLiteral("/usr/share/plymouth/themes/default.plymouth"),
                   QStringLiteral("default.plymouth"),
                   installPath,
                   QStringLiteral("100")});

    ActionReply reply = ActionReply::SuccessReply();
    if (!process.waitForStarted()) {
        reply = ActionReply::BackendError;
        reply.setErrorDescription(i18n("Cannot start update-alternatives."));
        return reply;
    }
    if (!process.waitForFinished()) {
        reply = ActionReply::BackendError;
        reply.setErrorDescription(i18n("update-alternatives failed to run."));
        return reply;
    }

    if (int ret = process.exitCode(); ret != 0) {
        reply = ActionReply(ActionReply::HelperErrorReply());
        reply.setErrorCode(static_cast<ActionReply::Error>(ret));
        reply.setErrorDescription(i18n("update-alternatives returned with error condition %1.", ret));
        return reply;
    }
    return reply;
}

ActionReply PlymouthHelper::save(const QVariantMap &args)
{
    const QString theme = args.value(QStringLiteral("theme")).toString();
    ActionReply reply;

    if (theme.isEmpty()) {
        reply = ActionReply::BackendError;
        reply.setErrorDescription(i18n("No theme specified in helper parameters."));
        return reply;
    }

    {
        KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral(PLYMOUTH_CONFIG_PATH)), QStringLiteral("Daemon"));
        cg.writeEntry("Theme", theme);
    }
    QFile configFile(QStringLiteral(PLYMOUTH_CONFIG_PATH));
    configFile.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther);

    // Special case: Ubuntu derivatives, which work different from everybody else
    if (hasUpdateAlternatives()) {
        // find the .plymouth file in the theme
        QDir dir(QStringLiteral(PLYMOUTH_THEMES_DIR) + theme);
        const QStringList themeFile = dir.entryList(QStringList() << QStringLiteral("*.plymouth"));
        if (themeFile.count() != 1) {
            reply = ActionReply::BackendError;
            reply.setErrorDescription(i18n("Theme corrupted: .plymouth file not found inside theme."));
            return reply;
        }

        QProcess checkProcess;
        QByteArray data;
        qDebug() << "Running update-alternatives --list default.plymouth now";
        checkProcess.start(updateAlternatives(), {QStringLiteral("--list"), QStringLiteral("default.plymouth")});
        if (!checkProcess.waitForStarted()) {
            reply = ActionReply::BackendError;
            reply.setErrorDescription(i18n("Cannot start update-alternatives."));
            return reply;
        }
        if (!checkProcess.waitForFinished()) {
            reply = ActionReply::BackendError;
            reply.setErrorDescription(i18n("update-alternatives failed to run."));
            return reply;
        }
        data = checkProcess.readAllStandardOutput();

        if (int ret = checkProcess.exitCode(); ret != 0) {
            reply = ActionReply(ActionReply::HelperErrorReply());
            reply.setErrorCode(static_cast<ActionReply::Error>(ret));
            reply.setErrorDescription(i18n("update-alternatives returned with error condition %1.", ret));
            return reply;
        }
        const QString installFile = dir.path() + QLatin1Char('/') + themeFile.first();
        if (!data.contains(installFile.toUtf8())) {
            qDebug() << "Plymouth file not found in update-alternatives. So install it";
            if (auto reply = updateAlternativesInstall(installFile); reply.failed()) {
                return reply;
            }
        } else {
            qDebug() << "Running update-alternatives --set  now";
            QProcess process;
            process.start(QStringLiteral("update-alternatives"), QStringList() << QStringLiteral("--set") << QStringLiteral("default.plymouth") << installFile);
            if (!process.waitForStarted()) {
                reply = ActionReply::BackendError;
                reply.setErrorDescription(i18n("Cannot start update-alternatives."));
                return reply;
            }
            if (!process.waitForFinished()) {
                reply = ActionReply::BackendError;
                reply.setErrorDescription(i18n("update-alternatives failed to run."));
                return reply;
            }

            if (int ret = process.exitCode(); ret != 0) {
                reply = ActionReply(ActionReply::HelperErrorReply());
                reply.setErrorCode(static_cast<ActionReply::Error>(ret));
                reply.setErrorDescription(i18n("update-alternatives returned with error condition %1.", ret));
                return reply;
            }
        }
    }

    QProcess process;
    if (QFileInfo::exists(QStringLiteral("/usr/sbin/update-initramfs"))) {
        qDebug() << "Running update-initramfs -u now";
        process.start(QStringLiteral("/usr/sbin/update-initramfs"), QStringList() << QStringLiteral("-u"));
        if (!process.waitForStarted()) {
            reply = ActionReply::BackendError;
            reply.setErrorDescription(i18n("Cannot start initramfs."));
            return reply;
        }
    }
    if (QFileInfo::exists(QStringLiteral("/usr/bin/mkinitcpio"))) {
        qDebug() << "Running mkinitcpio -P now";
        process.start(QStringLiteral("/usr/bin/mkinitcpio"), QStringList() << QStringLiteral("-P"));
        if (!process.waitForStarted()) {
            reply = ActionReply::BackendError;
            reply.setErrorDescription(i18n("Cannot start mkinitcpio."));
            return reply;
        }
    }
    // We don't know how long this will take. The helper will need to generate N=installed_kernels initrds.
    // Be very generous with the timeout! https://bugs.kde.org/show_bug.cgi?id=400641
    // NB: there is also a timeout in the KCM
    if (!process.waitForFinished(std::chrono::milliseconds(15min).count())) {
        reply = ActionReply::BackendError;
        reply.setErrorDescription(i18n("Initramfs failed to run."));
        return reply;
    }

    int ret = process.exitCode();
    if (ret == 0) {
        return ActionReply::SuccessReply();
    }
    reply = ActionReply(ActionReply::HelperErrorReply());
    reply.setErrorCode(static_cast<ActionReply::Error>(ret));
    reply.setErrorDescription(i18n("Initramfs returned with error condition %1.", ret));
    return reply;
}

ActionReply PlymouthHelper::install(const QVariantMap &args)
{
    const QString themearchive = args.value(QStringLiteral("themearchive")).toString();
    ActionReply reply;

    if (themearchive.isEmpty()) {
        return ActionReply::BackendError;
    }

    QDir basedir(QStringLiteral(PLYMOUTH_THEMES_DIR));
    if (!basedir.exists()) {
        return ActionReply::BackendError;
    }

    // this is weird but a decompression is not a single name, so take the path instead
    QString installpath = QStringLiteral(PLYMOUTH_THEMES_DIR);
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(themearchive);
    qWarning() << "Postinstallation: uncompress the file";

    // FIXME: check for overwriting, malicious archive entries (../foo) etc.
    // FIXME: KArchive should provide "safe mode" for this!
    QScopedPointer<KArchive> archive;

    static QList<QString> tarTypes = {
        QStringLiteral("application/tar"),
        QStringLiteral("application/x-gzip"),
        QStringLiteral("application/x-bzip"),
        QStringLiteral("application/x-lzma"),
        QStringLiteral("application/x-xz"),
        QStringLiteral("application/x-bzip-compressed-tar"),
        QStringLiteral("application/x-compressed-tar"),
    };

    if (mimeType.inherits(QStringLiteral("application/zip"))) {
        archive.reset(new KZip(themearchive));
    } else if (std::any_of(tarTypes.cbegin(), tarTypes.cend(), [&](const auto &type) {
                   return mimeType.inherits(type);
               })) {
        archive.reset(new KTar(themearchive));
    } else {
        qCritical() << "Could not determine type of archive file '" << themearchive << "'";
        return ActionReply::BackendError;
    }

    if (!archive->open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open archive file '" << themearchive << "'";
        return ActionReply::BackendError;
    }

    QString themeName;
    QString themePath;
    const KArchiveDirectory *dir = archive->directory();
    // if there is more than an item in the file,
    // put contents in a subdirectory with the same name as the file
    if (dir->entries().count() > 1) {
        installpath += QLatin1Char('/') + QFileInfo(archive->fileName()).baseName();
        themeName = QFileInfo(archive->fileName()).baseName();
        themePath = installpath;
    } else {
        themeName = dir->entries().constFirst();
        themePath = installpath + dir->entries().constFirst();
    }
    dir->copyTo(installpath);

    archive->close();

    // Special case: Ubuntu derivatives, which work different from everybody else
    if (hasUpdateAlternatives()) {
        // find the .plymouth file in the theme
        QDir dir(themePath);
        const QStringList themeFile = dir.entryList({QStringLiteral("*.plymouth")});
        if (themeFile.count() != 1) {
            reply = ActionReply::BackendError;
            reply.setErrorDescription(i18n("Theme corrupted: .plymouth file not found inside theme."));
            return reply;
        }

        if (auto reply = updateAlternativesInstall(themePath + QLatin1Char('/') + themeFile.first()); reply.failed()) {
            return reply;
        }
    }

    QVariantMap map;
    map[QStringLiteral("plugin")] = themeName;
    map[QStringLiteral("path")] = themePath;
    reply = ActionReply::SuccessReply();
    reply.setData(map);
    return reply;
}

ActionReply PlymouthHelper::uninstall(const QVariantMap &args)
{
    const QString theme = args.value(QStringLiteral("theme")).toString();
    ActionReply reply;

    if (theme.isEmpty()) {
        qWarning() << "No theme specified.";
        return ActionReply::BackendError;
    }

    QDir dir(QStringLiteral(PLYMOUTH_THEMES_DIR));
    if (!dir.exists()) {
        reply = ActionReply::BackendError;
        reply.setErrorDescription(i18n("Theme folder %1 does not exist.", QStringLiteral(PLYMOUTH_THEMES_DIR)));
        return reply;
    }

    if (!dir.cd(theme)) {
        reply = ActionReply::BackendError;
        reply.setErrorDescription(i18n("Theme %1 does not exist.", theme));
        return reply;
    }

    // Special case: Ubuntu derivatives, which work different from everybody else
    if (hasUpdateAlternatives()) {
        // find the .plymouth file in the theme
        const QStringList themeFile = dir.entryList(QStringList() << QStringLiteral("*.plymouth"));
        if (themeFile.count() != 1) {
            reply = ActionReply::BackendError;
            reply.setErrorDescription(i18n("Theme corrupted: .plymouth file not found inside theme."));
            return reply;
        }
        int ret = 0;
        QProcess process;

        process.start(QStringLiteral("update-alternatives"),
                      {QStringLiteral("--remove"), QStringLiteral("default.plymouth"), dir.path() + QLatin1Char('/') + themeFile.first()});
        if (!process.waitForStarted()) {
            reply = ActionReply::BackendError;
            reply.setErrorDescription(i18n("Cannot start update-alternatives."));
            return reply;
        }
        if (!process.waitForFinished()) {
            reply = ActionReply::BackendError;
            reply.setErrorDescription(i18n("update-alternatives failed to run."));
            return reply;
        }
        ret = process.exitCode();

        if (ret != 0) {
            reply = ActionReply(ActionReply::HelperErrorReply());
            reply.setErrorCode(static_cast<ActionReply::Error>(ret));
            reply.setErrorDescription(i18n("update-alternatives returned with error condition %1.", ret));
            return reply;
        }
    }

    if (dir.removeRecursively()) {
        return ActionReply::SuccessReply();
    }
    return ActionReply::BackendError;
}

KAUTH_HELPER_MAIN("org.kde.kcontrol.kcmplymouth", PlymouthHelper)

#include "moc_helper.cpp"
