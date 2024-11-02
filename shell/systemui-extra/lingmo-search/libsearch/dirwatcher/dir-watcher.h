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
#include <QDBusInterface>
#include <QDBusReply>

class DirWatcher : public QObject
{
    Q_OBJECT
public:
    static DirWatcher *getDirWatcher();

public Q_SLOTS:
    void initDbusService();

    QStringList currentIndexableDir();
    QStringList currentBlackListOfIndex();

    QStringList currentSearchableDir();
    QStringList searchableDirForSearchApplication();
    QStringList blackListOfDir(const QString &dirPath);

    QStringList getBlockDirsOfUser();

    void appendSearchDir(const QString &path);
    void removeSearchDir(const QString &path);
    void appendIndexableListItem(const QString &path);
    void removeIndexableListItem(const QString &path);
    void sendAppendSignal(const QString &path, const QStringList &blockList);
    void sendRemoveSignal(const QString&path);
Q_SIGNALS:
    void appendIndexItem(const QString&, const QStringList&);
    void removeIndexItem(const QString&);
private:
    DirWatcher(QObject *parent = nullptr);
    ~DirWatcher();

    QDBusInterface *m_dbusInterface = nullptr;
};

#endif // MOUNTDISKLISTENER_H

