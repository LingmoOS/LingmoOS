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

#ifndef CAPPLICATIONS_H
#define CAPPLICATIONS_H

#include <QObject>
#include <QFileSystemWatcher>

class CAppItem
{
public:
    QString path;
    QString localName;
    QString name;
    QString comment;
    QString icon;
    QString fullExec;
    QString exec;
    QString fileName;
    QString startupWMClass;
};

class CApplications : public QObject
{
    Q_OBJECT

public:
    static CApplications *self();
    explicit CApplications(QObject *parent = nullptr);
    ~CApplications();

    CAppItem *find(const QString &fileName);
    CAppItem *matchItem(quint32 pid, const QString &windowClass);

private:
    void refresh();
    void addApplication(const QString &filePath);
    void removeApplication(CAppItem *item);
    void removeApplications(QList<CAppItem *> items);

    QStringList commandFromPid(quint32 pid);

private:
    QFileSystemWatcher *m_watcher;
    QList<CAppItem *> m_items;
};

#endif // CAPPLICATIONS_H
