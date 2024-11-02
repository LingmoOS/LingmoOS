/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */
#ifndef MOUNTDISKLISTENER_H
#define MOUNTDISKLISTENER_H

#include <QObject>
#include <QStringList>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QMutex>

class DirWatcher : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface","org.lingmo.search.fileindex")

public:
    static DirWatcher *getDirWatcher();

public Q_SLOTS:

    void initDbusService();

    QStringList currentIndexableDir();
    QStringList currentBlackListOfIndex();

    QStringList currentSearchableDir();
    QStringList searchableDirForSearchApplication();
    QStringList blackListOfDir(const QString &dirPath);

    /**
     * @brief DirWatcher::appendIndexableListItem
     * add a item to indexable dirs
     * @param path: the path to be added to the index dirs list
     * @return int: the result code
     *  0: successful
     *  1: path or its parent dir has been added
     *  2: path is or under blacklist
     *  3: path is in repeat mounted devices and another path which is in the same device has been indexed
     *  4: another path which is in the same device has been indexed
     *  5: path is not exists
     */
    Q_SCRIPTABLE int appendIndexableListItem(const QString &path);
    Q_SCRIPTABLE bool removeIndexableListItem(const QString &path);

    //新接口
    Q_SCRIPTABLE int appendSearchDir(const QString &path);
    Q_SCRIPTABLE void removeSearchDir(const QString &path);

    QStringList currentSearchDirs();
    QStringList currentBlackList();

    QStringList blockDirsForUser();
    int addBlockDirOfUser(const QString& dir);
    void removeBlockDirOfUser(const QString& dir);

private:
    DirWatcher(QObject *parent = nullptr);
    ~DirWatcher();

    static QMutex s_mutex;

Q_SIGNALS:
    void appendIndexItem(const QString&, const QStringList&);
    void removeIndexItem(const QString&);
    //abondoned
    void udiskRemoved();
    void indexDirsChanged();
};

#endif // MOUNTDISKLISTENER_H

