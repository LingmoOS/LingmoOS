// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"

#include <DDciIcon>
#include <DFontSizeManager>

#include <QIcon>
#include <QSvgRenderer>
#include <QPainterPath>
#include <QApplication>

DWIDGET_USE_NAMESPACE

DGUI_USE_NAMESPACE

void DrawHelper::DrawImage(QPainter *painter, const QStyleOptionViewItem &option, const QString &iconName, bool isLightTheme)
{
    const int margin = 14;
    const QSize iconSize = QSize(32, 32);

    DDciIconPalette palette{option.palette.color(QPalette::Normal, QPalette::ToolTipText),
                            option.palette.color(QPalette::Normal, QPalette::HighlightedText)};
    DDciIcon icon = DDciIcon::fromTheme(iconName);
    qreal ratio = qApp->devicePixelRatio();
    QPixmap pixmap = icon.pixmap(ratio, iconSize.width(), isLightTheme ? DDciIcon::Light : DDciIcon::Dark, DDciIcon::Normal, palette);

    QRect pixMapRect(option.rect.left() + margin, option.rect.top() + margin, iconSize.width(), iconSize.height());
    painter->drawPixmap(pixMapRect, pixmap);
}

void DrawHelper::DrawText(QPainter *painter, const QStyleOptionViewItem &option, const QString &text, QColor color, bool withImage)
{
    QRect rect( option.rect );
    if (withImage) {
        rect.setY(rect.y() + 60);
    }

    QTextOption opt;
    opt.setAlignment(Qt::AlignCenter);

    painter->setPen(color);
    painter->drawText(rect, text, opt);
}

void DrawHelper::DrawProgressBar(QPainter *painter, const QStyleOptionViewItem &option, double progress, const QColor color)
{
    const QRect rect(option.rect);
    const int progressBarWidth = 220;
    const int progressBarHeight = 4;
    const float radius = progressBarHeight / 2.f;
    const int leftMargin = 55;
    const int topMargin = 28;

    painter->setPen(Qt::NoPen);

    QRect dest(rect.x() + leftMargin, rect.y() + topMargin, progressBarWidth, progressBarHeight);
    painter->setOpacity(0.1);
    painter->setBrush(color);
    painter->drawRoundedRect(dest, radius, radius);

    dest.setWidth(dest.width() * progress);
    painter->setOpacity(1);
    painter->setBrush(color);
    painter->drawRoundedRect(dest, radius, radius);
}

void DrawHelper::DrawBackground(QPainter *painter, const QStyleOptionViewItem &option)
{
    QPainterPath path;
    path.addRoundedRect(option.rect.marginsRemoved(QMargins(10, 10, 10, 10)), 5, 5);

    painter->fillPath(path, QColor("#2ca7f8"));
}

void DrawHelper::DrawPercentValue(QPainter *painter, const QStyleOptionViewItem &option, double value, const QColor &color)
{
    const int rightMargin = 16;
    const int percentFlagWidth = 9; // %号宽度
    const int percentFlagHeight = 20;
    const int topMargin = 14;
    const int ValueAreaHeight = 26;

    QFont font = DFontSizeManager::instance()->t4();
    font.setPixelSize(20); // 固定大小

    QFontMetrics fm(font);
    int valueAreaWidth = fm.horizontalAdvance(QString::number(value));
    QRect valueRect(option.rect.right() - rightMargin - valueAreaWidth - percentFlagWidth, option.rect.top() + topMargin,
                    valueAreaWidth, ValueAreaHeight);
    QPen pen;
    pen.setColor(color);
    painter->setPen(pen);
    painter->setFont(font);
    painter->drawText(valueRect, QString::number(value));

    font.setPixelSize(10); // 固定大小
    painter->setFont(font);

    QRect percentRect(valueRect.right() + 1, valueRect.top() + valueRect.height() / 2, percentFlagWidth, percentFlagHeight);
    painter->drawText(percentRect, "%");
}
