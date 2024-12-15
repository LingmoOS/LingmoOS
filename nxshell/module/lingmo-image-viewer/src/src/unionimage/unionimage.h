// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNIONIMAGE_H
#define UNIONIMAGE_H

#include <QtCore/qglobal.h>

#if defined(UNIONIMAGE_LIBRARY)
#  define UNIONIMAGESHARED_EXPORT Q_DECL_EXPORT
#else
#  define UNIONIMAGESHARED_EXPORT Q_DECL_IMPORT
#endif

#include <QHash>
#include <QString>
#include <QByteArray>
#include <QImage>
#include <QFileInfo>
#include <QStringList>
#include <QMap>

#include "unionimage_global.h"

namespace  LibUnionImage_NameSpace {

enum SupportType {
    UNKNOWNTYPE = 0,    // unknown type
    BITMAP      = 1,    // standard image               : 1-, 4-, 8-, 16-, 24-, 32-bit
    UINT16      = 2,    // array of unsigned short      : unsigned 16-bit
    INT16       = 3,    // array of short               : signed 16-bit
    UINT32      = 4,    // array of unsigned long       : unsigned 32-bit
    INT32       = 5,    // array of long                : signed 32-bit
    FLOAT       = 6,    // array of float               : 32-bit IEEE floating point
    DOUBLE      = 7,    // array of double              : 64-bit IEEE floating point
    COMPLEX     = 8,    // array of FICOMPLEX           : 2 x 64-bit IEEE floating point
    RGB16       = 9,    // 48-bit RGB image             : 3 x 16-bit
    RGBA16      = 10,   // 64-bit RGBA image            : 4 x 16-bit
    RGBF        = 11,   // 96-bit RGB float image       : 3 x 32-bit IEEE floating point
    RGBAF       = 12    // 128-bit RGBA float image     : 4 x 32-bit IEEE floating point
};

UNIONIMAGESHARED_EXPORT QString unionImageVersion();

/**
 * @brief UnionImageSupporFormat
 * @return const QStringList
 * @author DJH
 * 返回UnionImage支持的所有格式
 * StaticFormat返回支持的静态图片
 * DynamicFormat返回支持的动态图片
 */
UNIONIMAGESHARED_EXPORT const QStringList unionImageSupportFormat();
UNIONIMAGESHARED_EXPORT const QStringList supportStaticFormat();
UNIONIMAGESHARED_EXPORT const QStringList supportMovieFormat();


UNIONIMAGESHARED_EXPORT bool isDynamicFormat();
/**
 * @brief CreatNewImage
 * @param[out]          res
 * @param[in]           width
 * @param[in]           height
 * @param[in]           depth
 * @param[in]           type
 * @return bool
 * @author DJH
 * 创建一个可以自定义深度和颜色空间的图片
 */
UNIONIMAGESHARED_EXPORT bool creatNewImage(QImage &res, int width = 0, int height = 0, int depth = 0, SupportType type = UNKNOWNTYPE);

/**
 * @brief LoadImageFromFile
 * @param[in]           path
 * @param[out]          res
 * @param[out]          errorMsg
 * @return bool
 * @author DJH
 * 从文件载入图片
 * 载入成功返回true，图片数据返回到res
 * 载入失败返回false，如果需要可以读取errorMsg返回错误信息
 * 载入动态图片时，只会返回动态图片的第一帧，如果需要动图请使用UUnionMovieImage
 */
UNIONIMAGESHARED_EXPORT bool loadStaticImageFromFile(const QString &path, QImage &res, QString &errorMsg, const QString &format_bar = "");

/**
 * @brief detectImageFormat
 * @param path
 * @return QString
 * 返回图片的真格式
 */
UNIONIMAGESHARED_EXPORT QString detectImageFormat(const QString &path);

/**
 * @brief isNoneQImage
 * @param[in]           qi
 * @return bool
 * @author DJH
 * 判断是否为空图
 */
UNIONIMAGESHARED_EXPORT bool isNoneQImage(const QImage &qi);

/**
 * @brief rotateImage
 * @param[in]           angel
 * @param[out]          image
 * @return bool
 * @author DJH
 * 在内存中旋转图片
 */
UNIONIMAGESHARED_EXPORT bool rotateImage(int angel, QImage &image);

/**
 * @brief rotateImageFIle
 * @param[in]           angel   旋转角度
 * @param[in]           path    文件路径
 * @param[out]          erroMsg 错误信息
 * @param[in]           targetPath 目的路径，若为空则保存至原文件路径 path
 * @return bool
 * @author DJH
 * 旋转图片文件，旋转成功返回true，失败返回false
 * 当不需要获取旋转图片的结果或者只有文件地址时调用该函数
 * 失败时会将错误信息写入erroMsg
 */
UNIONIMAGESHARED_EXPORT bool rotateImageFile(int angel, const QString &path, QString &erroMsg, const QString &targetPath = {});

/**
 * @brief rotateImageFIle
 * @param[in]           angel
 * @param[in][out]      img
 * @param[in]           path
 * @param[out]          erroMsg
 * @return bool
 * @author DJH
 * 旋转图片文件，旋转成功返回true，失败返回false
 * 旋转成功会同时旋转img
 * 图片为空则不会旋转返回失败,失败时会将错误信息写入erroMsg
 */
UNIONIMAGESHARED_EXPORT bool rotateImageFIleWithImage(int angel, QImage &img, const QString &path, QString &erroMsg);

/**
 * @brief getAllMetaData
 * @param path
 * @author LMH
 * @return QMap<QString, QString>
 * 获取图片的所有数据,包括创建时间、修改时间、大小等
 */
UNIONIMAGESHARED_EXPORT QMap<QString, QString> getAllMetaData(const QString &path);

/**
   @brief 读取文件数据取得文件大小
   @param imagepath 文件路径
   @return 文件大小
 */
UNIONIMAGESHARED_EXPORT QSize getImageSize(const QString &imagepath);

/**
 * @brief isImageSupportRotate
 * @param path
 * @author LMH
 * @return bool
 * 图片是否可以旋转
 */
UNIONIMAGESHARED_EXPORT bool isImageSupportRotate(const QString &path);

/**
 * @brief canSave
 * @param path
 * @author LMH
 * @return bool
 * 图片是否可以保存
 */
UNIONIMAGESHARED_EXPORT bool canSave(const QString &path);

/**
 * @brief getOrientation 取得文件自带的方向信息
 * @param path 文件路径
 * @author LMH      
 * @return int 图片方向类型枚举
 * 获得图片的数据
 */
UNIONIMAGESHARED_EXPORT int getOrientation(const QString &path);

/**
 * @brief getImageType
 * @param path
 * @author LMH
 * @return QString
 * 获得图片的类型
 */

UNIONIMAGESHARED_EXPORT imageViewerSpace::ImageType getImageType(const QString &imagepath);

/**
 * @brief getPathType
 * @param path
 * @author LMH
 * @return QString
 * 获得路径类型
 */
UNIONIMAGESHARED_EXPORT imageViewerSpace::PathType getPathType(const QString &imagepath);
};


#endif // UNIONIMAGE_H
