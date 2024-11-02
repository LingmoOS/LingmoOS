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

#include <stb/stb_image_write.h>

#include <QPixmap>
#include <QPainter>
#include <QSvgGenerator>
#include <QFileInfo>
#include <QDir>
#include <QProcess>

#include "log.hpp"
#include "image/image_save.hpp"
#include "image/converter.hpp"

namespace kdk
{
namespace kabase
{

ImageSave::ImageSave()
{
    initSaveFormat();
}

ImageSave::~ImageSave() {}

void ImageSave::initSaveFormat(void)
{
    m_qtSaveFormat << image::ImageFormat::SVG << image::ImageFormat::ICO << image::ImageFormat::XPM
                   << image::ImageFormat::PBM << image::ImageFormat::PPM << image::ImageFormat::XBM
                   << image::ImageFormat::WBMP;

    m_fiSaveFormat << image::ImageFormat::EXR << image::ImageFormat::PSD << image::ImageFormat::JFI
                   << image::ImageFormat::JIF << image::ImageFormat::JP2 << image::ImageFormat::J2K
                   << image::ImageFormat::JNG << image::ImageFormat::PNM;

    m_stbSaveFormat << image::ImageFormat::TGA;

    m_allSaveFormat = m_qtSaveFormat + m_fiSaveFormat + m_cvSaveFormat + m_stbSaveFormat;

    return;
}

bool ImageSave::save(QString path, image::ImageResult imageResult)
{
    image::ImageFormat format = Utils::getImageFormat(path);

    switch (format) {
    case image::ImageFormat::GIF:
    case image::ImageFormat::APNG:
        return saveDynamicImage(path, imageResult, format);
    default:
        return saveStaticImage(path, imageResult.mat, format);
    }

    return false;
}

bool ImageSave::saveStaticImage(QString path, cv::Mat mat, image::ImageFormat format)
{
    if (m_qtSaveFormat.contains(format)) {
        return imageSaveFromQt(path, mat, format);
    } else if (m_fiSaveFormat.contains(format)) {
        return imageSaveFromFi(path, mat);
    } else if (m_stbSaveFormat.contains(format)) {
        return imageSaveFromStb(path, mat, format);
    } else {
        return imageSaveFromCv(path, mat);
    }

    return false;
}

bool ImageSave::saveDynamicImage(QString path, image::ImageResult imageResult, image::ImageFormat format)
{
    info << "kabase : start save dynamic image ...";
    /* 创建临时目录 */
    QFileInfo fileInfo(path);
    QString imageName = fileInfo.completeBaseName();
    QString resultName = imageName + QString(".apng");
    QString cachePath = QDir::homePath() + QString("/.kabase/dynamic_cache/") + imageName + QString("/");
    QDir dir;
    if (!dir.exists(cachePath)) {
        if (!dir.mkdir(cachePath)) {
            error << "kabase : create dynamic temp save path fail!";
            return false;
        }
    }

    /* 创建每一帧图片 */
    int count = 0;
    for (int i = 0; i < imageResult.dynamic.matList.size(); i++) {
        QString savePath = cachePath + QString::number(++count) + QString(".png");
        cv::imwrite(savePath.toLocal8Bit().data(), imageResult.dynamic.matList.at(i));
    }

    /* 合成动图 */
    QString command = QString("apngasm %1%2 %3*.png %4 1000 -z0")
                          .arg(cachePath)
                          .arg(resultName)
                          .arg(cachePath)
                          .arg(QString::number(imageResult.dynamic.fps));
    if (QProcess::execute(command)) {
        error << "kabase : create dynamic image fail!";
        Utils::deleteDir(cachePath);
        return false;
    }

    switch (format) {
    case image::ImageFormat::APNG: {
        /* 移动动图 */
        QString resultPath = cachePath + resultName;
        if (!Utils::copyFile(resultName, path)) {
            error << "kabase : copy dynamic image fail";
            Utils::deleteDir(cachePath);
            return false;
        }
        break;
    }
    case image::ImageFormat::GIF: {
        /* 格式转换 */
        command = QString("apng2gif %1").arg(cachePath + resultName);
        if (QProcess::execute(command)) {
            error << "kabase : dynamic image format converter fail!";
            Utils::deleteDir(cachePath);
            return false;
        }
        /* 移动动图 */
        QString resultPath = cachePath + imageName + QString(".gif");
        if (!Utils::copyFile(resultPath, path)) {
            error << "kabase : copy dynamic image fail";
            Utils::deleteDir(cachePath);
            return false;
        }

        break;
    }
    default:
        Utils::deleteDir(cachePath);
        return false;
    }

    /* 清除缓存 */
    Utils::deleteDir(cachePath);

    info << "end save dynamic image ...";

    return true;
}


bool ImageSave::imageSaveFromQt(QString path, cv::Mat mat, image::ImageFormat format)
{
    info << "kabase : save static image from Qt ...";

    Converter converter;

    switch (format) {
    case image::ImageFormat::SVG: {
        QPixmap pixmap = QPixmap::fromImage(converter.matToQimage(mat));
        QPainter painter;
        QSvgGenerator svgGenerator;
        svgGenerator.setFileName(path);
        svgGenerator.setViewBox(QRect(QPoint(0, 0), pixmap.size()));
        painter.begin(&svgGenerator);
        painter.drawImage(QPoint(0, 0), pixmap.toImage());
        painter.end();
        return true;
    }
    case image::ImageFormat::ICO:
    case image::ImageFormat::XPM:
    case image::ImageFormat::PBM:
    case image::ImageFormat::PPM:
    case image::ImageFormat::XBM:
    case image::ImageFormat::WBMP: {
        return (QPixmap::fromImage(converter.matToQimage(mat)).save(path));
    }
    default:
        break;
    }

    return false;
}

bool ImageSave::imageSaveFromCv(QString path, cv::Mat mat)
{
    info << "kabase : save static image from OpenCv ...";
    return cv::imwrite(path.toStdString(), mat);
}

bool ImageSave::imageSaveFromFi(QString path, cv::Mat mat)
{
    info << "kabase : save static image from FreeImage ...";

    Converter converter;

    FREE_IMAGE_FORMAT fiFormat = Utils::getImageFiFormat(path);
    return FreeImage_Save(fiFormat, converter.matToFibitmap(mat), path.toLocal8Bit().data(), 0);
}

bool ImageSave::imageSaveFromStb(QString path, cv::Mat mat, image::ImageFormat format)
{
    info << "kabase : save static image from stb ...";
    switch (format) {
    case image::ImageFormat::TGA: {
        cv::Mat tempMat;

        if (mat.type() == 3) {
            cv::cvtColor(mat, tempMat, cv::COLOR_RGB2BGR);
        } else if (mat.type() == 4) {
            cv::cvtColor(mat, tempMat, cv::COLOR_RGBA2BGRA);
        }

        return stbi_write_tga(path.toLocal8Bit().data(), tempMat.cols, tempMat.rows, 4, tempMat.data);
    }
    default:
        break;
    }

    return false;
}

} /* namespace kabase */
} /* namespace kdk */
