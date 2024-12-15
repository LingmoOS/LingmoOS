// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMGVIEWDELEGATE_H
#define IMGVIEWDELEGATE_H

#include <QObject>
#include <QDateTime>
#include <QStyledItemDelegate>
#include <QDebug>

#include "image-viewer_global.h"

class TDThumbnailThread;
class LibImgViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    enum DelegateType {
        NullType = 0,
        AllPicViewType,
        AlbumViewType,
        AlbumViewPhoneType,
        TimeLineViewType,
        SearchViewType
    };

    explicit LibImgViewDelegate(QObject *parent = nullptr);

    void setItemSize(QSize size);
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const Q_DECL_OVERRIDE;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const Q_DECL_OVERRIDE;
    bool editorEvent(QEvent *event,
                     QAbstractItemModel *model,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) Q_DECL_OVERRIDE;

signals:

private:
    imageViewerSpace::ItemInfo itemData(const QModelIndex &index) const;

public:

private:
    QImage m_damageImage;
};

#endif // ALBUMDELEGATE_H
