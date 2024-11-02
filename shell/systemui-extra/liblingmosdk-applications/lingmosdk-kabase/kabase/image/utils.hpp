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

#ifndef IMAGE_UTILS_H_
#define IMAGE_UTILS_H_

#include <QString>
#include <FreeImage.h>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>

namespace kdk
{
namespace kabase
{
namespace image
{
struct ImageResult
{
    cv::Mat mat; /* 为动图时存放动图的第一帧 */
    bool isDynamicImage;
    struct
    {
        int frameNum; /* 帧数 */
        int fps;      /* 帧率 */
        QList<cv::Mat> matList;
    } dynamic;
};
enum ImageFormat {
    UNKNOW = 0,
    BMP,
    ICO,
    JPG,
    JPE,
    JPS,
    JPEG,
    KOALA,
    KOA,
    LBM,
    IFF,
    MNG,
    PBM,
    PBMRAW,
    PCD,
    PCX,
    PGM,
    PGMRAW,
    PNG,
    PPM,
    PPMRAW,
    RAS,
    TGA,
    TARGA,
    TIFF,
    TIF,
    PCT,
    PIC,
    PICT,
    WEBP,
    JXR,
    PFM,
    DDS,
    GIF,
    HDR,
    FAX,
    G3,
    SGI,
    CUT,
    JNG,
    WBMP,
    PSD,
    XBM,
    XPM,
    EXR,
    JP2,
    J2K,
    SVG,
    APNG,
    DIB,
    SR,
    JFI,
    JIF,
    PNM
};
} /* namespace image */

class Utils
{
public:
    static image::ImageFormat getImageFormat(QString path);
    static FREE_IMAGE_FORMAT getImageFiFormat(QString path);
    static image::ImageFormat getImageRealFormat(QString path);
    static bool copyFile(QString srcPath, QString dstPath);
    static bool deleteDir(QString path);

private:
    static QHash<QString, image::ImageFormat> getSuffixFormatMap(void);
};

} /* namespace kabase */
} /* namespace kdk */

#endif
