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

#ifndef IMAGE_IMAGE_LOAD_H_
#define IMAGE_IMAGE_LOAD_H_

#include <FreeImage.h>
#include <QImage>

#include "utils.hpp"

namespace kdk
{
namespace kabase
{

class ImageLoad
{
public:
    ImageLoad();
    ~ImageLoad();

    /**
     * @brief 加载图片
     *
     * @param path 图片路径
     *
     * @return 加载结果
     */
    image::ImageResult load(QString path);

private:
    QVector<image::ImageFormat> m_qtLoadFormat;
    QVector<image::ImageFormat> m_cvLoadFormat;
    QVector<image::ImageFormat> m_fiLoadFormat;
    QVector<image::ImageFormat> m_stbLoadFormat;
    QVector<image::ImageFormat> m_dynamicLoadFormat;
    QVector<image::ImageFormat> m_allLoadFormat;

    /*
     * @brief 初始化加载格式
     *
     * @param 无
     *
     * @return 无
     */
    void initLoadFormat(void);

    /*
     * @brief 加载静态图片
     *
     * @param path 图片路径
     * @param format 图片格式
     *
     * @return 加载后的数据
     */
    cv::Mat loadStaticImage(QString path, image::ImageFormat format);

    /*
     * @brief 加载动态图片
     *
     * @param path 图片路径
     * @param format 图片格式
     *
     * @return 加载后的数据
     */
    image::ImageResult loadDynamicImage(QString path, image::ImageFormat format);

    /*
     * @brief 用 Qt 的方式加载图片
     *
     * @param path 图片路径
     * @param format 图片格式
     *
     * @return 加载后的数据
     */
    QImage imageLoadFromQt(QString path, image::ImageFormat format);

    /*
     * @brief 用 OpenCv 的方式加载图片
     *
     * @param path 图片路径
     * @param format 图片格式
     *
     * @return 加载后的数据
     */
    cv::Mat imageLoadFromCv(QString path, image::ImageFormat format);

    /*
     * @brief 用 FreeImage 的方式加载图片
     *
     * @param path 图片路径
     * @parma format 图片格式
     *
     * @return 加载后的数据
     */
    FIBITMAP *imageLoadFromFi(QString path);

    /*
     * @brief 用 STB 的方式加载图片
     *
     * @param path 图片路径
     *
     * @return 加载后的数据
     */
    cv::Mat imageLoadFromStb(QString path, image::ImageFormat format);

    /*
     * @brief 用 FreeImage 的方式加载 TIFF 图片
     *
     * @param path 图片路径
     *
     * @return 加载后的数据
     */
    image::ImageResult imageTiFFLoadFromFi(QString path);

    /*
     * @brief 获取动图 FPS
     *
     * @param path 图片路径
     * @param format 图片格式
     *
     * @return 获取到的图片 FPS
     */
    int getFps(QString path, image::ImageFormat format);

    /*
     * @brief 获取 GIF 图片 FPS
     *
     * @param path 图片路径
     *
     * @return 获取到的 GIF 图片的 FPS
     */
    int getGifFps(QString path);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
