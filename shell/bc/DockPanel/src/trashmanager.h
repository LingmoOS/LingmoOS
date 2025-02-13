/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     rekols <revenmartin@gmail.com>
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

#ifndef TRASHMANAGER_H
#define TRASHMANAGER_H

#include <QObject>
#include <QFileSystemWatcher>

class TrashManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit TrashManager(QObject *parent = nullptr);

    Q_INVOKABLE void moveToTrash(QList<QUrl> urls);
    Q_INVOKABLE void openTrash();
    Q_INVOKABLE void emptyTrash();

    int count() { return m_count; }

Q_SIGNALS:
    void countChanged();

private slots:
    void onDirectoryChanged();

private:
    QFileSystemWatcher *m_filesWatcher;
    int m_count;
};

#endif // TRASHMANAGER_H
