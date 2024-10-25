/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mimeappmanager.h"
#include "helper/filelauncher.h"

#include <QStandardPaths>
#include <QDirIterator>
#include <QUrl>
#include <QDir>

#include <QMimeDatabase>
#include <QMimeType>
#include <QSettings>
#include <QDateTime>
#include <QThread>
#include <QDebug>
#include <QSet>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

static MimeAppManager *SELF = nullptr;

MimeAppManager *MimeAppManager::self()
{
    if (!SELF)
        SELF = new MimeAppManager;

    return SELF;
}

MimeAppManager::MimeAppManager(QObject *parent)
    : QObject(parent),
      m_fileSystemWatcher(new QFileSystemWatcher),
      m_updateTimer(new QTimer(this))
{
    m_updateTimer->setInterval(100);
    m_updateTimer->setSingleShot(true);

    m_fileSystemWatcher->addPaths(desktopPaths());

    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &MimeAppManager::onFileChanged);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &MimeAppManager::onFileChanged);
    connect(m_updateTimer, &QTimer::timeout, this, &MimeAppManager::initApplications);

    m_updateTimer->start();
}

QStringList MimeAppManager::desktopPaths()
{
    QStringList folders;
    folders << QString("/usr/share/applications")
            << QString("/usr/local/share/applications/")
            << QDir::homePath() + QString("/.local/share/applications");

    return folders;
}

QString MimeAppManager::mimeAppsListFilePath()
{
    // return QString("%1/.config/mimeapps.list").arg(QDir::homePath());

    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/mimeapps.list");
}

void MimeAppManager::initApplications()
{
    m_desktopFiles.clear();
    m_desktopObjects.clear();

    QMap<QString, QSet<QString>> mimeAppsSet;

    for (const QString &folder : desktopPaths()) {
        QDirIterator itor(folder, QStringList("*.desktop"), QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (itor.hasNext()) {
            itor.next();
            QString filePath = itor.filePath();
            XdgDesktopFile desktopFile(filePath);

            if (!desktopFile.valid())
                continue;

            if (desktopFile.value("Terminal").toBool())
                continue;

            m_desktopFiles.append(filePath);
            m_desktopObjects.insert(filePath, desktopFile);

            // Load terminal
            QStringList categories = desktopFile.value("Categories").toString().split(";");
            if (categories.contains("TerminalEmulator")) {
                m_terminalApps.append(desktopFile);
            }

            QStringList mimeTypes = desktopFile.value("MimeType").toString().trimmed().split(";");
            for (const QString &mimeType : mimeTypes) {
                if (!mimeType.isEmpty()) {
                    QSet<QString> apps;
                    if (mimeAppsSet.contains(mimeType)) {
                        apps = mimeAppsSet.value(mimeType);
                        apps.insert(filePath);
                    } else {
                        apps.insert(filePath);
                    }

                    mimeAppsSet.insert(mimeType, apps);
                }
            }
        }
    }

    for (const QString &key : mimeAppsSet.keys()) {
        QSet<QString> apps = mimeAppsSet.value(key);
        QStringList orderApps;

        if (apps.count() > 1) {
            QFileInfoList fileInfos;
            for (const QString &app : apps) {
                QFileInfo info(app);
                fileInfos.append(info);
            }

            std::sort(fileInfos.begin(), fileInfos.end(), [=] (const QFileInfo &f1, const QFileInfo &f2) {
                return f1.birthTime() < f2.birthTime();
            });

            for (QFileInfo info : fileInfos) {
                orderApps.append(info.absoluteFilePath());
            }
        } else {
            orderApps.append(apps.values());
        }

        m_mimeApps.insert(key, orderApps);
    }

    // Check from cache.
    // ref: https://specifications.freedesktop.org/desktop-entry-spec/0.9.5/ar01s07.html

    QFile file("/usr/share/applications/mimeinfo.cache");
    if (!file.open(QIODevice::ReadOnly))
        return;

    QStringList audioDesktopList;
    QStringList imageDeksopList;
    QStringList textDekstopList;
    QStringList videoDesktopList;

    while (!file.atEnd()) {
        QString line = file.readLine();
        QString mimeType = line.split("=").first();
        QString _desktops = line.split("=").last();
        QStringList desktops = _desktops.split(";");

        for (const QString &desktop : desktops) {
            if (desktop.isEmpty() || audioDesktopList.contains(desktop))
                continue;

            if (mimeType.startsWith("audio")) {
                if (!audioDesktopList.contains(desktop))
                    audioDesktopList.append(desktop);
            } else if (mimeType.startsWith("image")) {
                if (!imageDeksopList.contains(desktop))
                    imageDeksopList.append(desktop);
            } else if (mimeType.startsWith("text")) {
                if (!textDekstopList.contains(desktop))
                    textDekstopList.append(desktop);
            } else if (mimeType.startsWith("video")) {
                if (!videoDesktopList.contains(desktop))
                    videoDesktopList.append(desktop);
            }
        }
    }

    file.close();

    const QString mimeInfoCacheRootPath = "/usr/share/applications";
    for (const QString &desktop : audioDesktopList) {
        const QString &path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path))
            continue;

        XdgDesktopFile desktopFile(path);
        if (desktopFile.valid())
            m_audioMimeApps.insert(path, desktopFile);
    }

    for (const QString &desktop : imageDeksopList) {
        const QString &path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path))
            continue;

        XdgDesktopFile desktopFile(path);
        if (desktopFile.valid())
            m_imageMimeApps.insert(path, desktopFile);
    }

    for (const QString &desktop : textDekstopList) {
        const QString &path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path))
            continue;

        XdgDesktopFile desktopFile(path);
        if (desktopFile.valid())
            m_textMimeApps.insert(path, desktopFile);
    }

    for (const QString &desktop : videoDesktopList) {
        const QString &path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path))
            continue;

        XdgDesktopFile desktopFile(path);
        if (desktopFile.valid())
            m_videoMimeApps.insert(path, desktopFile);
    }
}

QString MimeAppManager::getDefaultAppByFilePath(const QString &filePath)
{
    return getDefaultAppByMimeType(QMimeDatabase().mimeTypeForFile(filePath));
}

QString MimeAppManager::getDefaultAppByMimeType(const QMimeType &mimeType)
{
    QString mimeappsFile = mimeAppsListFilePath();

    if (!QFile::exists(mimeappsFile))
        return QString();

    QSettings settings(mimeappsFile, QSettings::IniFormat);

    // for (const QString group : settings.childGroups()) {
    //     settings.beginGroup(group);
    //     for (const QString &key : settings.allKeys()) {
    //         if (key == mimeType.name())
    //             return settings.value(key).toString();
    //     }
    //     settings.endGroup();
    // }

    settings.beginGroup("Default Applications");
    // TODO: User applications directory?
    if (settings.contains(mimeType.name())) {
        const QString desktopFile = QString("/usr/share/applications/%1").arg(settings.value(mimeType.name()).toString());
        if (QFile::exists(desktopFile)) {
            return desktopFile;
        }
    }

    settings.endGroup();

    settings.beginGroup("Added Associations");
    if (settings.contains(mimeType.name())) {
        QString desktopFile = QString("/usr/share/applications/%1").arg(settings.value(mimeType.name()).toString());
        if (QFile::exists(desktopFile)) {
            return desktopFile;
        }
    }

    return QString();
}

QString MimeAppManager::getDefaultAppDesktopByMimeType(const QString &mimeType)
{
    return getDefaultAppByMimeType(QMimeDatabase().mimeTypeForName(mimeType));
}

bool MimeAppManager::setDefaultAppForType(const QString &mimeType, const QString &app)
{
    Q_UNUSED(mimeType);

    // ref: https://specifications.freedesktop.org/mime-apps-spec/1.0.1/ar01s03.html

    QString mimeappsFile = mimeAppsListFilePath();
    QString desktop = app;

    if (QFile::exists(desktop)) {
        QFileInfo info(desktop);
        desktop = info.completeBaseName();
    } else {
        return false;
    }

    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"),
                                                           KConfig::NoGlobals,
                                                           QStandardPaths::GenericConfigLocation);
    KConfigGroup defaultApp(profile, "Default Applications");
    defaultApp.writeXdgListEntry(mimeType, {desktop});

    KConfigGroup addedApps(profile, "Added Associations");
    QStringList apps = addedApps.readXdgListEntry(mimeType);
    apps.removeAll(desktop);
    apps.prepend(desktop); // make it the preferred app
    addedApps.writeXdgListEntry(mimeType, apps);

    profile->sync();

    return true;
}

bool MimeAppManager::setDefaultAppForFile(const QString &filePath, const QString &desktop)
{
    // ref: https://specifications.freedesktop.org/mime-apps-spec/1.0.1/ar01s03.html

    QMimeType mimeType;
    QString value = desktop;

    if (!QFile::exists(filePath))
        return false;
    else
        mimeType = QMimeDatabase().mimeTypeForFile(filePath);

    if (QFile::exists(value)) {
        QFileInfo info(value);
        value = info.fileName();
    } else {
        return false;
    }

    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"),
                                                           KConfig::NoGlobals,
                                                           QStandardPaths::GenericConfigLocation);
    KConfigGroup defaultApp(profile, "Default Applications");
    defaultApp.writeXdgListEntry(mimeType.name(), {value});

    KConfigGroup addedApps(profile, "Added Associations");
    QStringList apps = addedApps.readXdgListEntry(mimeType.name());
    apps.removeAll(value);
    apps.prepend(value); // make it the preferred app
    addedApps.writeXdgListEntry(mimeType.name(), apps);

    profile->sync();

    return true;
}

QStringList MimeAppManager::getRecommendedAppsByFilePath(const QString &filePath)
{
    return getRecommendedAppsByMimeType(QMimeDatabase().mimeTypeForFile(filePath));
}

QStringList MimeAppManager::getRecommendedAppsByMimeType(const QMimeType &mimeType)
{
    QStringList recommendApps;
    QList<QMimeType> mimeTypeList;
    QMimeDatabase mimeDatabase;

    mimeTypeList.append(mimeType);

    while (recommendApps.isEmpty()) {
        for (const QMimeType &type : mimeTypeList) {
            QStringList typeNameList;

            typeNameList.append(type.name());
            typeNameList.append(type.aliases());

            for (const QString &name : typeNameList) {
                for (const QString &app : m_mimeApps.value(name)) {
                    bool exists = false;

                    for (const QString &other : recommendApps) {
                        const XdgDesktopFile &appDesktop = m_desktopObjects.value(app);
                        const XdgDesktopFile &otherDesktop = m_desktopObjects.value(other);

                        if (appDesktop.value("Exec").toString() == otherDesktop.value("Exec").toString() &&
                                appDesktop.localeName() == otherDesktop.localeName()) {
                            exists = true;
                            break;
                        }
                    }

                    // if desktop file was not existed do not recommend!!
                    if (!QFileInfo::exists(app)) {
                        qWarning() << app << "not exist anymore";
                        continue;
                    }

                    if (!exists)
                        recommendApps.append(app);
                }
            }
        }

        if (!recommendApps.isEmpty())
            break;

        QList<QMimeType> newMimeTypeList;
        for (const QMimeType &type : mimeTypeList) {
            for (const QString &name : type.parentMimeTypes())
                newMimeTypeList.append(mimeDatabase.mimeTypeForName(name));
        }

        mimeTypeList = newMimeTypeList;

        if (mimeTypeList.isEmpty())
            break;
    }

    return recommendApps;
}

QVariantList MimeAppManager::recommendedApps(const QUrl &url)
{
    QVariantList list;

    if (url.isValid()) {
        const QString &filePath = url.toString();

        for (const QString &path : getRecommendedAppsByFilePath(filePath)) {
            XdgDesktopFile desktop(path);

            if (!desktop.valid())
                continue;

            QVariantMap item;
            item["icon"] = desktop.value("Icon").toString();
            item["name"] = desktop.localeName();
            item["desktopFile"] = path;

            list << item;
        }
    }

    return list;
}

void MimeAppManager::launchTerminal(const QString &path)
{
    if (m_terminalApps.isEmpty())
        return;

    QSettings settings("lingmoos", "defaultApps");
    QString defaultTerminal = settings.value("terminal").toString();
    QString command;

    if (!defaultTerminal.isEmpty()) {
        for (const XdgDesktopFile &f : m_terminalApps) {
            if (f.fileName().contains(defaultTerminal)) {
                command = f.value("Exec").toString();
                break;
            }
        }
    }

    if (command.isEmpty()) {
        command = m_terminalApps.first().value("Exec").toString();
    }

    FileLauncher::startDetached(command, path, QStringList());
}

void MimeAppManager::onFileChanged(const QString &path)
{
    Q_UNUSED(path);

    m_updateTimer->start();
}
