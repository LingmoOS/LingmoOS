// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_Platform_THUMBNAIL_WORKER_H
#define _DMR_Platform_THUMBNAIL_WORKER_H

#include <QtWidgets>
#include <libffmpegthumbnailer/videothumbnailerc.h>
#include <atomic>
#include <mutex>


typedef video_thumbnailer *(*mvideo_thumbnailer)();
typedef void (*mvideo_thumbnailer_destroy)(video_thumbnailer *thumbnailer);
/* create image_data structure */
typedef image_data *(*mvideo_thumbnailer_create_image_data)(void);
/* destroy image_data structure */
typedef void (*mvideo_thumbnailer_destroy_image_data)(image_data *data);
typedef int (*mvideo_thumbnailer_generate_thumbnail_to_buffer)(video_thumbnailer *thumbnailer, const char *movie_filename, image_data *generated_image_data);

namespace dmr {
//using namespace ffmpegthumbnailer;

class PlayerEngine;

class Platform_ThumbnailWorker: public QThread
{
    Q_OBJECT
public:
    ~Platform_ThumbnailWorker();
    static Platform_ThumbnailWorker &get();
    // expected size for ui
    static QSize thumbSize()
    {
        return {178, 101};
    }
    bool isThumbGenerated(const QUrl &url, int secs);
    QPixmap getThumb(const QUrl &url, int secs);
    void stop()
    {
        _quit.store(1);
        quit();
    }
    void setPlayerEngine(PlayerEngine *pPlayerEngline);
public slots:
    void requestThumb(const QUrl &url, int secs);

signals:
    void thumbGenerated(const QUrl &url, int secs);

private:
    QList<QPair<QUrl, int>> _wq;
    QHash<QUrl, QMap<int, QPixmap>> _cache;
    QAtomicInt _quit{0};
    qint64 _cacheSize {0};
    video_thumbnailer *m_video_thumbnailer = nullptr;
    image_data *m_image_data = nullptr;
    PlayerEngine *_engine {nullptr};
    mvideo_thumbnailer m_mvideo_thumbnailer = nullptr;
    mvideo_thumbnailer_destroy m_mvideo_thumbnailer_destroy = nullptr;
    mvideo_thumbnailer_create_image_data m_mvideo_thumbnailer_create_image_data = nullptr;
    mvideo_thumbnailer_destroy_image_data m_mvideo_thumbnailer_destroy_image_data = nullptr;
    mvideo_thumbnailer_generate_thumbnail_to_buffer m_mvideo_thumbnailer_generate_thumbnail_to_buffer = nullptr;
    char *m_pCharTime;

    Platform_ThumbnailWorker();
    void initThumb();
    void run() override;
    void runSingle(QPair<QUrl, int> w);
    QPixmap genThumb(const QUrl &url, int secs);
    QString libPath(const QString &strlib);

private:
    static std::atomic<Platform_ThumbnailWorker *> m_instance;
    static QMutex m_instLock;
    static QMutex m_thumbLock;
    static QWaitCondition m_cond;
};

}

#endif /* ifndef _DMR_Platform_THUMBNAIL_WORKER_H */
