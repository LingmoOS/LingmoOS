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

#ifndef IMAGE_CONVERTER_H_
#define IMAGE_CONVERTER_H_

#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <FreeImage.h>
#include <FreeImagePlus.h>
#include <QImage>

#include "image/utils.hpp"

namespace kdk
{
namespace kabase
{

class Converter
{
public:
    Converter();
    ~Converter();

    /**
     * @brief FIBITMAP 转换 Mat
     *
     * @param fibitMap FIBITMAP数据指针
     *
     * @return 转换后的Mat
     */
    cv::Mat fibitmapToMat(FIBITMAP *fibitMap);

    /**
     * @brief Mat 转换 QImage
     *
     * @param mat Mat数据
     *
     * @return 转换后的QImage
     */
    QImage matToQimage(cv::Mat mat);

    /**
     * @brief Mat 转换 FIBITMAP
     *
     * @param mat Mat数据
     *
     * @return 转换后的FIBITMAP
     */
    FIBITMAP *matToFibitmap(cv::Mat mat);

    /**
     * @brief QImage 转换 Mat
     *
     * @param image image对象
     *
     * @return 转换后的Mat
     */
    cv::Mat qimageToMat(QImage image, image::ImageFormat format);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
