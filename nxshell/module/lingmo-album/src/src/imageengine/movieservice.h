// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOVIESERVICE_H
#define MOVIESERVICE_H

#include <QObject>
#include <QMap>
#include <QUrl>
#include <QFileInfo>
#include <QMutex>
#include <mutex>
#include <QDateTime>
#include <deque>
#include <QImage>
#include <libffmpegthumbnailer/videothumbnailerc.h>

typedef video_thumbnailer *(*mvideo_thumbnailer_create)();
typedef void (*mvideo_thumbnailer_destroy)(video_thumbnailer *thumbnailer);
/* create image_data structure */
typedef image_data *(*mvideo_thumbnailer_create_image_data)(void);
/* destroy image_data structure */
typedef void (*mvideo_thumbnailer_destroy_image_data)(image_data *data);
typedef int (*mvideo_thumbnailer_generate_thumbnail_to_buffer)(video_thumbnailer *thumbnailer, const char *movie_filename, image_data *generated_image_data);


struct MovieInfo {
    bool valid;
    QString title;
    QString fileType;
    QString resolution;
    QString filePath;
    QDateTime creation;

    // rotation in metadata, this affects width/height
    int raw_rotate;
    qint64 fileSize;
    QString duration = "-";
    int width = -1;
    int height = -1;

    //3.4添加视频信息
    //视频流信息
    QString vCodecID = "-";
    qint64 vCodeRate = 0;
    int fps = 0;
    float proportion = 0;
    //音频流信息
    QString aCodeID = "-";
    qint64 aCodeRate = 0;
    int aDigit = 0;
    int channels = 0;
    int sampling = 0;

//    static struct MovieInfo parseFromFile(const QFileInfo &fi, bool *ok = nullptr);
    /*QString durationStr() const
    {
        return utils::base::Time2str(duration);
    }

    QString videoCodec() const
    {
        return utils::base::videoIndex2str(vCodecID);
    }

    QString audioCodec() const
    {
        return utils::base::audioIndex2str(aCodeID);
    }*/

    QString sizeStr() const
    {
        auto K = 1024;
        auto M = 1024 * K;
        auto G = 1024 * M;
        if (fileSize > G) {
            return QString("%1G").arg((double)fileSize / G, 0, 'f', 1);
        } else if (fileSize > M) {
            return QString("%1M").arg((double)fileSize / M, 0, 'f', 1);
        } else if (fileSize > K) {
            return QString("%1K").arg((double)fileSize / K, 0, 'f', 1);
        }
        return QString("%1").arg(fileSize);
    }
};

class MovieService: public QObject
{
    Q_OBJECT
public:
    static MovieService *instance(QObject *parent = nullptr);

    //获取视频信息
    MovieInfo   getMovieInfo(const QUrl &url);
    //获取视频首帧图片
    QImage      getMovieCover(const QUrl &url);
private:
    struct MovieInfo parseFromFile(const QFileInfo &fi);
    explicit MovieService(QObject *parent = nullptr);
    void initThumb();
    void initFFmpeg();
    QString libPath(const QString &strlib);
private slots:

signals:
public:

private:
    QMutex m_queuqMutex;
    static MovieService *m_movieService;
    static std::once_flag instanceFlag;
    bool m_bInitThumb = false;

    video_thumbnailer *m_video_thumbnailer = nullptr;
    image_data *m_image_data = nullptr;

    QMutex m_bufferMutex;
    std::deque<std::pair<QUrl, MovieInfo>> m_movieInfoBuffer;

    mvideo_thumbnailer_create m_creat_video_thumbnailer = nullptr;
    mvideo_thumbnailer_destroy m_mvideo_thumbnailer_destroy = nullptr;
    mvideo_thumbnailer_create_image_data m_mvideo_thumbnailer_create_image_data = nullptr;
    mvideo_thumbnailer_destroy_image_data m_mvideo_thumbnailer_destroy_image_data = nullptr;
    mvideo_thumbnailer_generate_thumbnail_to_buffer m_mvideo_thumbnailer_generate_thumbnail_to_buffer = nullptr;
};

#endif // MOVIESERVICE_H
