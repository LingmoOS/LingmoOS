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

#ifndef MIMEAPPMANAGER_H
#define MIMEAPPMANAGER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QFileInfo>
#include <QTimer>
#include <QMap>

#include "xdgdesktopfile.h"

class QMimeType;
class MimeAppManager : public QObject
{
    Q_OBJECT

public:
    static MimeAppManager *self();
    explicit MimeAppManager(QObject *parent = nullptr);

    QStringList desktopPaths();
    QString mimeAppsListFilePath();
    void initApplications();

    QString getDefaultAppByFilePath(const QString &filePath);
    QString getDefaultAppByMimeType(const QMimeType &mimeType);
    QString getDefaultAppDesktopByMimeType(const QString &mimeType);
    Q_INVOKABLE bool setDefaultAppForType(const QString &mimeType, const QString &app);
    Q_INVOKABLE bool setDefaultAppForFile(const QString &filePath, const QString &desktop);

    QStringList getRecommendedAppsByFilePath(const QString &filePath);
    QStringList getRecommendedAppsByMimeType(const QMimeType &mimeType);

    Q_INVOKABLE QVariantList recommendedApps(const QUrl &url);

    Q_INVOKABLE void launchTerminal(const QString &path);

private slots:
    void onFileChanged(const QString &path);

private:
    QStringList m_desktopFiles;
    QMap<QString, QStringList> m_mimeApps;

    QMap<QString, XdgDesktopFile> m_videoMimeApps;
    QMap<QString, XdgDesktopFile> m_imageMimeApps;
    QMap<QString, XdgDesktopFile> m_textMimeApps;
    QMap<QString, XdgDesktopFile> m_audioMimeApps;
    QMap<QString, XdgDesktopFile> m_desktopObjects;

    QList<XdgDesktopFile> m_terminalApps;

    QFileSystemWatcher *m_fileSystemWatcher;
    QTimer *m_updateTimer;
};

#endif // MIMEAPPMANAGER_H
