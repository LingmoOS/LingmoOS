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

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QIcon>
#include <QUrl>

class SystemAppMonitor;
class Utils : public QObject
{
    Q_OBJECT

public:
    struct AppData
    {
        QString id; // Application id (*.desktop sans extension).
        QString name; // Application name.
        QString genericName; // Generic application name.
        QIcon icon;
        QUrl url;
        bool skipTaskbar = false;
    };

    enum UrlComparisonMode {
         Strict = 0,
         IgnoreQueryItems
    };

    static Utils *instance();

    explicit Utils(QObject *parent = nullptr);

    QStringList commandFromPid(quint32 pid);
    QString desktopPathFromMetadata(const QString &appId, quint32 pid = 0,
                                    const QString &xWindowWMClassName = QString());
    QMap<QString, QString> readInfoFromDesktop(const QString &desktopFile);

private:
    SystemAppMonitor *m_sysAppMonitor;
};

#endif // UTILS_H
