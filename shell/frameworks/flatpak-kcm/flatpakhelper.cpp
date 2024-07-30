/**
 * SPDX-FileCopyrightText: 2022 Suhaas Joshi <joshiesuhaas0@gmail.com>
 * SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "flatpakhelper.h"
#include "config.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>

#include <gio/gio.h>

namespace FlatpakHelper
{

QString userBaseDirectory()
{
    static const QString dir = []() {
        QString dir;
        const auto env = qEnvironmentVariable("FLATPAK_USER_DIR");
        if (!env.isEmpty()) {
            dir = QStringLiteral("%1").arg(env);
        } else {
            // Takes care of XDG_DATA_HOME
            const auto userDataDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
            dir = QStringLiteral("%1/flatpak").arg(userDataDir);
        }
        return dir;
    }();
    return dir;
}

QString systemBaseDirectory()
{
    static const QString dir = []() {
        const auto env = qEnvironmentVariable("FLATPAK_SYSTEM_DIR");
        if (!env.isEmpty()) {
            return env;
        }
        return QStringLiteral(FLATPAK_SYSTEMDIR);
    }();
    return dir;
}

QString userOverridesDirectory()
{
    static const QString dir = []() {
        const auto base = userBaseDirectory();
        const auto dir = QStringLiteral("%1/overrides").arg(base);
        return dir;
    }();
    return dir;
}

QString systemOverridesDirectory()
{
    static const QString dir = []() {
        const auto base = systemBaseDirectory();
        const auto dir = QStringLiteral("%1/overrides").arg(base);
        return dir;
    }();
    return dir;
}

static QString metadataPathForInstallation(const QString &flatpakBaseDirectory, const QString &flatpakName)
{
    return QStringLiteral("%1/app/%2/current/active/metadata").arg(flatpakBaseDirectory, flatpakName);
}

QString metadataPathForUserInstallation(const QString &flatpakName)
{
    const auto base = userBaseDirectory();
    return metadataPathForInstallation(base, flatpakName);
}

QString metadataPathForSystemInstallation(const QString &flatpakName)
{
    const auto base = systemBaseDirectory();
    return metadataPathForInstallation(base, flatpakName);
}

QUrl iconSourceUrl(const QString &displayName, const QString &flatpakName, const QString &appBaseDirectory)
{
    QString dirPath = appBaseDirectory + QStringLiteral("/files/share/icons/hicolor/");
    QDir dir(dirPath);
    dir.setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

    QString nextDir;
    if (dir.exists(QStringLiteral("scalable"))) {
        nextDir = QStringLiteral("scalable");
    } else if (dir.exists(QStringLiteral("symbolic"))) {
        nextDir = QStringLiteral("symbolic");
    } else if (!dir.isEmpty()) {
        nextDir = dir.entryList().at(0);
    } else {
        return QUrl();
    }
    dir.cd(nextDir + QStringLiteral("/apps"));

    QString file = flatpakName + QStringLiteral(".png");
    if (!dir.exists(file)) {
        file = flatpakName + QStringLiteral(".svg");
        if (!dir.exists(file)) {
            file = displayName.toLower() + QStringLiteral(".png");
            if (!dir.exists(file)) {
                file = displayName.toLower() + QStringLiteral(".svg");
                if (!dir.exists(file)) {
                    return QUrl();
                }
            }
        }
    }
    return QUrl::fromLocalFile(dir.absoluteFilePath(file));
}

bool verifyDBusName(QStringView name)
{
    if (name.endsWith(QLatin1String(".*"))) {
        name.chop(2);
        name = name.mid(-2);
    }

    const auto ownedName = name.toString();
    const auto arrayName = ownedName.toUtf8();
    const auto cName = arrayName.constData();

    return g_dbus_is_name(cName) && !g_dbus_is_unique_name(cName);
}
}
