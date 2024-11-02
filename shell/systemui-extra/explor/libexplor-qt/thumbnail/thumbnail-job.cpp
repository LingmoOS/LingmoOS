/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, Tianjin LINGMO Information Technology Co., Ltd.
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

#include "thumbnail-job.h"

#include "thumbnail-manager.h"

#include "file-watcher.h"
#include "file-info.h"
#include "file-utils.h"

#include "global-settings.h"
#include "generic-thumbnailer.h"
#include "volumeManager.h"

#include <QApplication>
#include <QDebug>

static int runCount = 0;
static int endCount = 0;

Peony::ThumbnailJob::ThumbnailJob(const QString &uri, const std::shared_ptr<Peony::FileWatcher> watcher, QObject *parent):
    QObject(parent), QRunnable()
{
    m_uri = uri;
    m_watcher = watcher;
    if (watcher) {
        if (watcher->parent()) {
            setParent(watcher->parent());
        }
    }

    setAutoDelete(true);

    auto info = FileInfo::fromUri(uri);
    if (!info->mimeType().isEmpty()) {
        setProperty("mimeType", info->mimeType());
    }
    if (!info->customIcon().isEmpty()) {
        setProperty("customIcon", info->customIcon());
        setType(CustomIcon);
        return;
    }

    auto settings = Peony::GlobalSettings::getInstance();
    if (settings->isExist(FORBID_THUMBNAIL_IN_VIEW)) {
        bool do_not_thumbnail = settings->getValue(FORBID_THUMBNAIL_IN_VIEW).toBool();
        if (do_not_thumbnail) {
            setType(Invalid);
            return;
        }
    }
}

Peony::ThumbnailJob::~ThumbnailJob()
{
    endCount++;
    //qDebug()<<"job end or cancelled. current end"<<endCount<<"current start request:"<<runCount;
}

void Peony::ThumbnailJob::run()
{
    /* 移动设备弹出时被ffmpeg占用时，强制弹出过程中防止该设备的文件仍继续使用ffmpeg，link to bug#117263 */
    {
        auto mutex = Experimental_Peony::VolumeManager::getInstance()->getMutex();
        QMutexLocker lk(mutex);
        auto occupiedVolume = Experimental_Peony::VolumeManager::getInstance()->getOccupiedVolume();
        if(occupiedVolume){
            QString volumeUri = Experimental_Peony::VolumeManager::getInstance()->getTargetUriFromUnixDevice(occupiedVolume->device());
            qDebug()<<occupiedVolume->device()<<volumeUri<<m_uri;
            if(m_uri.startsWith(volumeUri))
                return;
        }
    }//end

    if (!parent())
        return;

    // if all window closed, should not do a thumbnail job.
    if (qApp->topLevelWindows().count() == 0) {
        return;
    }
    if (type() == Invalid && !m_uri.endsWith(".desktop")|| !Peony::FileUtils::isFileExsit(m_uri)) {
        return;
    }

    runCount++;

    qDebug()<<"job start, current end:"<<endCount<<"current start request:"<<runCount<<"uri:"<<m_uri;

    setParent(nullptr);
    auto strongPtr = m_watcher.lock();

    if (!isForceUpdate()) {
        if (ThumbnailManager::getInstance()->hasThumbnailThreadSafety(m_uri)) {
            return;
        }
    }

    QString mimeType = property("mimeType").toString();
    if (mimeType.isEmpty()) {
        qWarning()<<"can not get file mime type:"<<m_uri;
        return;
    }

    if (mimeType.startsWith("image/")) {
        setType(Image);
    } else if (mimeType.contains("pdf")) {
        setType(Pdf);
    } else if (m_uri.endsWith(".desktop")) {
        setType(Desktop);
    } else if (mimeType.contains("djvu")) {
        setType(ImagePdf);
    } else if (mimeType.startsWith("video")
               || mimeType.endsWith("vnd.trolltech.linguist")
               || mimeType.endsWith("vnd.adobe.flash.movie")
               || mimeType.endsWith("vnd.rn-realmedia")
               || mimeType.endsWith("vnd.ms-asf")
               || mimeType.endsWith("octet-stream")) {
        setType(Video);
    } else {
        int idx = 0;
        QString mtype = nullptr;

        for (idx = 0; Peony::office_mime_types[idx] != "end"; idx++)
        {
            mtype = Peony::office_mime_types[idx];
            if (mimeType.contains(mtype))
            {
                setType(Office);
                break;
            }
        }
    }

    QString customIcon = property("customIcon").toString();
    if (!customIcon.isEmpty()) {
        setType(CustomIcon);
    }

    if (strongPtr.get()) {
//        QString uri = m_uri;
//        ThumbnailManager::getInstance()->createThumbnailInternal(uri, strongPtr);
        switch (type()) {
        case Image: {
            ThumbnailManager::getInstance()->createImageFileThumbnail(m_uri, strongPtr);
            break;
        }
        case Pdf: {
            ThumbnailManager::getInstance()->createPdfFileThumbnail(m_uri, strongPtr);
            break;
        }
        case ImagePdf: {
            ThumbnailManager::getInstance()->createImagePdfFileThumbnail(m_uri, strongPtr);
            break;
        }
        case Video: {
            ThumbnailManager::getInstance()->createVideFileThumbnail(m_uri, strongPtr);
            break;
        }
        case Office: {
            ThumbnailManager::getInstance()->createOfficeFileThumbnail(m_uri, strongPtr);
            break;
        }
        case Desktop: {
            ThumbnailManager::getInstance()->createDesktopFileThumbnail(m_uri, strongPtr);
            break;
        }
        case CustomIcon: {
            auto icon = GenericThumbnailer::generateThumbnail(customIcon);
            if (!icon.isNull()) {
                ThumbnailManager::getInstance()->insertOrUpdateThumbnail(m_uri, icon);
                if (strongPtr) {
                    strongPtr->fileChanged(m_uri);
                }
            }
            break;
        }
        default: {
            break;
        }
        }
    }
}

void Peony::ThumbnailJob::setForceUpdate(bool force)
{
    setProperty("forceUpdate", force);
}

bool Peony::ThumbnailJob::isForceUpdate()
{
    return property("forceUpdate").toBool();
}

void Peony::ThumbnailJob::setType(Peony::ThumbnailJob::Type type)
{
    setProperty("type", static_cast<int>(type));
}

Peony::ThumbnailJob::Type Peony::ThumbnailJob::type()
{
    return static_cast<Type>(property("type").toInt());
}
