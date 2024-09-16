// Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defendertableheaderview.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyle>
#include <DStyleHelper>

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

static const int kSpacingMargin = 4;

DefenderTableHeaderView::DefenderTableHeaderView(Qt::Orientation orientation, QWidget *parent)
    : DHeaderView(orientation, parent)
    , m_spacing(1)
{
    // 设置表头属性
    viewport()->setAutoFillBackground(false);
    setStretchLastSection(true);
    setSectionResizeMode(QHeaderView::Interactive);
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setFixedHeight(37);
    setFocusPolicy(Qt::StrongFocus);
    setSortIndicatorShown(true);

    QFont f;
    f.setPixelSize(14);
    setFont(f);
}

// 绘制表头
void DefenderTableHeaderView::paintSection(QPainter *painter,
                                           const QRect &rect,
                                           int logicalIndex) const
{
    // 绘制设置
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    DPalette::ColorGroup cg;
    cg = DPalette::Active;

    // 获取系统样式
    DPalette palette = qApp->palette();

    // 设置风格
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

    // 获取继承类的间隔
    QStyleOptionHeader option;
    initStyleOption(&option);
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    // 标题
    QRect contentRect(rect.x(), rect.y(), rect.width(), rect.height() - m_spacing);
    QRect hSpacingRect(rect.x(),
                       contentRect.height(),
                       rect.width(),
                       rect.height() - contentRect.height());

    QBrush contentBrush(palette.color(cg, DPalette::Base));
    // 横向间隔
    QBrush hSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    // 纵向间隔
    QBrush vSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    QRectF vSpacingRect(rect.x(),
                        rect.y() + kSpacingMargin,
                        m_spacing,
                        rect.height() - kSpacingMargin * 2);
    QBrush clearBrush(palette.color(cg, DPalette::Window));

    // 填充颜色
    painter->fillRect(hSpacingRect, clearBrush);
    painter->fillRect(hSpacingRect, hSpacingBrush);

    if (visualIndex(logicalIndex) > 0) {
        painter->fillRect(vSpacingRect, clearBrush);
        painter->fillRect(vSpacingRect, vSpacingBrush);
    }

    QPen forground;
    forground.setColor(palette.color(cg, DPalette::Text));
    // 绘制文字
    QRect textRect;
    if (sortIndicatorSection() == logicalIndex) {
        textRect = { contentRect.x() + margin,
                     contentRect.y(),
                     contentRect.width() - margin * 3 - 8,
                     contentRect.height() };
    } else {
        textRect = { contentRect.x() + margin,
                     contentRect.y(),
                     contentRect.width() - margin,
                     contentRect.height() };
    }
    QString title = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
    int align = Qt::AlignLeft | Qt::AlignVCenter;

    painter->setPen(forground);

    if (logicalIndex == 0) {
        QRect col0Rect = textRect;
        col0Rect.setX(textRect.x() + margin - 2);
        painter->drawText(col0Rect, static_cast<int>(align), title);
    } else {
        painter->drawText(textRect, static_cast<int>(align), title);
    }

    if ( // 是否显示排序指示
        isSortIndicatorShown()
        // 当前列为排序列
        && logicalIndex == sortIndicatorSection()
        // 非排序指示隐藏列
        && !m_indicatorHiddingColList.contains(logicalIndex)) {
        // 绘制排序的箭头图标（8×5）
        QRect sortIndicator(textRect.x() + textRect.width() + margin,
                            textRect.y() + (textRect.height() - 5) / 2,
                            8,
                            5);
        option.rect = sortIndicator;
        if (sortIndicatorOrder() == Qt::DescendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowDown, &option, painter, this);
        } else if (sortIndicatorOrder() == Qt::AscendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowUp, &option, painter, this);
        }
    }
    painter->restore();
}

void DefenderTableHeaderView::focusInEvent(QFocusEvent *event)
{
    m_reson = event->reason();
    DHeaderView::focusInEvent(event);
}

// DefenderTableHeaderView::paintEvent 在paintEvent里绘制背景
void DefenderTableHeaderView::paintEvent(QPaintEvent *event)
{
    // 绘制设置
    QPainter painter(viewport());
    painter.save();

    DPalette::ColorGroup cg;
    cg = DPalette::Active;

    // 获取系统样式
    DPalette palette = qApp->palette();

    // 设置风格
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

    QBrush bgBrush(palette.color(cg, DPalette::Base));

    // 坐标设置
    QStyleOptionHeader option;
    initStyleOption(&option);
    int radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);

    QRect rect = viewport()->rect();
    QRectF clipRect(rect.x(), rect.y(), rect.width(), rect.height() * 2);
    QRectF subRect(rect.x(), rect.y() + rect.height(), rect.width(), rect.height());
    QPainterPath clipPath, subPath;
    clipPath.addRoundedRect(clipRect, radius, radius);
    subPath.addRect(subRect);
    clipPath = clipPath.subtracted(subPath);

    painter.fillPath(clipPath, bgBrush);

    DHeaderView::paintEvent(event);
    painter.restore();
    // 绘画焦点
    if (hasFocus() && (m_reson == Qt::TabFocusReason || m_reson == Qt::BacktabFocusReason)) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(option);
        QRect focusRect{ rect.x() - offset(),
                         rect.y(),
                         length() - sectionPosition(0),
                         rect.height() };
        o.rect = style->visualRect(layoutDirection(), rect, focusRect);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, &o, &painter);
    }
}

QSize DefenderTableHeaderView::sizeHint() const
{
    return QSize(width(), 36 + m_spacing);
}

// DefenderTableHeaderView::sectionSizeHint 根据是否有排序箭头返回逻辑字段合适的尺寸
int DefenderTableHeaderView::sectionSizeHint(int logicalIndex) const
{
    QStyleOptionHeader option;
    initStyleOption(&option);
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    QFontMetrics fm(DApplication::font());
    QString buf = model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString();
    if (sortIndicatorSection() == logicalIndex) {
        return fm.horizontalAdvance(buf) + margin * 3 + 8;
    } else {
        return fm.horizontalAdvance(buf) + margin * 2;
    }
}

// 掩藏某列表头下标
void DefenderTableHeaderView::setHeadViewSortHide(int nIndex)
{
    setSortingIndicatorVisible(nIndex, false);
}

// 设置某列表头下标是否可见
void DefenderTableHeaderView::setSortingIndicatorVisible(int nIndex, bool visible)
{
    if (!visible) {
        // 判断该列是否已经隐藏
        if (m_indicatorHiddingColList.contains(nIndex)) {
            return;
        }
        m_indicatorHiddingColList.append(nIndex);
    } else {
        m_indicatorHiddingColList.removeOne(nIndex);
    }
}
