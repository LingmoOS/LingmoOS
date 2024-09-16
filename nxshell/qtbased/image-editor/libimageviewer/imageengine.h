// SPDX-FileCopyrightText: 2022 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEENGINE_H
#define IMAGEENGINE_H

#define DELETE_CONFIRM

#include "image-viewer_global.h"

#include <DWidget>
#include <QtCore/qglobal.h>
#include <QImage>
DWIDGET_USE_NAMESPACE

class ImageEnginePrivate;
class IMAGEVIEWERSHARED_EXPORT ImageEngine : public QObject
{
    Q_OBJECT
public:
    static ImageEngine *instance(QObject *parent = nullptr);
    explicit ImageEngine(QWidget *parent = nullptr);
    ~ImageEngine() override;

    //制作图片缩略图, paths:所有图片路径, makeCount:从第一个开始制作缩略图数量,remake:缩略图是否重新生成
    void makeImgThumbnail(QString thumbnailSavePath, QStringList paths, int makeCount, bool remake = false);
    //判断是否是图片格式
    bool isImage(const QString &path);
    //是否是可选转的图片
    bool isRotatable(const QString &path);

    //根据文件路径制作md5
    QString makeMD5(const QString &path);

signals:
    //一张缩略图制作完成
    void sigOneImgReady(QString path, imageViewerSpace::ItemInfo itemInfo);

    //当前图片数量为0
    void sigPicCountIsNull();

    //可以写一个专门的信号管理类，管理相册和其他地方需要用到的专门的信号
    //更新收藏按钮
    void sigUpdateCollectBtn();
    //删除
#ifdef DELETE_CONFIRM
    void sigDeleteImage(); // 相册确认删除图片，通知看图执行删除图片后的业务逻辑
#endif
    void sigDel(QString path); // 通知相册，删除图片
    //获取自定义相册
    void sigGetAlbumName(const QString &path);
    //添加到已有相册/新建相册
    void sigAddToAlbum(bool isNew, const QString &album, const QString &path);
    void sigAddToAlbumWithUID(bool isNew, int UID, const QString &path);  //采用UID方案的添加至相册
    //收藏/取消收藏
    void sigAddOrRemoveToFav(const QString &path, bool isAdd);
    //导出
    void sigExport(const QString &path);
    //从自定义相册中移除
    void sigRemoveFromCustom(const QString &path, const QString &album);
    void sigRemoveFromCustomWithUID(const QString &path, int UID);  //采用UID方案的从相册中删除
    //退出幻灯片
    void exitSlideShow();
    //按下ESC键
    void escShortcutActivated(bool isSwitchFullScreen);
    //通知旋转
    void sigRotatePic(const QString &path);

    // 授权操作通知信号
    void sigAuthoriseNotify(const QJsonObject &data);

private:
    static ImageEngine *m_ImageEngine;

    QScopedPointer<ImageEnginePrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), ImageEngine)
};

#endif  // IMAGEENGINE_H
