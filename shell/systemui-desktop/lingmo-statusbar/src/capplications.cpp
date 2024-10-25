/*
 * Copyright (C) 2024 LingmoOS Team.
 *
 * Author:     lingmoos <lingmoos@foxmail.com>
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

#include "capplications.h"

#include <QRegularExpression>
#include <QSettings>
#include <QLocale>

#include <QDirIterator>
#include <QDir>

static CApplications *SELF = nullptr;
static QString s_systemAppFolder = "/usr/share/applications";

static QByteArray detectDesktopEnvironment()
{
    const QByteArray desktop = qgetenv("XDG_CURRENT_DESKTOP");

    if (!desktop.isEmpty())
        return desktop.toUpper();

    return QByteArray("UNKNOWN");
}

CApplications *CApplications::self()
{
    if (!SELF)
        SELF = new CApplications;

    return SELF;
}

CApplications::CApplications(QObject *parent)
    : QObject(parent)
    , m_watcher(new QFileSystemWatcher(this))
{
    m_watcher->addPath(s_systemAppFolder);
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this, &CApplications::refresh);
    refresh();
}

CApplications::~CApplications()
{
    while (!m_items.isEmpty())
        delete m_items.takeFirst();
}

CAppItem *CApplications::find(const QString &fileName)
{
    for (CAppItem *item : m_items)
        if (item->path == fileName)
            return item;

    return nullptr;
}

CAppItem *CApplications::matchItem(quint32 pid, const QString &windowClass)
{
    QStringList commands = commandFromPid(pid);

    // The value returned from the commandFromPid() may be empty.
    // Calling first() and last() below will cause the statusbar to crash.
    if (commands.isEmpty() || windowClass.isEmpty())
        return nullptr;

    QString command = commands.first();
    QString commandName = commands.last();

    if (command.isEmpty())
        return nullptr;

    for (CAppItem *item : m_items) {
        bool founded = false;

        // StartupWMClass
        if (item->startupWMClass.startsWith(windowClass, Qt::CaseInsensitive)) {
            founded = true;
        }

        // Command name
        if (item->fullExec == command ||
                item->exec == command ||
                item->fullExec == commandName ||
                item->exec == commandName)
            founded = true;

        // Desktop name
        if (!founded && (item->fileName == command
                         || item->fileName == commandName))
            founded = true;

        if (!founded && item->fileName.startsWith(windowClass, Qt::CaseInsensitive))
            founded = true;

        // Try matching mapped name against 'Name'.
        if (!founded && item->name.startsWith(windowClass, Qt::CaseInsensitive))
            founded = true;

        // Icon Name
        if (!founded && item->icon == command)
            founded = true;

        if (founded)
            return item;
        else
            continue;
    }

    return nullptr;
}

void CApplications::refresh()
{
    QStringList addedEntries;
    for (CAppItem *item : qAsConst(m_items)) {
        addedEntries.append(item->path);
    }

    QStringList allEntries;
    QDirIterator it(s_systemAppFolder, { "*.desktop" }, QDir::NoFilter, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        const QString &filePath = it.next();

        if (!QFile::exists(filePath))
            continue;

        allEntries.append(filePath);
    }

    for (const QString &filePath : allEntries) {
        if (!addedEntries.contains(filePath)) {
            addApplication(filePath);
        }
    }

    QList<CAppItem *> removeItems;
    for (CAppItem *item : qAsConst(m_items)) {
        if (!allEntries.contains(item->path)) {
            removeItems.append(item);
        }
    }

    removeApplications(removeItems);
}

void CApplications::addApplication(const QString &filePath)
{
    if (find(filePath))
        return;

    QSettings desktop(filePath, QSettings::IniFormat);
    desktop.setIniCodec("UTF-8");
    desktop.beginGroup("Desktop Entry");

    // Skip...
    if (desktop.contains("OnlyShowIn")) {
        const QString &value = desktop.value("OnlyShowIn").toString();
        if (!value.contains(detectDesktopEnvironment(), Qt::CaseInsensitive)) {
            return;
        }
    }

    if (desktop.value("NoDisplay").toBool() ||
        desktop.value("Hidden").toBool()) {
        return;
    }

    // Local Name
    QString localName = desktop.value(QString("Name[%1]").arg(QLocale::system().name())).toString();
    if (localName.isEmpty())
        localName = desktop.value("Name").toString();

    // Exec
    QString simplifiedExec = desktop.value("Exec").toString();
    simplifiedExec.remove(QRegularExpression("%."));
    simplifiedExec.remove(QRegularExpression("^\""));
    // appExec.remove(QRegularExpression(" *$"));
    simplifiedExec = simplifiedExec.simplified();

    // New data
    CAppItem *item = new CAppItem;
    item->path = filePath;
    item->localName = localName;
    item->name = desktop.value("Name").toString();;
    item->comment = desktop.value("Comment").toString();
    item->icon = desktop.value("Icon").toString();
    item->fullExec = desktop.value("Exec").toString();
    item->exec = simplifiedExec;
    item->fileName = QFileInfo(filePath).baseName();
    item->startupWMClass = desktop.value("StartupWMClass").toString();
    m_items.append(item);
}

void CApplications::removeApplication(CAppItem *item)
{
    int index = m_items.indexOf(item);
    if (index < 0 || index > m_items.size())
        return;

    m_items.removeAt(index);
    delete item;
}

void CApplications::removeApplications(QList<CAppItem *> items)
{
    for (CAppItem *item : items) {
        m_items.removeOne(item);
        delete item;
    }
}

QStringList CApplications::commandFromPid(quint32 pid)
{
    QFile file(QString("/proc/%1/cmdline").arg(pid));

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray cmd = file.readAll();

        // ref: https://github.com/KDE/kcoreaddons/blob/230c98aa7e01f9e36a9c2776f3633182e6778002/src/lib/util/kprocesslist_unix.cpp#L137
        if (!cmd.isEmpty()) {
            // extract non-truncated name from cmdline
            int zeroIndex = cmd.indexOf('\0');
            int processNameStart = cmd.lastIndexOf('/', zeroIndex);
            if (processNameStart == -1) {
                processNameStart = 0;
            } else {
                processNameStart++;
            }

            QString name = QString::fromLocal8Bit(cmd.mid(processNameStart, zeroIndex - processNameStart));

            // reion: Remove parameters
            name = name.split(' ').first();

            cmd.replace('\0', ' ');
            QString command = QString::fromLocal8Bit(cmd).trimmed();

            // There may be parameters.
            if (command.split(' ').size() > 1) {
                command = command.split(' ').first();
            }

            return { command, name };
        }
    }

    return QStringList();
}

