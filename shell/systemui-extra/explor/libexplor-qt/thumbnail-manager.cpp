/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2019-2020, Tianjin LINGMO Information Technology Co., Ltd.
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
 *          Burgess Chang <brs@sdf.org>
 *
 */

#include "thumbnail-manager.h"

#include "file-info-manager.h"

#include "file-watcher.h"
#include "file-utils.h"

#include "thumbnail/pdf-thumbnail.h"
#include "thumbnail/video-thumbnail.h"
#include "thumbnail/office-thumbnail.h"
#include "thumbnail/image-pdf-thumbnail.h"
#include "generic-thumbnailer.h"
#include "thumbnail-job.h"

#include "global-settings.h"

#include <QtConcurrent>
#include <QIcon>
#include <QUrl>

#include <QThreadPool>
#include <QSemaphore>

#include <QGuiApplication>

#include <gio/gdesktopappinfo.h>

using namespace Peony;

static ThumbnailManager *global_instance = nullptr;
static bool m_tril_exist = false;

static QThreadPool *desktop_thumbnail_thread_pool = nullptr;

/*!
 * \brief ThumbnailManager::ThumbnailManager
 * \param parent
 * \bug
 * thumbnail will do i/o on the file. if we write on a pictrue and save
 * save it, the image editor might report a modified error due to we hold
 * the file in explor-qt.
 *
 * this bug is not critical, but i have to consider current thumbnailer
 * might be a bad desgin.
 */
ThumbnailManager::ThumbnailManager(QObject *parent) : QObject(parent)
{
    desktop_thumbnail_thread_pool = new QThreadPool();

    GlobalSettings::getInstance();

    m_thumbnail_thread_pool = new QThreadPool(this);
    m_thumbnail_thread_pool->setMaxThreadCount(1);

    m_semaphore = new QSemaphore(1);

    findAtril();

    connect(qApp, &QGuiApplication::lastWindowClosed, this, [=]{
        clearThumbnail();
        m_thumbnail_thread_pool->clear();
        m_thumbnail_thread_pool->waitForDone(500);
    });

    m_thumbnail = new QGSettings("org.lingmo.explor.settings", QByteArray(), this);
    connect(m_thumbnail, &QGSettings::changed, this, [=](const QString &key) {
        if (FORBID_THUMBNAIL_IN_VIEW == key) {
            auto settings = Peony::GlobalSettings::getInstance();
            if (m_do_not_thumbnail != settings->getValue(FORBID_THUMBNAIL_IN_VIEW).toBool()) {
                m_do_not_thumbnail = settings->getValue(FORBID_THUMBNAIL_IN_VIEW).toBool();
                if (true == m_do_not_thumbnail) {
                    Peony::ThumbnailManager::getInstance()->clearThumbnail();
                }
                Q_EMIT updateFileThumbnail();
            }
        }
    });

//    connect(this, &ThumbnailManager::updateFileThemedIconFromThread, this, [=](const QString &uri, const QString &themedIcon){
//        auto icon = QIcon::fromTheme(themedIcon);
//        if (icon.isNull()) {
//            return false;
//        }
//        this->insertOrUpdateThumbnail(uri, icon);
//        return true;
//    }, Qt::BlockingQueuedConnection);
}

ThumbnailManager::~ThumbnailManager()
{
    delete m_semaphore;
}

ThumbnailManager *ThumbnailManager::getInstance()
{
    if (!global_instance)
        global_instance = new ThumbnailManager;
    return global_instance;
}

void ThumbnailManager::syncThumbnailPreferences()
{
    GlobalSettings::getInstance()->forceSync(FORBID_THUMBNAIL_IN_VIEW);
}

void ThumbnailManager::insertOrUpdateThumbnail(const QString &uri, const QIcon &icon)
{
    m_semaphore->acquire();
    m_hash.remove(uri);
    m_hash.insert(uri, icon);
    m_semaphore->release();
}

void ThumbnailManager::setForbidThumbnailInView(bool forbid)
{
    GlobalSettings::getInstance()->setValue(FORBID_THUMBNAIL_IN_VIEW, forbid);
}

void ThumbnailManager::createVideFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher)
{
    QIcon thumbnail;

    VideoThumbnail videoThumbnail(uri);
    thumbnail = videoThumbnail.generateThumbnail();
    if (!thumbnail.isNull()) {
        insertOrUpdateThumbnail(uri, thumbnail);
        if (watcher) {
            watcher->fileChanged(uri);
        }
    }

    return;
}

void ThumbnailManager::createImagePdfFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher)
{
    QIcon thumbnail;

    ImagePdfThumbnail officeThumbnail(uri);
//    ThumbnailManager::getInstance()->updateFileThemedIconFromThread(uri, "atril");
    auto job = new UpdateThemedIconJob(uri, "atril");
    job->start();
    job->wait();
    delete job;
    if (watcher) {
        watcher->fileChanged(uri);
    }

//    thumbnail = officeThumbnail.generateThumbnail();;
//    if (!thumbnail.isNull()) {
//        insertOrUpdateThumbnail(uri, thumbnail);
//        if (watcher) {
//            watcher->fileChanged(uri);
//        }
//    }

    return;
}

void ThumbnailManager::createPdfFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher)
{
    QIcon thumbnail;
    QUrl url = uri;

    if (!uri.startsWith("file:///")) {
        url = FileUtils::getTargetUri(uri);
        //qDebug()<<url;
    }

    PdfThumbnail pdfThumbnail(url.path());
    QPixmap pix = pdfThumbnail.generateThumbnail();

    thumbnail = GenericThumbnailer::generateThumbnail(pix, true);
    if (!thumbnail.isNull()) {
        insertOrUpdateThumbnail(uri, thumbnail);
        if (watcher) {
            watcher->fileChanged(uri);
        }
    }

    return;
}
void ThumbnailManager::createImageFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher)
{
    QUrl url = uri;

    if (!uri.startsWith("file:///")) {
        url = FileUtils::getTargetUri(uri);
        //qDebug()<<url;
    }

    QIcon thumbnail = GenericThumbnailer::generateThumbnail(url.path(), true);
    if (!thumbnail.isNull()) {
        insertOrUpdateThumbnail(uri, thumbnail);
        if (watcher) {
            watcher->fileChanged(uri);
        }
    } else if (uri.startsWith("gphoto2://") || uri.startsWith("mtp://")) {
        //手机传输和图片传输需要重定向path后获取对应缩略图
        auto fileInfo = FileInfo::fromUri(uri);
        QIcon thumbnail = GenericThumbnailer::generateThumbnail(fileInfo.get()->filePath(), true);
        if (!thumbnail.isNull()) {
            insertOrUpdateThumbnail(uri, thumbnail);
            if (watcher) {
                watcher->fileChanged(uri);
            }
        }
    }

    //qApp->processEvents();
    return;
}

void ThumbnailManager::createOfficeFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher)
{
    QIcon thumbnail;

    OfficeThumbnail officeThumbnail(uri);
    thumbnail = officeThumbnail.generateThumbnail();;
    if (!thumbnail.isNull()) {
        insertOrUpdateThumbnail(uri, thumbnail);
        if (watcher) {
            watcher->fileChanged(uri);
        }
    }

    return;
}

void ThumbnailManager::createDesktopFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher)
{
    QIcon thumbnail;
    QUrl url = uri;
    QString path = url.path();

    if (!uri.startsWith("file:///")) {
        g_autoptr (GFile) gfile = g_file_new_for_uri(uri.toUtf8().constData());
        g_autoptr (GFileInfo) gfileinfo = g_file_query_info(gfile, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI, G_FILE_QUERY_INFO_NONE, 0, 0);
        g_autofree gchar *target_uri = g_file_info_get_attribute_as_string(gfileinfo, G_FILE_ATTRIBUTE_STANDARD_TARGET_URI);
        if (target_uri) {
            url = QString(target_uri);
        }
    }

    QString string;
    g_autoptr (GDesktopAppInfo) desktop_app_info = g_desktop_app_info_new_from_filename(path.toUtf8().constData());
    if (desktop_app_info) {
        auto app_info = G_APP_INFO(desktop_app_info);
        GIcon *icon = g_app_info_get_icon(app_info);
        string = FileUtils::getIconStringFromGIconThreadSafety(icon);
    }

    if (string.isEmpty()) {
        string = url.fileName().remove(".desktop");
        auto key_file = g_key_file_new();
        if (g_key_file_load_from_file(key_file, path.toUtf8().constData(), G_KEY_FILE_NONE, 0)) {
            g_autofree gchar* icon_name = g_key_file_get_value(key_file, G_KEY_FILE_DESKTOP_GROUP, G_KEY_FILE_DESKTOP_KEY_ICON, 0);
            if (icon_name) {
                string = icon_name;
            }
            g_key_file_free(key_file);
        } else {
            qWarning()<<"failed to load desktop file";
            return;
        }
    }

    if (string.startsWith("/")) {
        thumbnail = GenericThumbnailer::generateThumbnail(string, true);
        insertOrUpdateThumbnail(uri, thumbnail);
        if (watcher) {
            watcher->fileChanged(uri);
        }
        return;
    } else {
        if (string.endsWith(".jpg") || string.endsWith(".jpeg") || string.endsWith(".png") || string.endsWith(".svg")) {
            string.chop(string.count() - string.lastIndexOf("."));
        }
    }

    auto job = new UpdateThemedIconJob(uri, string);
    job->start();
    job->wait();
//    bool successed = ThumbnailManager::getInstance()->updateFileThemedIconFromThread(uri, string);
    bool successed = job->successed();
    successed = !thumbnail.isNull() || successed;
    delete job;

    //fix desktop file set customer icon issue, link to bug#77638
//    auto info = FileInfo::fromUri(uri);
//    if (! info->customIcon().isEmpty()){
//        thumbnail = GenericThumbnailer::generateThumbnail(info->customIcon(), true);
//    }

    //add special path search /use/share/pixmaps
    if (!successed)
    {
        QString path = QString("/usr/share/pixmaps/%1.%2").arg(string).arg("png");
        QString path_svg = QString("/usr/share/pixmaps/%1.%2").arg(string).arg("svg");
        //qDebug() << "createDesktopFileThumbnail path:" <<path;
        if(QFile::exists(path)){
            thumbnail=QIcon(path);
        }
        else if(QFile::exists(path_svg)){
            thumbnail=QIcon(path_svg);
        }
        else{
            //search /usr/share/icons/hicolor/scalable/apps
            //fix installed app desktop icon not loaded in time issue
            path_svg = QString("/usr/share/icons/hicolor/scalable/apps/%1.%2").arg(string).arg("svg");
            if(QFile::exists(path_svg))
            {
               thumbnail=QIcon(path_svg);
            }
        }

        //still not find icon, default find 64*64 png
        //link to bug#69429, after install app not show icon issue
        if (thumbnail.isNull())
        {
            path = QString("/usr/share/icons/hicolor/64x64/apps/%1.%2").arg(string).arg("png");
            if(QFile::exists(path)){
                thumbnail=QIcon(path);
            }
        }

        if (thumbnail.isNull()) {
            path = QString("/usr/share/lingmo-software-center/data/icons/%1.%2").arg(string).arg("png");
            if(QFile::exists(path)){
                thumbnail=QIcon(path);
            }
        }
    } else {
        if (watcher) {
            watcher->fileChanged(uri);
        }
        return;
    }

    if (!thumbnail.isNull()) {
        insertOrUpdateThumbnail(uri, thumbnail);
        if (watcher) {
            watcher->fileChanged(uri);
        }
    } else {
        qWarning()<<"can not pharse desktop file"<<uri;
    }

    return;
}

//is system has atril software
void ThumbnailManager::findAtril()
{
    QtConcurrent::run([](){
        GList *infos = g_app_info_get_all();
        GList *l = infos;
        while (l && ! m_tril_exist) {
            const char *cmd = g_app_info_get_executable(static_cast<GAppInfo*>(l->data));
            QString tmp = cmd;
            if (tmp.contains("atril")) {
                m_tril_exist = true;
            }
            l = l->next;
        }
        g_list_free_full(infos, g_object_unref);
    });
}

void ThumbnailManager::createThumbnailInternal(const QString &uri, std::shared_ptr<FileWatcher> watcher, bool force)
{
    // deprecated
}

void ThumbnailManager::createThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher, bool force)
{
    //qDebug() <<"createThumbnail:" <<force<<uri;
    auto thumbnail = tryGetThumbnail(uri);
    if (!thumbnail.isNull()) {
        if (!force) {
            watcher->thumbnailUpdated(uri);
            watcher->fileChanged(uri);
            qDebug() <<"createThumbnail return:" <<uri;
            return;
        }
    }

    // check if need thumbnail
    bool needThumbnail = false;

    auto info = FileInfo::fromUri(uri);

    bool hasCustomIcon = false;
    if (!info->customIcon().isEmpty() /*&& info->customIcon().startsWith("/")*/) {
        needThumbnail = true;
        hasCustomIcon = true;
    }

    if (!info->mimeType().isEmpty()) {
        if (info->isImageFile()) {
            needThumbnail = true;
        }
        else if (info->mimeType().contains("pdf")) {
            needThumbnail = true;
        }
        else if(info->isVideoFile()) {
            needThumbnail = true;
        }
        else if (info->isOfficeFile()) {
            needThumbnail = true;
        }
        else if (info->uri().endsWith(".desktop")) {
            if (thumbnail.isNull())
            {
                needThumbnail = false;
                updateDesktopFileThumbnail(uri, watcher);
            }
            else
                needThumbnail = true;
        }
    }

    if (!needThumbnail)
        return;

    auto thumbnailJob = new ThumbnailJob(uri, watcher, this);
    thumbnailJob->setForceUpdate(force);
    m_thumbnail_thread_pool->start(thumbnailJob, hasCustomIcon? QThread::HighestPriority: 0);
    qDebug() <<"createThumbnail thumbnailJob start:" <<uri;
}

void ThumbnailManager::updateDesktopFileThumbnail(const QString &uri, std::shared_ptr<FileWatcher> watcher)
{
    auto info = FileInfo::fromUri(uri);
    if (info->uri().endsWith(".desktop")) {
        //qDebug()<<"is desktop file"<<uri;
        //get desktop file icon.
        //async
        //qDebug()<<"desktop file"<<uri;
        auto thumbnailJob = new ThumbnailJob(uri, watcher, this);
        desktop_thumbnail_thread_pool->start(thumbnailJob, QThread::Priority::HighestPriority);
    } else {
        releaseThumbnail(uri);
        if (watcher) {
            watcher->thumbnailUpdated(uri);
        }
    }
}

void ThumbnailManager::clearThumbnail()
{
    m_semaphore->acquire();
    if (!m_hash.isEmpty()) {
        m_hash.clear();
    }
    m_semaphore->release();
}

void ThumbnailManager::releaseThumbnail(const QString &uri)
{
    m_semaphore->acquire();
    m_hash.remove(uri);
    m_semaphore->release();
}

void ThumbnailManager::releaseThumbnail(const QStringList &uris)
{
    m_semaphore->acquire();
    for (auto uri : uris) {
        m_hash.remove(uri);
    }
    m_semaphore->release();
}

const QIcon ThumbnailManager::tryGetThumbnail(const QString &uri)
{
    m_semaphore->acquire();
    auto icon = m_hash.value(uri);
    m_semaphore->release();
    return icon;
}

bool ThumbnailManager::hasThumbnailThreadSafety(const QString &uri)
{
    m_semaphore->acquire();
    bool res = hasThumbnail(uri);
    m_semaphore->release();
    return res;
}

UpdateThemedIconJob::UpdateThemedIconJob(const QString &uri, const QString &themeIcon, QObject *parent) : QThread(parent)
{
    m_uri = uri;
    m_themeIconName = themeIcon;

    connect(this, &UpdateThemedIconJob::updateFileThemedIconFromThread, ThumbnailManager::getInstance(), [=](const QString &uri, const QString &themeIcon){
        auto icon = QIcon::fromTheme(themeIcon);
        if (icon.isNull()) {
            return false;
        }
        ThumbnailManager::getInstance()->insertOrUpdateThumbnail(uri, icon);
        return true;
    }, Qt::BlockingQueuedConnection);
}

void UpdateThemedIconJob::run()
{
    m_successed = updateFileThemedIconFromThread(m_uri, m_themeIconName);
}

bool UpdateThemedIconJob::successed() const
{
    return m_successed;
}
