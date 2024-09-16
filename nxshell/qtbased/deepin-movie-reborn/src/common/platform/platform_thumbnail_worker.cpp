// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "platform_thumbnail_worker.h"
#include "player_engine.h"
#include <QLibrary>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "compositing_manager.h"

#define SIZE_THRESHOLD (10 * 1<<20)

namespace dmr {
std::atomic<Platform_ThumbnailWorker *> Platform_ThumbnailWorker::m_instance(nullptr);
QMutex Platform_ThumbnailWorker::m_instLock;
QMutex Platform_ThumbnailWorker::m_thumbLock;
QWaitCondition Platform_ThumbnailWorker::m_cond;

Platform_ThumbnailWorker::~Platform_ThumbnailWorker()
{
    free(m_pCharTime);
    if (m_video_thumbnailer) {
        m_mvideo_thumbnailer_destroy(m_video_thumbnailer);
    }
}

Platform_ThumbnailWorker &Platform_ThumbnailWorker::get()
{
    if (m_instance == nullptr) {
        QMutexLocker lock(&m_instLock);
        m_instance = new Platform_ThumbnailWorker;
        if(CompositingManager::get().platform() != Platform::Mips) {
            (*m_instance).start();
        }
    }
    return *m_instance;
}

bool Platform_ThumbnailWorker::isThumbGenerated(const QUrl &url, int secs)
{
    QMutexLocker lock(&m_thumbLock);
    if (!_cache.contains(url)) return false;

    const auto &l = _cache[url];
    return l.contains(secs);
}

QPixmap Platform_ThumbnailWorker::getThumb(const QUrl &url, int secs)
{
    QMutexLocker lock(&m_thumbLock);
    QPixmap pm;

    if (_cache.contains(url)) {
        pm = _cache[url].value(secs);
    }

    return pm;
}

void Platform_ThumbnailWorker::setPlayerEngine(PlayerEngine *pPlayerEngline)
{
    _engine = pPlayerEngline;
}

void Platform_ThumbnailWorker::requestThumb(const QUrl &url, int secs)
{
    if(CompositingManager::get().platform() != Platform::Mips) {
        if (m_thumbLock.tryLock()) {
            _wq.push_front(qMakePair(url, secs));
            m_cond.wakeOne();
            m_thumbLock.unlock();
        }
    } else {
        runSingle(qMakePair(url, secs));
    }
}

Platform_ThumbnailWorker::Platform_ThumbnailWorker()
{
    initThumb();
    m_video_thumbnailer->thumbnail_size = m_video_thumbnailer->thumbnail_size * qApp->devicePixelRatio();

    m_pCharTime = (char *)malloc(20);
}

QString Platform_ThumbnailWorker::libPath(const QString &strlib)
{
    QDir  dir;
    QString lib_path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(lib_path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.contains(strlib)) {
        return strlib;
    } else {
        list.sort();
    }

    Q_ASSERT(list.size() > 0);
    return list.last();
}

void Platform_ThumbnailWorker::initThumb()
{
    QLibrary library(CompositingManager::libPath("libffmpegthumbnailer.so"));
    m_mvideo_thumbnailer = (mvideo_thumbnailer) library.resolve("video_thumbnailer_create");
    m_mvideo_thumbnailer_destroy = (mvideo_thumbnailer_destroy) library.resolve("video_thumbnailer_destroy");
    m_mvideo_thumbnailer_create_image_data = (mvideo_thumbnailer_create_image_data) library.resolve("video_thumbnailer_create_image_data");
    m_mvideo_thumbnailer_destroy_image_data = (mvideo_thumbnailer_destroy_image_data) library.resolve("video_thumbnailer_destroy_image_data");
    m_mvideo_thumbnailer_generate_thumbnail_to_buffer = (mvideo_thumbnailer_generate_thumbnail_to_buffer) library.resolve("video_thumbnailer_generate_thumbnail_to_buffer");
    m_video_thumbnailer = m_mvideo_thumbnailer();
    if (m_mvideo_thumbnailer == nullptr || m_mvideo_thumbnailer_destroy == nullptr
            || m_mvideo_thumbnailer_create_image_data == nullptr || m_mvideo_thumbnailer_destroy_image_data == nullptr
            || m_mvideo_thumbnailer_generate_thumbnail_to_buffer == nullptr
            || m_video_thumbnailer == nullptr) {
        return;
    }
}

QPixmap Platform_ThumbnailWorker::genThumb(const QUrl &url, int secs)
{
    auto dpr = qApp->devicePixelRatio();
    QPixmap pm;
    pm.setDevicePixelRatio(dpr);

    if (m_image_data == nullptr) {
        m_image_data = m_mvideo_thumbnailer_create_image_data();
    }

    QTime d(0, 0, 0);
    d = d.addSecs(secs);
    strcpy(m_pCharTime, d.toString("hh:mm:ss").toLatin1().data());
    m_video_thumbnailer->seek_time = m_pCharTime;
    auto file = QFileInfo(url.toLocalFile()).absoluteFilePath();
    try {
        m_mvideo_thumbnailer_generate_thumbnail_to_buffer(m_video_thumbnailer, file.toUtf8().data(),  m_image_data);
        auto img = QImage::fromData(m_image_data->image_data_ptr, static_cast<int>(m_image_data->image_data_size), "png");

        pm = QPixmap::fromImage(img.scaled(thumbSize() * dpr, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        pm.setDevicePixelRatio(dpr);
    } catch (const std::logic_error &e) {
    }

    m_mvideo_thumbnailer_destroy_image_data(m_image_data);
    m_image_data = nullptr;

    return pm;
}

//cppcheck 误报
void Platform_ThumbnailWorker::run()
{
    setPriority(QThread::IdlePriority);
    while (!_quit.load()) {

        QPair<QUrl, int> w;
        {
            QMutexLocker lock(&m_thumbLock);
            while (_wq.isEmpty() && !_quit.load()) {
                m_cond.wait(lock.mutex(), 40);
            }

            if (!_wq.isEmpty()) {
                w = _wq.takeFirst();
                _wq.clear();
            }
        }

        if (_quit.load()) break;

        {
            QMutexLocker lock(&m_thumbLock);
            //TODO: optimize: need a lru map
            if (_cacheSize > SIZE_THRESHOLD) {
                qInfo() << "thumb cache size exceeds maximum, clean up";
                _cache.clear();
                _cacheSize = 0;
            }
        }

        if (!isThumbGenerated(w.first, w.second)) {
            auto pm = genThumb(w.first, w.second);

            QMutexLocker lock(&m_thumbLock);
            _cache[w.first].insert(w.second, pm);
            _cacheSize += pm.width() * pm.height() * (pm.hasAlpha() ? 4 : 3);

            QTime d(0, 0, 0);
            d = d.addSecs(w.second);
            qInfo() << "thumb for " << w.first << d.toString("hh:mm:ss");
        }

        emit thumbGenerated(w.first, w.second);
    }

    _wq.clear();
}

void Platform_ThumbnailWorker::runSingle(QPair<QUrl, int> w)
{
    if (_cacheSize > SIZE_THRESHOLD) {
        qInfo() << "thumb cache size exceeds maximum, clean up";
        _cache.clear();
        _cacheSize = 0;
    }

    if (!isThumbGenerated(w.first, w.second)) {
        auto pm = genThumb(w.first, w.second);

        QMutexLocker lock(&m_thumbLock);
        _cache[w.first].insert(w.second, pm);
        _cacheSize += pm.width() * pm.height() * (pm.hasAlpha() ? 4 : 3);

        QTime d(0, 0, 0);
        d = d.addSecs(w.second);
        qInfo() << "thumb for " << w.first << d.toString("hh:mm:ss");
    }

    emit thumbGenerated(w.first, w.second);
}
}

