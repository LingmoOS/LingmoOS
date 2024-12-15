// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ffmpegvideothumbnailer.h"

#include <QApplication>

#include <QString>
#include <QLibrary>
#include <QDir>
#include <QLibraryInfo>
#include <QDebug>

#include <libffmpegthumbnailer/videothumbnailerc.h>

typedef video_thumbnailer *(*mvideo_thumbnailer_create)();
typedef void (*mvideo_thumbnailer_destroy)(video_thumbnailer *thumbnailer);
typedef image_data *(*mvideo_thumbnailer_create_image_data)(void);
typedef void (*mvideo_thumbnailer_destroy_image_data)(image_data *data);
typedef int (*mvideo_thumbnailer_generate_thumbnail_to_buffer)(video_thumbnailer *thumbnailer, const char *movie_filename, image_data *generated_image_data);

static mvideo_thumbnailer_create m_creat_video_thumbnailer = nullptr;
static mvideo_thumbnailer_destroy m_mvideo_thumbnailer_destroy = nullptr;
static mvideo_thumbnailer_create_image_data m_mvideo_thumbnailer_create_image_data = nullptr;
static mvideo_thumbnailer_destroy_image_data m_mvideo_thumbnailer_destroy_image_data = nullptr;
static mvideo_thumbnailer_generate_thumbnail_to_buffer m_mvideo_thumbnailer_generate_thumbnail_to_buffer = nullptr;

static video_thumbnailer *m_video_thumbnailer = nullptr;

//解析成功标记
static bool resolveSuccessed = false;

bool initFFmpegVideoThumbnailer()
{
    QLibrary library("libffmpegthumbnailer.so.4");

    m_creat_video_thumbnailer = reinterpret_cast<mvideo_thumbnailer_create>(library.resolve("video_thumbnailer_create"));
    m_mvideo_thumbnailer_destroy = reinterpret_cast<mvideo_thumbnailer_destroy>(library.resolve("video_thumbnailer_destroy"));
    m_mvideo_thumbnailer_create_image_data = reinterpret_cast<mvideo_thumbnailer_create_image_data>(library.resolve("video_thumbnailer_create_image_data"));
    m_mvideo_thumbnailer_destroy_image_data = reinterpret_cast<mvideo_thumbnailer_destroy_image_data>(library.resolve("video_thumbnailer_destroy_image_data"));
    m_mvideo_thumbnailer_generate_thumbnail_to_buffer = reinterpret_cast<mvideo_thumbnailer_generate_thumbnail_to_buffer>(library.resolve("video_thumbnailer_generate_thumbnail_to_buffer"));

    if (nullptr == m_creat_video_thumbnailer) {
        qWarning() << QString("Resolve libffmpegthumbnailer.so data failed, %1").arg(library.errorString());
        resolveSuccessed = false;
        return false;
    }
    m_video_thumbnailer = m_creat_video_thumbnailer();

    if (m_mvideo_thumbnailer_destroy == nullptr
            || m_mvideo_thumbnailer_create_image_data == nullptr
            || m_mvideo_thumbnailer_destroy_image_data == nullptr
            || m_mvideo_thumbnailer_generate_thumbnail_to_buffer == nullptr
            || m_video_thumbnailer == nullptr) {
        resolveSuccessed = false;
        return false;
    }

    resolveSuccessed = true;

    return true;
}

QImage runFFmpegVideoThumbnailer(const QUrl &url)
{
    if (!resolveSuccessed) {
        return QImage();
    }

    m_video_thumbnailer->thumbnail_size = static_cast<int>(400 * qApp->devicePixelRatio());
    image_data *image_data = m_mvideo_thumbnailer_create_image_data();
    QString file = QFileInfo(url.toLocalFile()).absoluteFilePath();
    m_mvideo_thumbnailer_generate_thumbnail_to_buffer(m_video_thumbnailer, file.toUtf8().data(), image_data);
    QImage img = QImage::fromData(image_data->image_data_ptr, static_cast<int>(image_data->image_data_size), "png");
    m_mvideo_thumbnailer_destroy_image_data(image_data);
    image_data = nullptr;
    return img;
}

