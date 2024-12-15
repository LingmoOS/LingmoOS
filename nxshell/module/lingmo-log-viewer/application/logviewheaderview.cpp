// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logviewheaderview.h"

#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>
#include <DStyle>

static const int kSpacingMargin = 4;
#define ROW_HEIGHT 36
#define ROW_HEIGHT_COMPACT 24

LogViewHeaderView::LogViewHeaderView(Qt::Orientation orientation, QWidget *parent)
    : DHeaderView(orientation, parent)
{
    m_option = new QStyleOptionHeader;
    viewport()->setAutoFillBackground(false);
    setStretchLastSection(true);
    setSectionResizeMode(QHeaderView::Interactive);
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setFixedHeight(37);
    setFocusPolicy(Qt::NoFocus);

#ifdef DTKWIDGET_CLASS_DSizeMode
    // 紧凑模式信号处理
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &LogViewHeaderView::updateSizeMode);
    updateSizeMode();
#endif
}

LogViewHeaderView::~LogViewHeaderView()
{
    if (m_option != nullptr) {
        delete m_option;
        m_option = nullptr;
    }
}
/**
 * @brief LogViewHeaderView::paintSection 绘制表头
 * @param painter
 * @param rect
 * @param logicalIndex
 */
void LogViewHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    DPalette::ColorGroup cg;
#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }
#else
    cg = DPalette::Active;
#endif

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

    initStyleOption(m_option);
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, m_option);

    // title
    QRect contentRect(rect.x(), rect.y(), rect.width(), rect.height() - m_spacing);
    QRect hSpacingRect(rect.x(), contentRect.height(), rect.width(),
                       rect.height() - contentRect.height());

    QBrush contentBrush(palette.color(cg, DPalette::Base));
    // horizontal spacing
    QBrush hSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    // vertical spacing
    QBrush vSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    QRectF vSpacingRect(rect.x(), rect.y() + kSpacingMargin, m_spacing,
                        rect.height() - kSpacingMargin * 2);
    QBrush clearBrush(palette.color(cg, DPalette::Window));

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
    if (logicalIndex == sortIndicatorSection()) {
        textRect = {contentRect.x() + margin, contentRect.y(), contentRect.width() - margin * 3 - 8,
                    contentRect.height()
                   };
    } else {
        textRect = {contentRect.x() + margin, contentRect.y(), contentRect.width() - margin,
                    contentRect.height()
                   };
    }
    if (!model()) {
        return;
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
    if (isSortIndicatorShown() && logicalIndex == sortIndicatorSection()) {
        // 绘制排序的箭头图标（8×5）
        QRect sortIndicator(textRect.x() + textRect.width() + margin,
                            textRect.y() + (textRect.height() - 5) / 2, 8, 10);
        m_option->rect = sortIndicator;
        if (sortIndicatorOrder() == Qt::DescendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowDown, m_option, painter, this);
        } else if (sortIndicatorOrder() == Qt::AscendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowUp, m_option, painter, this);
        }
    }
    painter->restore();
}

void LogViewHeaderView::focusInEvent(QFocusEvent *event)
{
    m_reson = event->reason();
    DHeaderView::focusInEvent(event);
}

/**
 * @brief LogViewHeaderView::paintEvent 在paintEvent里绘制背景
 * @param event
 */
void LogViewHeaderView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    painter.save();

    DPalette::ColorGroup cg;
    //是否有为激活窗口状态
#ifdef ENABLE_INACTIVE_DISPLAY
    QWidget *wnd = DApplication::activeWindow();
    if (!wnd) {
        cg = DPalette::Inactive;
    } else {
        cg = DPalette::Active;
    }
#else
    cg = DPalette::Active;
#endif
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    painter.setRenderHints(QPainter::Antialiasing);
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    QBrush bgBrush(palette.color(cg, DPalette::Base));

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
    // draw focus
    if (hasFocus() && (m_reson == Qt::TabFocusReason || m_reson == Qt::BacktabFocusReason)) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(option);
        QRect focusRect {rect.x() - offset(), rect.y(), length() - sectionPosition(0), rect.height()};
        o.rect = style->visualRect(layoutDirection(), rect, focusRect);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, &o, &painter);
    }
}

QSize LogViewHeaderView::sizeHint() const
{
    int nRowHeight = ROW_HEIGHT;
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::isCompactMode())
        nRowHeight = ROW_HEIGHT_COMPACT;
    else
        nRowHeight = ROW_HEIGHT;
#else
    nRowHeight = ROW_HEIGHT;
#endif
    return QSize(width(), nRowHeight + m_spacing);
}
/**
 * @brief LogViewHeaderView::sectionSizeHint 根据是否有排序箭头返回逻辑字段合适的尺寸
 * @param logicalIndex
 * @return
 */
int LogViewHeaderView::sectionSizeHint(int logicalIndex) const
{
    QStyleOptionHeader option;
    initStyleOption(&option);
    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    QFontMetrics fm(DApplication::font());
    if (!model()) {
        return -1;
    }
    QString buf = model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString();
    if (sortIndicatorSection() == logicalIndex) {
        return fm.width(buf) + margin * 3 + 8;
    } else {
        return fm.width(buf) + margin * 2;
    }
}

void LogViewHeaderView::updateSizeMode()
{
    int nRowHeight = ROW_HEIGHT;
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::isCompactMode())
        nRowHeight = ROW_HEIGHT_COMPACT;
    else
        nRowHeight = ROW_HEIGHT;
#else
    nRowHeight = ROW_HEIGHT;
#endif
    setFixedHeight(nRowHeight);
}
