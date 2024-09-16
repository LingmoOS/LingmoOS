// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "diskhealthheaderview.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <DStyleHelper>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include <QPainterPath>

static const int kSpacingMargin = 4;
// recommend size
// static const QSize kDropDownSize {10, 6};
static const QSize kDropDownSize {11, 10};

DiskHealthHeaderView::DiskHealthHeaderView(Qt::Orientation orientation, QWidget *parent) : DHeaderView(orientation, parent)
{
    installEventFilter(this);

    viewport()->setAutoFillBackground(false);
}

QSize DiskHealthHeaderView::sizeHint() const
{
    return QSize(width(), 36 + m_spacing);
}

int DiskHealthHeaderView::sectionSizeHint(int logicalIndex) const
{
//    QStyleOptionHeader option;
//    initStyleOption(&option);
//    auto *style = dynamic_cast<DStyle *>(DApplication::style());
//    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

//    QFontMetrics fm(this->font());
//    QString buf = model()->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString();
//    if (sortIndicatorSection() == logicalIndex) {
//        return fm.size(Qt::TextSingleLine, buf).width() + margin * 3 + kDropDownSize.width();
//    } else {
//        return fm.size(Qt::TextSingleLine, buf).width() + margin * 2;
//    }
    return DHeaderView::sectionSizeHint(logicalIndex);
}

void DiskHealthHeaderView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    painter.save();

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
//    cg = DPalette::Current;
#endif

    auto palette = DApplicationHelper::instance()->applicationPalette();

    auto *style = dynamic_cast<DStyle *>(DApplication::style());

    QBrush bgBrush(palette.color(cg, DPalette::Base));

    QStyleOptionHeader option;
    initStyleOption(&option);
    auto radius = style->pixelMetric(DStyle::PM_FrameRadius, &option);

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
    if (hasFocus()) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(option);
        QRect focusRect {rect.x() - offset(), rect.y(), length() - sectionPosition(0), rect.height()};
        o.rect = style->visualRect(layoutDirection(), rect, focusRect);
        style->drawPrimitive(DStyle::PE_FrameFocusRect, &o, &painter);
    }
}

void DiskHealthHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setOpacity(1);

    DPalette::ColorGroup cg;
    QStyleOption opt;
    opt.initFrom(this);
    if (!(opt.state & DStyle::State_Enabled)) {
        cg = DPalette::Disabled;
    } else {
#ifdef ENABLE_INACTIVE_DISPLAY
        if (!wnd) {
            cg = DPalette::Inactive;
        } else {
            if (wnd->isModal()) {
                cg = DPalette::Inactive;
            } else {
                cg = DPalette::Active;
            }
        }
#else
        cg = DPalette::Active;
//        cg = DPalette::Current;
#endif
    }

    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
    //    DPalette palette = dAppHelper->palette(this);

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());

    QStyleOptionHeader option;
    initStyleOption(&option);
    int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

    // title
    QRect contentRect(rect.x(), rect.y(), rect.width(), rect.height() - m_spacing);
    QRect hSpacingRect(rect.x(), contentRect.height(), rect.width(),
                       rect.height() - contentRect.height());

    QBrush contentBrush(palette.color(cg, DPalette::Base));
    // horizontal spacing
    QBrush hSpacingBrush(palette.color(cg, DPalette::FrameBorder));
    //    QBrush hSpacingBrush(
    //        DApplicationHelper::instance()->palette(this).color(DPalette::FrameBorder));
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
    QRect textRect;
    if (sortIndicatorSection() == logicalIndex) {
        textRect = {contentRect.x() + margin, contentRect.y() + 7,
                    contentRect.width() - margin * 3 - kDropDownSize.width(), contentRect.height()
                   };
    } else {
        textRect = {contentRect.x() + margin, contentRect.y() + 7, contentRect.width() - margin,
                    contentRect.height()
                   };
    }
    QString title = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
    int align = model()->headerData(logicalIndex, orientation(), Qt::TextAlignmentRole).toInt();
    painter->setPen(forground);
    painter->drawText(textRect, int(align), title);

    // sort indicator
    if (isSortIndicatorShown() && logicalIndex == sortIndicatorSection()) {
        QRect sortIndicator(textRect.x() + textRect.width() + margin,
                            textRect.y() + qCeil((textRect.height() - kDropDownSize.height()) / 2.),
                            kDropDownSize.width(), kDropDownSize.height());
        option.rect = sortIndicator;
        if (sortIndicatorOrder() == Qt::DescendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowDown, &option, painter, this);
        } else if (sortIndicatorOrder() == Qt::AscendingOrder) {
            style->drawPrimitive(DStyle::PE_IndicatorArrowUp, &option, painter, this);
        }
    }

    painter->restore();
}

bool DiskHealthHeaderView::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent *kev = dynamic_cast<QKeyEvent *>(ev);
        if (kev->key() == Qt::Key_Space ||
                kev->key() == Qt::Key_Up ||
                kev->key() == Qt::Key_Down) {
            return true;
        }
    }
    return DHeaderView::eventFilter(obj, ev);
}
