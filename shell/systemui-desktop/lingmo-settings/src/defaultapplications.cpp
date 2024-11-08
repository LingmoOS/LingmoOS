/*
 * Copyright (C) 2021 - 2022 LingmoOS Team.
 *
 * Author:     Kate Leet <kate@lingmoos.com>
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

#include "defaultapplications.h"
#include "desktopproperties.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDirIterator>
#include <QDir>
#include <QDebug>

#include <KConfig>
#include <KConfigGroup>
#include <KSharedConfig>

DefaultApplications::DefaultApplications(QObject *parent)
    : QObject(parent)
{
    loadApps();
}

void DefaultApplications::loadApps()
{
    QDirIterator it("/usr/share/applications", { "*.desktop" },
                    QDir::NoFilter, QDirIterator::Subdirectories);

    // Load apps
    while (it.hasNext()) {
        const auto fileName = it.next();
        if (!QFile::exists(fileName))
            continue;

        DesktopProperties desktop(fileName, "Desktop Entry");
        QString name = desktop.value(QString("Name[%1]").arg(QLocale::system().name())).toString();

        if (name.isEmpty())
            name = desktop.value("Name").toString();

        AppItem item;
        item.path = fileName;
        item.name = name;
        item.icon = desktop.value("Icon").toString();
        item.mimeType = desktop.value("MimeType").toString();
        item.categories = desktop.value("Categories").toString();
        item.fileName = QFileInfo(fileName).fileName();

        if (item.categories.contains("FileManager")
                && item.mimeType.contains("inode/directory")) {
            m_fileManagerList.append(item);
        } else if (item.categories.contains("WebBrowser")
                   && item.mimeType.contains("x-scheme-handler/http")) {
            m_browserList.append(item);
        } else if (item.categories.contains("Email")
                   && item.mimeType.contains("x-scheme-handler/mailto")) {
            m_emailList.append(item);
        } else if (item.categories.contains("TerminalEmulator")) {
            m_terminalList.append(item);
        }
    }

    // Load xdg config.
    QSettings mimeApps(mimeAppsListFilePath(), QSettings::IniFormat);
    mimeApps.beginGroup("Default Applications");

    QSettings settings("lingmoos", "defaultApps");

    QString defaultBrowser = mimeApps.value("x-scheme-handler/http").toString();
    QString defaultFM = mimeApps.value("inode/directory").toString();
    QString defaultEMail = mimeApps.value("x-scheme-handler/mailto").toString();
    QString defaultTerminal = settings.value("terminal").toString();

    // Init indexes.
    for (int i = 0; i < m_browserList.size(); ++i) {
        if (defaultBrowser == m_browserList.at(i).fileName) {
            m_browserIndex = i;
            break;
        }
    }

    for (int i = 0; i < m_fileManagerList.size(); ++i) {
        if (defaultFM == m_fileManagerList.at(i).fileName) {
            m_fileManagerIndex = i;
            break;
        }
    }

    for (int i = 0; i < m_emailList.size(); ++i) {
        if (defaultEMail == m_emailList.at(i).fileName) {
            m_emailIndex = i;
            break;
        }
    }

    for (int i = 0; i < m_terminalList.size(); ++i) {
        if (defaultTerminal == m_terminalList.at(i).fileName) {
            m_terminalIndex = i;
            break;
        }
    }
}

QVariantList DefaultApplications::browserList()
{
    QVariantList list;

    for (const AppItem &item : m_browserList) {
        QVariantMap map;
        map["name"] = item.name;
        map["icon"] = item.icon;
        map["path"] = item.path;
        list << map;
    }

    return list;
}

QVariantList DefaultApplications::fileManagerList()
{
    QVariantList list;

    for (const AppItem &item : m_fileManagerList) {
        QVariantMap map;
        map["name"] = item.name;
        map["icon"] = item.icon;
        map["path"] = item.path;
        list << map;
    }

    return list;
}

QVariantList DefaultApplications::emailList()
{
    QVariantList list;

    for (const AppItem &item : m_emailList) {
        QVariantMap map;
        map["name"] = item.name;
        map["icon"] = item.icon;
        map["path"] = item.path;
        list << map;
    }

    return list;
}

QVariantList DefaultApplications::terminalList()
{
    QVariantList list;

    for (const AppItem &item : m_terminalList) {
        QVariantMap map;
        map["name"] = item.name;
        map["icon"] = item.icon;
        map["path"] = item.path;
        list << map;
    }

    return list;
}

int DefaultApplications::browserIndex()
{
    return m_browserIndex;
}

int DefaultApplications::fileManagerIndex()
{
    return m_fileManagerIndex;
}

int DefaultApplications::emailIndex()
{
    return m_emailIndex;
}

int DefaultApplications::terminalIndex()
{
    return m_terminalIndex;
}

void DefaultApplications::setDefaultBrowser(int index)
{
    if (!m_browserList.isEmpty() && m_browserList.size() < index)
        return;

    const QString desktop = m_browserList.at(index).fileName;

    setDefaultApp("x-scheme-handler/http", desktop);
    setDefaultApp("x-scheme-handler/https", desktop);
}

void DefaultApplications::setDefaultFileManager(int index)
{
    if (!m_fileManagerList.isEmpty() && m_fileManagerList.size() < index)
        return;

    const QString desktop = m_fileManagerList.at(index).fileName;

    setDefaultApp("inode/directory", desktop);
}

void DefaultApplications::setDefaultEMail(int index)
{
    if (!m_emailList.isEmpty() && m_emailList.size() < index)
        return;

    const QString desktop = m_emailList.at(index).fileName;

    setDefaultApp("x-scheme-handler/mailto", desktop);
}

void DefaultApplications::setDefaultTerminal(int index)
{
    if (!m_terminalList.isEmpty() && m_terminalList.size() < index)
        return;

    const QString desktop = m_terminalList.at(index).fileName;

    qDebug() << index << desktop;

    QSettings settings("lingmoos", "defaultApps");
    settings.setValue("terminal", desktop);
}

void DefaultApplications::setDefaultApp(const QString &mimeType, const QString &path)
{
    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"),
                                                           KConfig::NoGlobals,
                                                           QStandardPaths::GenericConfigLocation);
    KConfigGroup defaultApp(profile, "Default Applications");
    defaultApp.writeXdgListEntry(mimeType, {path});
}

QString DefaultApplications::mimeAppsListFilePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/mimeapps.list");
}
