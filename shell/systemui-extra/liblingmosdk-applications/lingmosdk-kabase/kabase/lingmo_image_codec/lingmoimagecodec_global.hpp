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

#ifndef LINGMO_IMAGE_CODEC_LINGMOIMAGECODEC_GLOBAL_H_
#define LINGMO_IMAGE_CODEC_LINGMOIMAGECODEC_GLOBAL_H_

#include <stb/stb_image_write.h>
#include <stb/stb_image.h>
#include <gif_lib.h>
#include <FreeImage.h>
#include <FreeImagePlus.h>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>

#include <QtCore/qglobal.h>
#include <QDebug>
#include <QFileInfo>
#include <QPainter>
#include <QSvgRenderer>
#include <QSvgGenerator>
#include <QMovie>
#include <QPixmap>
#include <QHash>

using namespace cv;

#if defined(LINGMOIMAGECODEC_LIBRARY)
#define LINGMOIMAGECODEC_EXPORT Q_DECL_EXPORT
#else
#define LINGMOIMAGECODEC_EXPORT Q_DECL_IMPORT
#endif

#endif // LINGMOIMAGECODEC_GLOBAL_H
