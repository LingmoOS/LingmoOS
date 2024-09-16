// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMGUTIL_H
#define IMGUTIL_H

#include "baseutils.h"

#include <QDateTime>
#include <QFileInfo>
#include <QPixmap>
#include <QDir>

#define VAULT_DECRYPT_DIR_NAME          "vault_unlocked"
#define VAULT_BASE_PATH (QDir::homePath() + QString("/.local/share/applications"))  //! 获取保险箱创建的目录地址
namespace Libutils {

namespace image {

const int THUMBNAIL_MAX_SIZE = 291 * 2;
const int THUMBNAIL_NORMAL_SIZE = 128 * 2;

enum ThumbnailType {
    ThumbNormal,
    ThumbLarge,
    ThumbFail
};

const QPixmap                       cachePixmap(const QString &path);
const QPixmap                       cutSquareImage(const QPixmap &pixmap);
const QPixmap                       cutSquareImage(const QPixmap &pixmap,
                                                   const QSize &size);
const QMap<QString, QString>        getAllMetaData(const QString &path);
const QDateTime                     getCreateDateTime(const QString &path);
const QFileInfoList                 getImagesInfo(const QString &dir,
                                                  bool recursive = true);
int getOrientation(const QString &path);
const QImage                        getRotatedImage(const QString &path);

#ifndef USE_UNIONIMAGE
const QImage loadTga(QString filePath, bool &success);
#endif

bool                                imageSupportRead(const QString &path);
bool                                imageSupportSave(const QString &path);
bool                                imageSupportWallPaper(const QString &path);
bool                                imageSupportGreeterDirect(const QString &path);
bool                                rotate(const QString &path, int degree);
const QImage                        scaleImage(const QString &path,
                                               const QSize &size = QSize(384, 383));

bool                                generateThumbnail(const QString &path);
const QPixmap                       getThumbnail(const QString &path,
                                                 bool cacheOnly = false);
void                                removeThumbnail(const QString &path);
const QString                       thumbnailCachePath();
const QString                       thumbnailPath(const QString &path, ThumbnailType type = ThumbLarge);
bool                                thumbnailExist(const QString &path, ThumbnailType type = ThumbLarge);

QStringList                         supportedImageFormats();

QPixmap getDamagePixmap(bool bLight = true);

//保险箱地址判断
QString                             makeVaultLocalPath(const QString &path, const QString &base);
bool                                isVaultFile(const QString &path);
//增加该地址能否被删除的api
bool                                isCanRemove(const QString &path);

// 取得缓存图像文件路径
QString                             getCacheImagePath();
// 初始化缓存图像文件夹
bool                                initCacheImageFolder();
// 清空图像缓存文件夹内容
bool                                clearCacheImageFolder();
// 检测缓存文件夹中是否存在 fileName 文件
bool                                checkCacheImage(const QString &fileName);
}  // namespace image

}  // namespace utils

#endif // IMGUTIL_H
