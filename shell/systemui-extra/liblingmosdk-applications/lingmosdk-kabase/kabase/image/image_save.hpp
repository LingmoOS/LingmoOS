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

#ifndef IMAGE_IMAGE_SAVE_H_
#define IMAGE_IMAGE_SAVE_H_

#include <QString>

#include "image/utils.hpp"

namespace kdk
{
namespace kabase
{

class ImageSave
{
public:
    ImageSave();
    ~ImageSave();

    /**
     * @brief 保存图片
     *
     * @param path 保存路径
     * @param imageResult 需要保存的数据
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool save(QString path, image::ImageResult imageResult);

private:
    QVector<image::ImageFormat> m_qtSaveFormat;
    QVector<image::ImageFormat> m_cvSaveFormat;
    QVector<image::ImageFormat> m_fiSaveFormat;
    QVector<image::ImageFormat> m_stbSaveFormat;
    QVector<image::ImageFormat> m_allSaveFormat;

    /*
     * @brief 初始化保存格式
     *
     * @param 无
     *
     * @return 无
     */
    void initSaveFormat(void);

    /*
     * @brief 保存静态图片
     *
     * @param path 保存路径
     * @param mat 要保存的数据
     * @param format 保存格式
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool saveStaticImage(QString path, cv::Mat mat, image::ImageFormat format);

    /*
     * @brief 保存动态图片
     *
     * @param path 保存路径
     * @param imageResult 要保存的数据
     * @param format 保存格式
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool saveDynamicImage(QString path, image::ImageResult imageResult, image::ImageFormat format);

    /*
     * @brief 用 OpenCv 的方式保存图片
     *
     * @param path 保存路径
     * @param mat 要保存的数据
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool imageSaveFromCv(QString path, cv::Mat mat);

    /*
     * @brief 用 Qt 的方式保存图片
     *
     * @param path 保存路径
     * @param mat 要保存的数据
     * @param format 保存格式
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool imageSaveFromQt(QString path, cv::Mat mat, image::ImageFormat format);

    /*
     * @brief 用 FreeImage 的方式保存图片
     *
     * @param path 保存路径
     * @param mat 要保存的数据
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool imageSaveFromFi(QString path, cv::Mat mat);

    /*
     * @brief 用 stb 的方式保存图片
     *
     * @param path 保存路径
     * @param mat 要保存的数据
     * @param format 保存格式
     *
     * @retval true 成功
     * @retval false 失败
     */
    bool imageSaveFromStb(QString path, cv::Mat mat, image::ImageFormat format);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
