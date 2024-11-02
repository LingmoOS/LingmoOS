#ifndef LINGMOIMAGECODEC_GLOBAL_H
#define LINGMOIMAGECODEC_GLOBAL_H

#include <QtCore/qglobal.h>

#include <QDebug>

#include <QFileInfo>
#include <QPainter>
#include <QSvgRenderer>
#include <QSvgGenerator>
#include <QMovie>
#include <QPixmap>
#include <QHash>
#include <vector>
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>
#include <gif_lib.h>
#include <FreeImage.h>
#include <FreeImagePlus.h>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#define signals rsvg_signals
#include <librsvg/rsvg.h>
#undef signals

using namespace cv;

#if defined(LINGMOIMAGECODEC_LIBRARY)
#define LINGMOIMAGECODEC_EXPORT Q_DECL_EXPORT
#else
#define LINGMOIMAGECODEC_EXPORT Q_DECL_IMPORT
#endif

#endif // LINGMOIMAGECODEC_GLOBAL_H
