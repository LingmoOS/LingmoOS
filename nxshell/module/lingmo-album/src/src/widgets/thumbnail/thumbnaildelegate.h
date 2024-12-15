// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ALBUMDELEGATE_H
#define ALBUMDELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>
#include <QDebug>

#include "unionimage/unionimage_global.h"

class TDThumbnailThread;
class ThumbnailDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum DelegateType {
        NullType = 0,
        AllPicViewType,//所有照片

        TimeLineViewType,//时间线

        SearchViewType,//搜索

        AlbumViewImportTimeLineViewType,//相册-最近导入
        AlbumViewTrashType,//相册-已删除
        AlbumViewFavoriteType,//相册-收藏
        AlbumViewCustomType,//相册-自定义
        AlbumViewPhoneType//相册-设备
    };

    explicit ThumbnailDelegate(DelegateType type, QObject *parent = nullptr);
    void setIsDataLocked(bool value);
//    void setNeedPaint(bool value);

    void setItemSize(QSize size);
    //绘制图片和视频
    void drawImgAndVideo(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private slots:
    void onThemeTypeChanged(int themeType);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;

    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) Q_DECL_OVERRIDE;

signals:
    void sigCancelFavorite(const QModelIndex &index);
//    void sigPageNeedResize(const int &index) const;

private:
    DBImgInfo itemData(const QModelIndex &index) const;
    QRect updatePaintedRect(const QRectF &boundingRect, const QImage& image) const;
public:
    QString m_imageTypeStr;

private:
    QPixmap selectedPixmapLight;
    QPixmap selectedPixmapDark;
    QColor m_borderColor;
    QString  m_defaultThumbnail;
    bool m_itemdata = false;
    DelegateType m_delegatetype = NullType;
    bool bneedpaint = true;
    QSize m_size;
    QPixmap m_default;//图片默认图片
    QPixmap m_videoDefault;//视频默认图片
    QPixmap m_damagePixmap;
};

#endif // ALBUMDELEGATE_H
