// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bigcolorbutton.h"

#include <QApplication>
#include <QDebug>

#include "../utils/baseutils.h"
#include "../utils/configsettings.h"

const qreal COLOR_RADIUS = 4;
const QSize BTN_SIZE = QSize(32, 26);

BigColorButton::BigColorButton(DWidget *parent)
    : DPushButton(parent),
      m_color(QColor(Qt::red)),
      m_isHover(false),
      m_isChecked(false)
{
    setFixedSize(BTN_SIZE);
    setCheckable(true);
    int colIndex = ConfigSettings::instance()->value(
                       "common", "color_index").toInt();
    m_color = colorIndexOf(colIndex);

    connect(this, &DPushButton::clicked, this,
            &BigColorButton::setCheckedStatus);
    connect(ConfigSettings::instance(), &ConfigSettings::shapeConfigChanged,
            this, &BigColorButton::updateConfigColor);
}

void BigColorButton::updateConfigColor(const QString &shape, const QString &key, int index)
{
    if (shape == "common" && key == "color_index") {
        setColor(colorIndexOf(index));
    }
}

BigColorButton::~BigColorButton()
{

}

void BigColorButton::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);

    painter.setBrush(QBrush(QColor(0, 0, 0, 20)));
    if (m_isHover) {
        painter.drawRoundedRect(rect(), 4, 4);
    }

    painter.setBrush(QBrush(QColor(m_color)));
    painter.drawEllipse(QPointF(16, 13),
                        COLOR_RADIUS, COLOR_RADIUS);

    qreal ration = this->devicePixelRatioF();
    if (m_isChecked) {
        QPixmap checkedPic = QIcon(":/images/action/colors_checked.svg"
                                  ).pixmap(BTN_SIZE);
        checkedPic.setDevicePixelRatio(ration);

        painter.drawPixmap(rect(), checkedPic);
    } else if (m_isHover && !m_isChecked) {
        QPixmap hoverPic = QIcon(":/images/action/colors_hover.svg"
                                ).pixmap(BTN_SIZE);
        hoverPic.setDevicePixelRatio(ration);

        painter.drawPixmap(rect(), hoverPic);
    } else {
        QPixmap normalPic = QIcon(":/images/action/colors_hover.svg"
                                 ).pixmap(BTN_SIZE);
        normalPic.setDevicePixelRatio(ration);

        painter.drawPixmap(rect(), normalPic);
    }
}

void BigColorButton::setColor(QColor color)
{
    m_color = color;
    update();
}
/*
 * never used
void BigColorButton::setColorIndex()
{
    int colorNum = ConfigSettings::instance()->value("common", "color_index").toInt();
    m_color = colorIndexOf(colorNum);
    update();
}
*/
void BigColorButton::setCheckedStatus(bool checked)
{
    if (checked) {
        m_isChecked = true;
        update();
    }

}

void BigColorButton::enterEvent(QEvent *)
{
    if (!m_isHover) {
        m_isHover = true;
        update();
    }
}

void BigColorButton::leaveEvent(QEvent *)
{
    if (m_isHover) {
        m_isHover = false;
        update();
    }
}
