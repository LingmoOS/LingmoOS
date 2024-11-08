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

#include "log.hpp"
#include "image/converter.hpp"

namespace kdk
{
namespace kabase
{

Converter::Converter() {}

Converter::~Converter() {}

cv::Mat Converter::fibitmapToMat(FIBITMAP *fibitMap)
{
    cv::Mat ret;

    int cvType = -1;
    int cvCvt = -1;

    int bpp = FreeImage_GetBPP(fibitMap);
    FREE_IMAGE_TYPE fiType = FreeImage_GetImageType(fibitMap);

    switch (fiType) {
    case FIT_UINT16:
        cvType = cv::DataType<ushort>::type;
        break;
    case FIT_INT16:
        cvType = cv::DataType<short>::type;
        break;
    case FIT_UINT32:
        cvType = CV_32S;
        break;
    case FIT_INT32:
        cvType = cv::DataType<int>::type;
        break;
    case FIT_FLOAT:
        cvType = cv::DataType<float>::type;
        break;
    case FIT_DOUBLE:
        cvType = cv::DataType<double>::type;
        break;
    case FIT_COMPLEX:
        cvType = CV_64FC2;
        break;
    case FIT_RGB16:
        cvType = CV_16UC3;
        cvCvt = cv::ColorConversionCodes::COLOR_RGB2BGR;
        break;
    case FIT_RGBA16:
        cvType = CV_16UC4;
        cvCvt = cv::ColorConversionCodes::COLOR_RGBA2BGRA;
        break;
    case FIT_RGBF:
        cvType = CV_32FC3;
        cvCvt = cv::ColorConversionCodes::COLOR_RGB2BGR;
        break;
    case FIT_RGBAF:
        cvType = CV_32FC4;
        cvCvt = cv::ColorConversionCodes::COLOR_RGBA2BGRA;
        break;
    case FIT_BITMAP:
        switch (bpp) {
        case 8:
            cvType = CV_8UC1;
            break;
        case 16:
            cvType = CV_8UC2;
            break;
        case 24:
            cvType = CV_8UC3;
            break;
        case 32:
            cvType = CV_8UC4;
            break;
        default:
            cvType = -1;
        }
        break;
    default:
        error << "kabase : freeimage to opencv type converter fail!";
        return ret;
    }

    int width = FreeImage_GetWidth(fibitMap);
    int height = FreeImage_GetHeight(fibitMap);
    int step = FreeImage_GetPitch(fibitMap);

    if (cvType >= 0) {
        ret = cv::Mat(height, width, cvType, FreeImage_GetBits(fibitMap), step).clone();

        if (cvCvt > 0) {
            cv::cvtColor(ret, ret, cvCvt);
        }
    } else {
        QVector<uchar> lut;
        int n = pow(2, bpp);
        for (int i = 0; i < n; i++) {
            lut.push_back(static_cast<uchar>((255 / (n - 1)) * i));
        }

        FIBITMAP *palletized = FreeImage_ConvertTo8Bits(fibitMap);
        BYTE *data = FreeImage_GetBits(fibitMap);
        for (int row = 0; row < height; row++) {
            for (int column = 0; column < width; column++) {
                ret.at<uchar>(row, column) = cv::saturate_cast<uchar>(lut[data[row * step + column]]);
            }
        }
    }

    cv::flip(ret, ret, 0);

    if (!ret.data) {
        error << "kabase : fibitmap convert mat fail!";
    }

    return ret;
}

/* PRO : 返回值内存释放问题 */
FIBITMAP *Converter::matToFibitmap(cv::Mat mat)
{
    FIBITMAP *ret = NULL;

    int width = mat.size().width;
    int height = mat.size().height;

    switch (mat.type()) {
    case CV_8U:
        ret = FreeImage_AllocateT(FIT_BITMAP, width, height, 8);
        break;
    case CV_8UC3:
        ret = FreeImage_AllocateT(FIT_BITMAP, width, height, 24);
        break;
    case CV_16U:
        ret = FreeImage_AllocateT(FIT_UINT16, width, height, 16);
        break;
    case CV_16S:
        ret = FreeImage_AllocateT(FIT_INT16, width, height, 16);
        break;
    case CV_32S:
        ret = FreeImage_AllocateT(FIT_INT32, width, height, 32);
        break;
    case CV_32F:
        ret = FreeImage_AllocateT(FIT_FLOAT, width, height, 32);
        break;
    case CV_64F:
        ret = FreeImage_AllocateT(FIT_DOUBLE, width, height, 32);
        break;
    case CV_8UC4:
        ret = FreeImage_AllocateT(FIT_BITMAP, width, height, 32);
        break;
    case CV_32FC3:
        ret = FreeImage_AllocateT(FIT_RGBF, width, height, 96);
        break;
    default:
        ret = NULL;
        break;
    }

    if (ret == NULL) {
        error << "kabase : mat convert fibitmap fail!";
        return ret;
    }

    int srcRowBytes = width * mat.elemSize();
    for (int i = 0; i < height; i++) {
        BYTE *ptr2Line = FreeImage_GetScanLine(ret, (height - 1) - i);
        memcpy(ptr2Line, mat.ptr(i), srcRowBytes);
    }

    return ret;
}

QImage Converter::matToQimage(cv::Mat mat)
{
    switch (mat.type()) {
    case CV_8UC4: {
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
    }
    case CV_8UC3: {
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888).rgbSwapped();
    }
    case CV_8UC1: {
        QVector<QRgb> colorTable;
        for (int i = 0; i < 256; ++i) {
            colorTable.push_back(qRgb(i, i, i));
        }

        QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        image.setColorTable(colorTable);
        return image;
    }
    case CV_32FC3: {
        cv::Mat tmpMat;
        mat.convertTo(tmpMat, CV_8UC3, 255.0 / 1);
        return matToQimage(tmpMat.clone());
    }
    }

    error << "kabase : mat convert QPixmap fail!";

    return QImage();
}

cv::Mat Converter::qimageToMat(QImage image, image::ImageFormat format)
{
    switch (format) {
    case image::ImageFormat::SVG:
    case image::ImageFormat::ICO:
        return cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar *>(image.bits()));
    case image::ImageFormat::XPM:
        return cv::Mat(image.height(), image.width(), CV_8UC3, const_cast<uchar *>(image.bits()));
    case image::ImageFormat::GIF:
    case image::ImageFormat::APNG:
    case image::ImageFormat::PNG:
        /* 用于转换动态图片的一帧 */
        return cv::Mat(image.height(), image.width(), CV_8UC4, const_cast<uchar *>(image.bits()),
                       static_cast<size_t>(image.bytesPerLine()));
    default:
        break;
    }

    return cv::Mat();
}

} /* namespace kabase */
} /* namespace kdk */
