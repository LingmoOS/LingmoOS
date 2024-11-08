/*
 * liblingmosdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */

#include <gif_lib.h>
#include <stb/stb_image.h>

#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QMovie>

#include "log.hpp"
#include "image_load.hpp"
#include "image/converter.hpp"

namespace kdk
{
namespace kabase
{

ImageLoad::ImageLoad()
{
    initLoadFormat();
}

ImageLoad::~ImageLoad() {}

void ImageLoad::initLoadFormat(void)
{
    m_qtLoadFormat << image::ImageFormat::SVG << image::ImageFormat::ICO << image::ImageFormat::XPM;

    m_fiLoadFormat << image::ImageFormat::EXR << image::ImageFormat::PSD << image::ImageFormat::JFI
                   << image::ImageFormat::JIF << image::ImageFormat::JP2 << image::ImageFormat::J2K
                   << image::ImageFormat::JNG << image::ImageFormat::WBMP << image::ImageFormat::XBM
                   << image::ImageFormat::PNM;

    m_cvLoadFormat << image::ImageFormat::JPG << image::ImageFormat::JPE << image::ImageFormat::JPEG
                   << image::ImageFormat::PBM << image::ImageFormat::PGM << image::ImageFormat::PPM
                   << image::ImageFormat::RAS << image::ImageFormat::BMP << image::ImageFormat::WEBP
                   << image::ImageFormat::SR << image::ImageFormat::DIB;

    m_stbLoadFormat << image::ImageFormat::TGA;

    m_dynamicLoadFormat << image::ImageFormat::GIF << image::ImageFormat::APNG << image::ImageFormat::PNG;

    m_allLoadFormat = m_qtLoadFormat + m_fiLoadFormat + m_cvLoadFormat + m_stbLoadFormat + m_dynamicLoadFormat;

    return;
}

image::ImageResult ImageLoad::load(QString path)
{
    image::ImageResult ret;

    image::ImageFormat imageFormat = Utils::getImageFormat(path);
    info << "kabase : load image format is : " << imageFormat;


    if (!m_allLoadFormat.contains(imageFormat)) {
        error << "kabase : image format not support load!";
        return ret;
    }

    switch (imageFormat) {
    case image::ImageFormat::GIF:
    case image::ImageFormat::APNG:
    case image::ImageFormat::PNG:
        return loadDynamicImage(path, imageFormat);
    /* 由于 TIFF 即有静态又有动态 , 此处单独处理 */
    case image::ImageFormat::TIF:
    case image::ImageFormat::TIFF:
        return imageTiFFLoadFromFi(path);
    default:
        ret.mat = loadStaticImage(path, imageFormat);
    }

    return ret;
}

cv::Mat ImageLoad::loadStaticImage(QString path, image::ImageFormat format)
{
    Converter converter;

    if (m_qtLoadFormat.contains(format)) {
        QImage image = imageLoadFromQt(path, format);
        return converter.qimageToMat(image, format);
    } else if (m_fiLoadFormat.contains(format)) {
        FIBITMAP *fibitMap = imageLoadFromFi(path);
        return converter.fibitmapToMat(fibitMap);
    } else if (m_stbLoadFormat.contains(format)) {
        return imageLoadFromStb(path, format);
    } else {
        return imageLoadFromCv(path, format);
    }

    return cv::Mat();
}

image::ImageResult ImageLoad::loadDynamicImage(QString path, image::ImageFormat format)
{
    info << "kabase : start load dynamic image ...";

    image::ImageResult ret;
    Converter converter;

    switch (format) {
    case image::ImageFormat::GIF:
    case image::ImageFormat::APNG:
    case image::ImageFormat::PNG: {
        /* 如果不指定解码格式 , QMovie 将猜想格式进行加载 */
        /* 目前还不能准确获取到动图的格式 , 先使用猜想的方式 */
        QMovie movie(path);
        ret.dynamic.frameNum = movie.frameCount();
        movie.jumpToFrame(0);

        ret.isDynamicImage = true;
        /* 存放动图第一帧 */
        ret.mat = converter.qimageToMat(movie.currentImage(), image::ImageFormat::PNG);
        ret.dynamic.matList.append(ret.mat);
        if (movie.frameCount() > 1) {
            /* 获取帧率 */
            ret.dynamic.fps = getFps(path, format);

            /* 获取剩余帧数 */
            for (int i = 1; i < movie.frameCount(); i++) {
                movie.jumpToFrame(i);
                ret.dynamic.matList.append(converter.qimageToMat(movie.currentImage(), image::ImageFormat::PNG));
            }
        }
        break;
    }
    default:
        break;
    }

    info << "kabase : end load dynamic image ...";

    return ret;
}

QImage ImageLoad::imageLoadFromQt(QString path, image::ImageFormat format)
{
    info << "kabase : load static image from Qt ...";

    switch (format) {
    case image::ImageFormat::SVG: {
        QSvgRenderer svg(path);
        QPixmap pixmap(svg.defaultSize());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        svg.render(&painter);

        return pixmap.toImage();
    }
    case image::XPM: {
        QImage image(path);
        image = image.convertToFormat(QImage::Format_RGB888);
        image = image.rgbSwapped();

        return image;
    }
    case image::ICO: {
        return QImage(path);
    }
    default:
        break;
    }

    return QImage();
}

cv::Mat ImageLoad::imageLoadFromCv(QString path, image::ImageFormat format)
{
    info << "kabase : load static image from OpenCv ...";

    cv::Mat mat = imread(path.toLocal8Bit().data(), cv::IMREAD_UNCHANGED);

    /* 加载三通道bmp格式图片时 , 某些图可能会按照 4 通道去加载 , 导致图片完全透明 */
    if (format == image::ImageFormat::BMP) {
        if (mat.channels() == 4) {
            mat = imread(path.toLocal8Bit().data(), cv::IMREAD_ANYCOLOR);
        }
    } else if (format == image::ImageFormat::JPEG) {
        if (mat.channels() == 4) {
            mat = imread(path.toLocal8Bit().data(), cv::IMREAD_COLOR);
        }
    }

    return mat;
}

/* PRO : 内存释放问题 */
FIBITMAP *ImageLoad::imageLoadFromFi(QString path)
{
    info << "kabase : load static image form FreeImage ...";

    FREE_IMAGE_FORMAT fiFormat = Utils::getImageFiFormat(path);
    FREE_IMAGE_FORMAT format = FreeImage_GetFileType(path.toLocal8Bit().data());
    if (format != FIF_UNKNOWN) {
        fiFormat = static_cast<FREE_IMAGE_FORMAT>(format);
    }

    if (fiFormat != FIF_UNKNOWN && FreeImage_FIFSupportsReading(fiFormat)) {
        return FreeImage_Load(fiFormat, path.toLocal8Bit().data(), 0);
    } else {
        error << "kabase : load static image from FreeImage fail!";
    }

    return NULL;
}

image::ImageResult ImageLoad::imageTiFFLoadFromFi(QString path)
{
    info << "kabase : start load TIFF image form FreeImage ...";

    image::ImageResult ret;

    FIMULTIBITMAP *fiMultibitMap = FreeImage_OpenMultiBitmap(FIF_TIFF, path.toLocal8Bit().data(), false, true, true, 0);
    int pageCount = FreeImage_GetPageCount(fiMultibitMap);
    if (pageCount > 0) {
        cv::Mat mat = imageLoadFromCv(path, image::ImageFormat::TIFF);
        ret.mat = mat;
    }
    if (pageCount > 1) {
        /* 目前没有获取 tiff 动图帧率的方法 , 目前先设置为默认 1 秒 */
        ret.dynamic.fps = 1000;
        ret.dynamic.matList << ret.mat;
        for (int i = 1; i < pageCount; i++) {
            FIBITMAP *fibitMap = FreeImage_LockPage(fiMultibitMap, i);
            Converter converter;
            cv::Mat mat = converter.fibitmapToMat(fibitMap);
            ret.dynamic.matList << mat;
            FreeImage_UnlockPage(fiMultibitMap, fibitMap, i);
        }
    }
    FreeImage_CloseMultiBitmap(fiMultibitMap);

    info << "kabase : end load TIFF image from FreeImage ...";

    return ret;
}

cv::Mat ImageLoad::imageLoadFromStb(QString path, image::ImageFormat format)
{
    info << "kabase : load static image frome stb ...";
    cv::Mat mat;

    switch (format) {
    case image::ImageFormat::TGA: {
        int width, height, channel;
        unsigned char *data = stbi_load(path.toLocal8Bit().data(), &width, &height, &channel, 0);
        if (data) {
            if (channel == 3) {
                cv::Mat tempMat = cv::Mat(height, width, CV_8UC3, const_cast<uchar *>(data)).clone();
                cv::cvtColor(tempMat, mat, cv::COLOR_BGR2RGB);
            } else if (channel == 4) {
                cv::Mat tempMat = cv::Mat(height, width, CV_8UC4, const_cast<uchar *>(data)).clone();
                cv::cvtColor(tempMat, mat, cv::COLOR_BGRA2RGBA);
            }
        }
        stbi_image_free(data);
        break;
    }
    default:
        break;
    }

    return mat;
}

int ImageLoad::getFps(QString path, image::ImageFormat format)
{
    int fps = 0;

    switch (format) {
    case image::ImageFormat::GIF:
        fps = getGifFps(path);
        break;
    default:
        break;
    }

    /* 当没有获取到 fps 时 , 采用下边方式来获取一个 , 获取方式待优化 */
    if (fps == 0) {
        /* 同样先采用猜想的方式构造 QMovie */
        QMovie movie(path);
        for (int i = 0; i < 5; i++) {
            movie.start();
            movie.stop();
            fps = movie.nextFrameDelay() > fps ? movie.nextFrameDelay() : fps;
        }
    }

    /* 为确保动图有帧率 , 当获取不到帧率时 , 返回一个固定值 */
    return fps == 0 ? 100 : fps;
}

int ImageLoad::getGifFps(QString path)
{
    int fps = 0;
    int error = 0;
    GifFileType *gifFileType = DGifOpenFileName(path.toLocal8Bit().data(), &error);
    if (error != 0) {
        DGifCloseFile(gifFileType, &error);
        return 0;
    }

    DGifSlurp(gifFileType);
    /* 拿第一帧图片的信息 */
    SavedImage *frame = &gifFileType->SavedImages[0];
    for (int i = 0; i < frame->ExtensionBlockCount; i++) {
        if (frame->ExtensionBlocks[i].Function == GRAPHICS_EXT_FUNC_CODE) {
            ExtensionBlock *extensionBlock = &(frame->ExtensionBlocks[i]);
            if (extensionBlock != NULL) {
                fps = 10 * (extensionBlock->Bytes[2] << 8 | extensionBlock->Bytes[1]);
                break;
            }
        }
    }

    DGifCloseFile(gifFileType, &error);

    return fps;
}

} /* namespace kabase */
} /* namespace kdk */
