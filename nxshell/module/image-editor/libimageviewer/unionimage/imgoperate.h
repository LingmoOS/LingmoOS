// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ImgOperate_H
#define ImgOperate_H

#include <QObject>
#include <QDateTime>
#include <QMutex>
#include <QDebug>
#include <QImage>

#include "image-viewer_global.h"

class LibImgOperate : public QObject
{
    Q_OBJECT
public:
    explicit LibImgOperate(QObject *parent = nullptr);
    ~LibImgOperate();

public slots:
    void slotMakeImgThumbnail(QString thumbnailSavePath, QStringList paths, int makeCount, bool remake);

signals:
    //图片信息以及缩略图制作完成
    void sigOneImgReady(QString imagepath, imageViewerSpace::ItemInfo itemInfo);

private:
    //判断图片类型
    imageViewerSpace::ImageType getImageType(const QString &imagepath);
    //判断路径类型
    imageViewerSpace::PathType getPathType(const QString &imagepath);
};

#endif // ImgOperate_H
