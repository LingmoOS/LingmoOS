/*
 * Peony-Qt
 *
 * Copyright (C) 2023, KylinSoft Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef USERDIRMANAGER_H
#define USERDIRMANAGER_H

#include <QObject>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QMap>
#include <QDir>
#include <QSettings>

namespace Peony {
class UserdirManager : public QObject
{
    Q_OBJECT
public:
    explicit UserdirManager(QObject *parent = nullptr);
    void getUserdir();
    void moveFile();

Q_SIGNALS:
    void desktopDirChanged();
    void thumbnailSetingChange();

private:
    QString m_user_name;
    QString m_user_path;
    QMap<QString,QString> m_last_user_dir;
    QMap<QString,QString> m_current_user_dir;
    QFileSystemWatcher *m_user_dir_watcher;
    QDir *m_dir;
    QStringList m_file_list;
    QSettings *m_settings;
    bool m_do_not_thumbnail;
    bool m_allow_parallel;
    bool m_showTrashDialog;
    int m_times = 9;

};

}

#endif // USERDIRMANAGER_H
