// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "coloredprogressbar.h"

#include <DObjectPrivate>

#include <QMap>
#include <QPainter>
#include <QDebug>
#include <QStylePainter>
#include <QStyleOptionProgressBar>

ColoredProgressBar::ColoredProgressBar(QWidget *parent)
    : QProgressBar(parent)
{
    themeChanged();
    QObject::connect(
        DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &ColoredProgressBar::themeChanged);
}

void ColoredProgressBar::themeChanged()
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    m_themeType = themeType;
    if (themeType == DGuiApplicationHelper::LightType) {
        threshmap.clear();
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        DPalette pa = DebApplicationHelper::instance()->palette(this);

        QColor colorStart = pa.color(QPalette::Highlight);
        QColor colorEnd = pa.color(QPalette::HighlightedText);

        threshmap.insert(0, QBrush(colorStart));
        threshmap.insert(1, QBrush(colorEnd));
    } else {
        threshmap.clear();
    }
}

void ColoredProgressBar::addThreshold(int threshold, QBrush brush)
{
    threshmap[threshold] = brush;
}

void ColoredProgressBar::removeThreshold(int threshold)
{
    if (threshmap.contains(threshold)) {
        threshmap.remove(threshold);
    }
}

QList<int> ColoredProgressBar::thresholds() const
{
    return threshmap.keys();
}

void ColoredProgressBar::paintEvent(QPaintEvent *event)
{
    DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType();
    if (themeType == DGuiApplicationHelper::LightType) {
        QProgressBar::paintEvent(event);
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        QStylePainter painter(this);
        QStyleOptionProgressBar styopt;
        initStyleOption(&styopt);

        if (threshmap.upperBound(value()) != threshmap.begin()) {
            styopt.palette.setBrush(QPalette::ColorRole::Highlight, (--threshmap.upperBound(value())).value());
        }

        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);
        painter.drawRect(styopt.rect);

        painter.drawControl(QStyle::ControlElement::CE_ProgressBarGroove, styopt);
        painter.drawControl(QStyle::ControlElement::CE_ProgressBarContents, styopt);

        if (styopt.textVisible && styopt.orientation == Qt::Horizontal) {
            painter.drawControl(QStyle::ControlElement::CE_ProgressBarLabel, styopt);
        }
    } else {
        QProgressBar::paintEvent(event);
    }
}
