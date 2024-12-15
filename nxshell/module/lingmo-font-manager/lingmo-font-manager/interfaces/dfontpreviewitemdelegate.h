// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFONTPREVIEWITEMDELEGATE_H
#define DFONTPREVIEWITEMDELEGATE_H

#include "dfontpreviewitemdef.h"

#include <QFontDatabase>
#include <QStyledItemDelegate>
#include <QPainterPath>

#include <dstyleoption.h>

class DFontPreviewListView;

class DFontPreviewItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit DFontPreviewItemDelegate(QAbstractItemView *parent = nullptr);
//    static void setNoFont(bool noFont);
    //代理的绘画函数
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    //获取当前listview一项尺寸的大小
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    enum FontRole {
        FontPreviewRole = Dtk::UserRole + 1,
        FontSizeRole = Dtk::UserRole + 2,
        FontFamilyRole,
        FontStyleRole,
        FontOwnPreviewRole,
        FontFamilyStylePreviewRole,
    };
protected:
    //事件过滤器
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    //调整预览区域
    QRect adjustPreviewRect(const QRect bgRect) const;
    //根据预览区域和文字显示效果获取基准点
    QPoint adjustPreviewFontBaseLinePoint(const QRect &fontPreviewRect, const QFontMetrics &previewFontMetrics) const;
    //绘制字体预览列表中的复选框
    void paintForegroundCheckBox(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const;
    //绘制字体预览列表中的字体名字
    void paintForegroundFontName(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const;
    //绘制字体预览列表中的收藏图标
    void paintForegroundCollectIcon(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const;
    //绘制字体预览列表中的预览内容
    void paintForegroundPreviewFont(QPainter *painter, const QStyleOptionViewItem &option, const QString &familyName,
                                    const QString &styleName, bool isEnabled, int fontPixelSize, QString &fontPreviewText) const;
    //绘制字体预览列表的背景
    void paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    //绘制tab选中后的背景效果
    void paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &bgRect) const;
    //获取需要绘制区域的路径
    void setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius) const;

    DFontPreviewListView *m_parentView;
};

#endif  // DFONTPREVIEWITEMDELEGATE_H
