/*
 * Copyright (C) 2021 LingmoOS.
 *
 * Author:     Reion Wong <reion@lingmoos.com>
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

#include "launchermodel.h"
#include "desktopproperties.h"
#include "processprovider.h"

#include <QDBusInterface>
#include <QDBusPendingCallWatcher>

#include <QtConcurrent/QtConcurrentRun>
#include <QRegularExpression>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QScopedPointer>
#include <QSettings>
#include <QDirIterator>
#include <QDebug>
#include <QIcon>
#include <QDir>

static QByteArray detectDesktopEnvironment()
{
    const QByteArray desktop = qgetenv("XDG_CURRENT_DESKTOP");

    if (!desktop.isEmpty())
        return desktop.toUpper();

    return QByteArray("UNKNOWN");
}

LauncherModel::LauncherModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_settings("lingmoos", "launcher-applist", this)
    , m_mode(NormalMode)
    , m_firstLoad(false)
{
    // Init datas.
    QByteArray listByteArray = m_settings.value("list").toByteArray();
    QDataStream in(&listByteArray, QIODevice::ReadOnly);
    in >> m_appItems;

    if (m_appItems.isEmpty())
        m_firstLoad = true;

    QtConcurrent::run(LauncherModel::refresh, this);

    m_fileWatcher->addPath("/usr/share/applications");
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &LauncherModel::onFileChanged);
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString &) {
        QtConcurrent::run(LauncherModel::refresh, this);
    });

    m_saveTimer.setInterval(1000);
    m_saveTimer.setSingleShot(true);
    connect(&m_saveTimer, &QTimer::timeout, this, &LauncherModel::save);

    connect(this, &QAbstractItemModel::rowsInserted, this, &LauncherModel::countChanged);
    connect(this, &QAbstractItemModel::rowsRemoved, this, &LauncherModel::countChanged);
    connect(this, &QAbstractItemModel::modelReset, this, &LauncherModel::countChanged);
    connect(this, &QAbstractItemModel::layoutChanged, this, &LauncherModel::countChanged);
    connect(this, &LauncherModel::refreshed, this, &LauncherModel::onRefreshed);
}

LauncherModel::~LauncherModel()
{
    LauncherModel::save();
}

int LauncherModel::count() const
{
    return rowCount();
}

int LauncherModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (m_mode == SearchMode)
        return m_searchItems.size();

    return m_appItems.size();
}

QHash<int, QByteArray> LauncherModel::roleNames() const
{
    static QHash<int, QByteArray> roles;

    if (roles.isEmpty()) {
        roles.insert(AppIdRole, "appId");
        roles.insert(ApplicationRole, "application");
        roles.insert(NameRole, "name");
        roles.insert(GenericNameRole, "genericName");
        roles.insert(CommentRole, "comment");
        roles.insert(IconNameRole, "iconName");
        roles.insert(CategoriesRole, "categories");
        roles.insert(FilterInfoRole, "filterInfo");
        roles.insert(PinnedRole, "pinned");
        roles.insert(PinnedIndexRole, "pinnedIndex");
        roles.insert(NewInstalledRole, "newInstalled");
    }

    return roles;
}

QVariant LauncherModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    AppItem appItem = m_mode == NormalMode ? m_appItems.at(index.row())
                                           : m_searchItems.at(index.row());

    switch (role) {
    case AppIdRole:
        return appItem.id;
    case NameRole:
        return appItem.name;
    case IconNameRole:
        return appItem.iconName;
    case FilterInfoRole:
        return QString(appItem.name + QStringLiteral(" ")
                       + appItem.genericName
                       + QStringLiteral(" ")
                       + appItem.comment);
    case NewInstalledRole:
        return appItem.newInstalled;
    }

    return QVariant();
}

void LauncherModel::search(const QString &key)
{
    m_mode = key.isEmpty() ? NormalMode : SearchMode;
    m_searchItems.clear();

    for (const AppItem &item : qAsConst(m_appItems)) {
        const QString &name = item.name;
        const QString &fileName = item.id;

        if (name.contains(key, Qt::CaseInsensitive) ||
                fileName.contains(key, Qt::CaseInsensitive)) {
            m_searchItems.append(item);
            continue;
        }
    }

    emit layoutChanged();
}

void LauncherModel::sendToDock(const QString &key)
{
    int index = findById(key);

    if (index != -1) {
        QDBusMessage message = QDBusMessage::createMethodCall("com.lingmo.Dock",
                                                              "/Dock",
                                                              "com.lingmo.Dock",
                                                               "add");
        message.setArguments(QList<QVariant>() << key);
        QDBusConnection::sessionBus().asyncCall(message);
    }
}

void LauncherModel::sendToDesktop(const QString &key)
{
    int index = findById(key);

    if (index != -1) {
        QFileInfo info(key);

        QString newFileName = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        newFileName.append(QString("/%1").arg(info.fileName()));

        QFile::copy(key, newFileName);
    }
}

void LauncherModel::removeFromDock(const QString &desktop)
{
    int index = findById(desktop);

    if (index != -1) {
        QDBusMessage message = QDBusMessage::createMethodCall("com.lingmo.Dock",
                                                              "/Dock",
                                                              "com.lingmo.Dock",
                                                               "remove");
        message.setArguments(QList<QVariant>() << desktop);
        QDBusConnection::sessionBus().asyncCall(message);
    }
}

int LauncherModel::findById(const QString &id)
{
    for (int i = 0; i < m_appItems.size(); ++i) {
        if (m_appItems.at(i).id == id)
            return i;
    }

    return -1;
}

void LauncherModel::refresh(LauncherModel *manager)
{
    QStringList addedEntries;
    for (const AppItem &item : qAsConst(manager->m_appItems))
        addedEntries.append(item.id);

    QStringList allEntries;

    QSettings settings(QSettings::UserScope, "lingmoos", "launcher");
    QStringList directories = settings.value("appDirectories").toStringList();

    if (directories.isEmpty()) {
        directories = QStringList{
            "/usr/share/applications",
            "/var/lib/flatpak/exports/share/applications"
        };
        settings.setValue("appDirectories", directories);
    }

    for (const QString &dir : directories) {
        QDirIterator it(dir, { "*.desktop" }, QDir::NoFilter, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            const auto fileName = it.next();
            if (!QFile::exists(fileName))
                continue;

            allEntries.append(fileName);
        }
    }

    for (const QString &fileName : allEntries) {
        QMetaObject::invokeMethod(manager, "addApp", Q_ARG(QString, fileName));
    }

    for (const AppItem &item : qAsConst(manager->m_appItems))
        if (!allEntries.contains(item.id))
            QMetaObject::invokeMethod(manager, "removeApp", Q_ARG(QString, item.id));

    // Signal the model was refreshed
    QMetaObject::invokeMethod(manager, "refreshed");
}

void LauncherModel::move(int from, int to, int page, int pageCount)
{
    if (from == to)
        return;

    int newFrom = from + (page * pageCount);
    int newTo = to + (page * pageCount);

    m_appItems.move(newFrom, newTo);

//    if (from < to)
//        beginMoveRows(QModelIndex(), from, from, QModelIndex(), to + 1);
//    else
//        beginMoveRows(QModelIndex(), from, from, QModelIndex(), to);

    //    endMoveRows();

    delaySave();
}

void LauncherModel::save()
{
    m_settings.clear();
    QByteArray datas;
    QDataStream out(&datas, QIODevice::WriteOnly);
    out << m_appItems;
    m_settings.setValue("list", datas);
}

void LauncherModel::delaySave()
{
    if (m_saveTimer.isActive())
        m_saveTimer.stop();

    m_saveTimer.start();
}

bool LauncherModel::launch(const QString &path)
{
    int index = findById(path);

    if (index != -1) {
        AppItem &item = m_appItems[index];
        QStringList args = item.args;
        QString cmd = args.takeFirst();

        if (item.newInstalled) {
            item.newInstalled = false;
            emit dataChanged(LauncherModel::index(index), LauncherModel::index(index));
            delaySave();
        }

        // Because launcher has hidden animation,
        // lingmo-screenshot needs to be processed.
        if (cmd == "lingmo-screenshot") {
            ProcessProvider::startDetached(cmd, QStringList() << "-d" << "200");
        } else {
            ProcessProvider::startDetached(cmd, args);
        }

        Q_EMIT applicationLaunched();

        return true;
    }

    return false;
}

void LauncherModel::onRefreshed()
{
    if (!m_firstLoad)
        return;

    m_firstLoad = false;

    beginResetModel();
    std::sort(m_appItems.begin(), m_appItems.end(), [=] (AppItem &a, AppItem &b) {
        return a.name < b.name;
    });
    endResetModel();

    delaySave();
}

void LauncherModel::onFileChanged(const QString &path)
{
    int index = findById(path);

    if (index == 0) {
        return;
    }

    AppItem &item = m_appItems[index];
    DesktopProperties desktop(item.id, "Desktop Entry");
    QString appName = desktop.value(QString("Name[%1]").arg(QLocale::system().name())).toString();
    QString appExec = desktop.value("Exec").toString();

    // Update datas.
    if (appName.isEmpty())
        appName = desktop.value("Name").toString();

    appExec.remove(QRegularExpression("%."));
    appExec.remove(QRegularExpression("^\""));
    appExec = appExec.replace("\"", "");
    appExec = appExec.simplified();
    item.name = appName;
    item.genericName = desktop.value("Comment").toString();
    item.comment = desktop.value("Comment").toString();
    item.iconName = desktop.value("Icon").toString();
    item.args = appExec.split(" ");

    emit dataChanged(LauncherModel::index(index), LauncherModel::index(index));
}

void LauncherModel::addApp(const QString &fileName)
{
    int index = findById(fileName);

    DesktopProperties desktop(fileName, "Desktop Entry");

    if (desktop.contains("Terminal") && desktop.value("Terminal").toBool())
        return;

    if (desktop.contains("OnlyShowIn")) {
        const QStringList items = desktop.value("OnlyShowIn").toString().split(';');

        if (!items.contains(detectDesktopEnvironment()))
            return;
    }

    if (desktop.value("NoDisplay").toBool() ||
        desktop.value("Hidden").toBool())
        return;

    QString appName = desktop.value(QString("Name[%1]").arg(QLocale::system().name())).toString();
    QString appExec = desktop.value("Exec").toString();

    if (appName.isEmpty())
        appName = desktop.value("Name").toString();

    appExec.remove(QRegularExpression("%."));
    appExec.remove(QRegularExpression("^\""));
    // appExec.remove(QRegularExpression(" *$"));
    appExec = appExec.replace("\"", "");
    appExec = appExec.simplified();

    // 存在需要更新信息
    if (index >= 0 && index <= m_appItems.size()) {
        AppItem &item = m_appItems[index];
        item.name = appName;
        item.genericName = desktop.value("Comment").toString();
        item.comment = desktop.value("Comment").toString();
        item.iconName = desktop.value("Icon").toString();
        item.args = appExec.split(" ");
        emit dataChanged(LauncherModel::index(index), LauncherModel::index(index));
    } else {
        AppItem appItem;
        appItem.id = fileName;
        appItem.name = appName;
        appItem.genericName = desktop.value("Comment").toString();
        appItem.comment = desktop.value("Comment").toString();
        appItem.iconName = desktop.value("Icon").toString();
        appItem.args = appExec.split(" ");
        appItem.newInstalled = true;

        beginInsertRows(QModelIndex(), m_appItems.count(), m_appItems.count());
        m_appItems.append(appItem);
        qDebug() << "added: " << appItem.name << appItem.newInstalled;
        endInsertRows();

        if (!m_firstLoad) {
            delaySave();
        }
    }

    // Update desktop files.
    if (!m_fileWatcher->files().contains(fileName))
        m_fileWatcher->addPath(fileName);
}

void LauncherModel::removeApp(const QString &fileName)
{
    int index = findById(fileName);
    if (index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_appItems.removeAt(index);
    endRemoveRows();

    delaySave();

    // Remove
    if (m_fileWatcher->files().contains(fileName))
        m_fileWatcher->removePath(fileName);
}
