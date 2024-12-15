// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DPalette>
#include <DPaletteHelper>
#include <DApplicationHelper>
#include <DApplication>
#include <DStyle>

#include <QPainter>
#include <QPainterPath>

#include "../core/utils.h"
#include "borderradiusheaderview.h"

static const int kSpacingMargin = 4;

BorderRadiusHeaderView::BorderRadiusHeaderView(Qt::Orientation orientation, QWidget *parent)
    : DHeaderView (orientation, parent)
{
    viewport()->setAutoFillBackground(false);
}

void BorderRadiusHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setOpacity(1);

        DPalette::ColorGroup cg = DPalette::Active;
        DPalette palette = DApplicationHelper::instance()->applicationPalette();
        DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
        if (!style)
            return;

        if (model() && logicalIndex == 0)
        {
            QStyleOptionButton option;
            option.rect = QRect(210,9,15,15);
            bool checked = model()->headerData(logicalIndex, orientation(), Qt::CheckStateRole).toBool();
            option.state = checked ? QStyle::State_On : QStyle::State_Off;
            this->style()->drawControl(QStyle::CE_CheckBox, &option, painter);
            QStyleOptionButton options;
            options.rect = QRect(235,9,30,15);
            options.text = QString(tr("All"));
            this->style()->drawControl(QStyle::CE_CheckBoxLabel , &options, painter);
        }

        QStyleOptionHeader option;
        initStyleOption(&option);
        int margin = style->pixelMetric(DStyle::PM_ContentsMargins, &option);

        // title
        QRect contentRect(rect.x(), rect.y(), rect.width(), rect.height() - 1);
        QRect hSpacingRect(rect.x(), contentRect.height(), rect.width(),
                           rect.height() - contentRect.height());

        QBrush contentBrush(palette.color(cg, DPalette::Base));
        // horizontal spacing
        QBrush hSpacingBrush(palette.color(cg, DPalette::FrameBorder));
        // vertical spacing
        QBrush vSpacingBrush(QBrush(QColor("#1A000000")));
        QRectF vSpacingRect(rect.x(), rect.y() + kSpacingMargin, 1,
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
        QRect textRect = {contentRect.x() + margin, contentRect.y(), contentRect.width() - margin,
                        contentRect.height()};
        QString title = model()->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
        int align = Qt::AlignLeft | Qt::AlignVCenter;

        painter->setPen(forground);
        painter->drawText(textRect, static_cast<int>(align), title);
        painter->restore();
}

void BorderRadiusHeaderView::paintEvent(QPaintEvent *event)
{
    QPainter painter(viewport());
    painter.save();

    DPalette::ColorGroup cg = DPalette::Active;
    DPalette palette = DApplicationHelper::instance()->applicationPalette();

    DStyle *style = dynamic_cast<DStyle *>(DApplication::style());
    if (!style)
        return;

    QBrush bgBrush(QColor(isDarkMode() ? "#CC202020" : "#FFFFFFFF"));

    QStyleOptionHeader option;
    initStyleOption(&option);
    int radius = 8;
    int thickness = 1;

    QRect rect = viewport()->rect();
    painter.fillRect(rect, bgBrush);

    // Draw rounded rectangle
    // Fill the substraction of outer and inner rounded rectangle to draw the borders.
    QPainterPath paintPath, paintPathOut, paintPathIn;
    rect.setHeight(rect.height() + radius);
    paintPathOut.addRoundedRect(rect, radius, radius);

    QRect rectIn = QRect(rect.x() + thickness, rect.y() + thickness, rect.width() - thickness * 2, rect.height() - thickness * 2);
    paintPathIn.addRoundedRect(rectIn, radius, radius);
    paintPath = paintPathOut.subtracted(paintPathIn);
    QBrush borderBrush(QColor("#1A000000"));
    painter.fillPath(paintPath, borderBrush);

    // Rounded corners.
    QPainterPath cornerOutPath;
    cornerOutPath.addRect(rect);
    cornerOutPath = cornerOutPath.subtracted(paintPathOut);
    painter.fillPath(cornerOutPath, QBrush(QColor(isDarkMode() ? "#232323" : "#FFFFFF")));

    painter.restore();
    DHeaderView::paintEvent(event);
}

void BorderRadiusHeaderView::mouseReleaseEvent(QMouseEvent* event)
{
        QHeaderView::mouseReleaseEvent(event);
        if(model())
        {
            int section = logicalIndexAt(event->pos());
            if (section == 0)
            {
                bool checked = model()->headerData(section, orientation(), Qt::CheckStateRole).toBool();
                model()->setHeaderData(section, orientation(), !checked, Qt::CheckStateRole);
                viewport()->update();
                emit selectAllButtonClicked(!checked);
            }
        }
}
