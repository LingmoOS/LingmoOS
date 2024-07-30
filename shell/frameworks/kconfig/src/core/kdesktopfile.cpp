/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Pietro Iglio <iglio@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdesktopfile.h"

#include "kauthorized.h"
#include "kconfig_core_log_settings.h"
#include "kconfig_p.h"
#include "kconfiggroup.h"
#include "kconfigini_p.h"
#include "kdesktopfileaction.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>

#ifndef Q_OS_WIN
#include <unistd.h>
#endif

#include <algorithm>

class KDesktopFilePrivate : public KConfigPrivate
{
public:
    KDesktopFilePrivate(QStandardPaths::StandardLocation resourceType, const QString &fileName)
        : KConfigPrivate(KConfig::NoGlobals, resourceType)
    {
        changeFileName(fileName);
    }
    KConfigGroup desktopGroup;
};

KDesktopFile::KDesktopFile(QStandardPaths::StandardLocation resourceType, const QString &fileName)
    : KConfig(*new KDesktopFilePrivate(resourceType, fileName))
{
    Q_D(KDesktopFile);
    reparseConfiguration();
    d->desktopGroup = KConfigGroup(this, QStringLiteral("Desktop Entry"));
}

KDesktopFile::KDesktopFile(const QString &fileName)
    : KDesktopFile(QStandardPaths::ApplicationsLocation, fileName)
{
}

KDesktopFile::~KDesktopFile() = default;

KConfigGroup KDesktopFile::desktopGroup() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup;
}

QString KDesktopFile::locateLocal(const QString &path)
{
    static const QLatin1Char slash('/');

    // Relative to config? (e.g. for autostart)
    const QStringList genericConfig = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    // Iterate from the last item since some items may be subfolders of others.
    auto it = std::find_if(genericConfig.crbegin(), genericConfig.crend(), [&path](const QString &dir) {
        return path.startsWith(dir + slash);
    });
    if (it != genericConfig.crend()) {
        return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + slash + QStringView(path).mid(it->size() + 1);
    }

    QString relativePath;
    // Relative to xdg data dir? (much more common)
    const QStringList lstGenericDataLocation = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const QString &dir : lstGenericDataLocation) {
        if (path.startsWith(dir + slash)) {
            relativePath = path.mid(dir.length() + 1);
        }
    }
    if (relativePath.isEmpty()) {
        // What now? The desktop file doesn't come from XDG_DATA_DIRS. Use filename only and hope for the best.
        relativePath = path.mid(path.lastIndexOf(slash) + 1);
    }
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + slash + relativePath;
}

bool KDesktopFile::isDesktopFile(const QString &path)
{
    return path.endsWith(QLatin1String(".desktop"));
}

bool KDesktopFile::isAuthorizedDesktopFile(const QString &path)
{
    if (path.isEmpty()) {
        return false; // Empty paths are not ok.
    }

    if (QDir::isRelativePath(path)) {
        return true; // Relative paths are ok.
    }

    const QString realPath = QFileInfo(path).canonicalFilePath();
    if (realPath.isEmpty()) {
        return false; // File doesn't exist.
    }

#ifndef Q_OS_WIN
    static constexpr Qt::CaseSensitivity sensitivity = Qt::CaseSensitive;
#else
    static constexpr Qt::CaseSensitivity sensitivity = Qt::CaseInsensitive;
#endif

    // Check if the .desktop file is installed as part of KDE or XDG.
    const QStringList appsDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    auto it = std::find_if(appsDirs.cbegin(), appsDirs.cend(), [&realPath, &path](const QString &prefix) {
        QFileInfo info(prefix);
        return info.exists() && info.isDir() && (realPath.startsWith(info.canonicalFilePath(), sensitivity) || path.startsWith(info.canonicalFilePath()));
    });
    if (it != appsDirs.cend()) {
        return true;
    }

    const QString autostartDir = QStringLiteral("autostart/");
    const QStringList lstConfigPath = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation);
    auto configIt = std::find_if(lstConfigPath.cbegin(), lstConfigPath.cend(), [&realPath, &autostartDir](const QString &xdgDataPrefix) {
        QFileInfo info(xdgDataPrefix);
        if (info.exists() && info.isDir()) {
            const QString prefix = info.canonicalFilePath();
            return realPath.startsWith(prefix + QLatin1Char('/') + autostartDir, sensitivity);
        }
        return false;
    });
    if (configIt != lstConfigPath.cend()) {
        return true;
    }

    // Forbid desktop files outside of standard locations if kiosk is set so
    if (!KAuthorized::authorize(QStringLiteral("run_desktop_files"))) {
        qCWarning(KCONFIG_CORE_LOG) << "Access to" << path << "denied because of 'run_desktop_files' restriction.";
        return false;
    }

    // Not otherwise permitted, so only allow if the file is executable, or if
    // owned by root (uid == 0)
    QFileInfo entryInfo(path);
    if (entryInfo.isExecutable() || entryInfo.ownerId() == 0) {
        return true;
    }

    qCInfo(KCONFIG_CORE_LOG) << "Access to" << path << "denied, not owned by root and executable flag not set.";
    return false;
}

QString KDesktopFile::readType() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("Type", QString());
}

QString KDesktopFile::readIcon() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("Icon", QString());
}

QString KDesktopFile::readName() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("Name", QString());
}

QString KDesktopFile::readComment() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("Comment", QString());
}

QString KDesktopFile::readGenericName() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("GenericName", QString());
}

QString KDesktopFile::readPath() const
{
    Q_D(const KDesktopFile);
    // NOT readPathEntry, it is not XDG-compliant: it performs
    // various expansions, like $HOME.  Note that the expansion
    // behaviour still happens if the "e" flag is set, maintaining
    // backwards compatibility.
    return d->desktopGroup.readEntry("Path", QString());
}

QString KDesktopFile::readUrl() const
{
    Q_D(const KDesktopFile);
    if (hasDeviceType()) {
        return d->desktopGroup.readEntry("MountPoint", QString());
    } else {
        // NOT readPathEntry (see readPath())
        QString url = d->desktopGroup.readEntry("URL", QString());
        if (!url.isEmpty() && !QDir::isRelativePath(url)) {
            // Handle absolute paths as such (i.e. we need to escape them)
            return QUrl::fromLocalFile(url).toString();
        }
        return url;
    }
}

QStringList KDesktopFile::readActions() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readXdgListEntry("Actions");
}

QStringList KDesktopFile::readMimeTypes() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readXdgListEntry("MimeType");
}

KConfigGroup KDesktopFile::actionGroup(const QString &group)
{
    return KConfigGroup(this, QLatin1String("Desktop Action ") + group);
}

KConfigGroup KDesktopFile::actionGroup(const QString &group) const
{
    return const_cast<KDesktopFile *>(this)->actionGroup(group);
}

bool KDesktopFile::hasActionGroup(const QString &group) const
{
    return hasGroup(QString(QLatin1String("Desktop Action ") + group));
}

bool KDesktopFile::hasLinkType() const
{
    return readType() == QLatin1String("Link");
}

bool KDesktopFile::hasApplicationType() const
{
    return readType() == QLatin1String("Application");
}

bool KDesktopFile::hasDeviceType() const
{
    return readType() == QLatin1String("FSDevice");
}

bool KDesktopFile::tryExec() const
{
    Q_D(const KDesktopFile);
    // Test for TryExec and "X-KDE-AuthorizeAction"
    // NOT readPathEntry (see readPath())
    const QString te = d->desktopGroup.readEntry("TryExec", QString());
    if (!te.isEmpty() && QStandardPaths::findExecutable(te).isEmpty()) {
        return false;
    }
    const QStringList list = d->desktopGroup.readEntry("X-KDE-AuthorizeAction", QStringList());
    const auto isNotAuthorized = std::any_of(list.cbegin(), list.cend(), [](const QString &action) {
        return !KAuthorized::authorize(action.trimmed());
    });
    if (isNotAuthorized) {
        return false;
    }

    // See also KService::username()
    if (d->desktopGroup.readEntry("X-KDE-SubstituteUID", false)) {
        QString user = d->desktopGroup.readEntry("X-KDE-Username", QString());
        if (user.isEmpty()) {
            user = qEnvironmentVariable("ADMIN_ACCOUNT"), QStringLiteral("root");
        }
        if (!KAuthorized::authorize(QLatin1String("user/") + user)) {
            return false;
        }
    }

    return true;
}

QString KDesktopFile::readDocPath() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readPathEntry("X-DocPath", QString());
}

KDesktopFile *KDesktopFile::copyTo(const QString &file) const
{
    KDesktopFile *config = new KDesktopFile(QString());
    this->KConfig::copyTo(file, config);
    return config;
}

QString KDesktopFile::fileName() const
{
    return name();
}

bool KDesktopFile::noDisplay() const
{
    Q_D(const KDesktopFile);
    return d->desktopGroup.readEntry("NoDisplay", false);
}

QList<KDesktopFileAction> KDesktopFile::actions() const
{
    QList<KDesktopFileAction> desktopFileActions;
    const QStringList actionKeys = readActions();
    for (const QString &actionKey : actionKeys) {
        const KConfigGroup grp = actionGroup(actionKey);
        desktopFileActions << KDesktopFileAction(actionKey, grp.readEntry("Name"), grp.readEntry("Icon"), grp.readEntry("Exec"), fileName());
    }
    return desktopFileActions;
}
