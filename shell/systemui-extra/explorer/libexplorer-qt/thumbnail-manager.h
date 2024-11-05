/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include <QObject>
#include "explorer-core_global.h"
#include "file-info.h"

#include <QHash>
#include <QIcon>
#include <QMutex>

#include <QGSettings>
#include <QThread>

class QThreadPool;
class QSemaphore;

namespace Peony {

class FileWatcher;

class PEONYCORESHARED_EXPORT ThumbnailManager : public QObject
{
    friend class UpdateThemedIconJob;
    friend class ThumbnailJob;
    Q_OBJECT
public:
    static ThumbnailManager *getInstance();

    void setForbidThumbnailInView(bool forbid);

    /*!
     * \brief hasThumbnail
     * \param uri
     * \return
     * \deprecated
     */
    bool hasThumbnail(const QString &uri) {
        return !m_hash.values(uri).isEmpty();
    }

    void createThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher = nullptr, bool force = false);
    void clearThumbnail();
    void releaseThumbnail(const QString &uri);
    void releaseThumbnail(const QStringList &uris);
    void updateDesktopFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher = nullptr);
    const QIcon tryGetThumbnail(const QString &uri);

    bool hasThumbnailThreadSafety(const QString &uri);

Q_SIGNALS:
    void updateFileThumbnail();
    bool updateFileThemedIconFromThread(const QString &uri, const QString &themedIcon); //deprecated

public Q_SLOTS:
    void syncThumbnailPreferences();

protected:
    void insertOrUpdateThumbnail(const QString &uri, const QIcon &icon);

private:
    explicit ThumbnailManager(QObject *parent = nullptr);
    ~ThumbnailManager();
    void createThumbnailInternal(const QString &uri, std::shared_ptr<FileWatcher> watcher = nullptr, bool force = false);

    void createVideFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher);
    void createPdfFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher);
    void createImageFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher);
    void createOfficeFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher);
    void createDesktopFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher);

    //djvu file process
    void findAtril();
    void createImagePdfFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher);

    QHash<QString, QIcon> m_hash;
    //QMutex m_mutex;

    QThreadPool *m_thumbnail_thread_pool;
    QSemaphore *m_semaphore;

    bool m_do_not_thumbnail = false;
    QGSettings* m_thumbnail = nullptr;
};

class UpdateThemedIconJob : public QThread
{
    Q_OBJECT
public:
    UpdateThemedIconJob(const QString &uri, const QString &themeIcon, QObject *parent = nullptr);

    bool successed() const;

Q_SIGNALS:
    bool updateFileThemedIconFromThread(const QString &uri, const QString &themedIcon);

protected:
    void run() override;

private:
    QString m_uri;
    QString m_themeIconName;
    bool m_successed = false;
};

}

#endif // THUMBNAILMANAGER_H
