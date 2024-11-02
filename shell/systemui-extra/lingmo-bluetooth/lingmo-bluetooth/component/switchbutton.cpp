/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "switchbutton.h"

#include <QDebug>
#include <QKeyEvent>
#define THEME_QT_SCHEMA "org.lingmo.style"
#define THEME_GTK_SCHEMA "org.mate.interface"

SwitchButton::SwitchButton(QWidget *parent) :
    QWidget(parent)
{
//    this->resize(QSize(52, 24));
    this->setFixedSize(QSize(50, 24));

    checked = false;
    hover = false;
    disabled = false;

    space = 4;
//    rectRadius = 5;

    step = width() / 40;
    startX = 0;
    endX= 0;

    timer = new QTimer(this);
    timer->setInterval(5);
    connect(timer, SIGNAL(timeout()), this, SLOT(updatevalue()));
    if (QGSettings::isSchemaInstalled(THEME_GTK_SCHEMA) && QGSettings::isSchemaInstalled(THEME_QT_SCHEMA)) {
        QByteArray qtThemeID(THEME_QT_SCHEMA);
        QByteArray gtkThemeID(THEME_GTK_SCHEMA);

        m_gtkThemeSetting = new QGSettings(gtkThemeID,QByteArray(),this);
        m_qtThemeSetting = new QGSettings(qtThemeID,QByteArray(),this);

        QString style = m_qtThemeSetting->get("styleName").toString();
        changeColor(style);

        connect(m_qtThemeSetting,&QGSettings::changed, [this] (const QString &key) {
            QString style = m_qtThemeSetting->get("styleName").toString();
            if (key == "styleName") {
                changeColor(style);
            }
        });
    }
}

SwitchButton::~SwitchButton()
{
}

void SwitchButton::paintEvent(QPaintEvent *){
    //启用反锯齿
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    drawBg(&painter);
    drawSlider(&painter);

    //焦点预选框
    if (!focused)
        return;
    QColor color = QColor("#818181");
    painter.setPen(color);
    QRect rect = this->rect();
    rect.setWidth(this->rect().width());
    rect.setHeight(this->rect().height());
    painter.drawRect(rect);
}

void SwitchButton::changeColor(const QString &themes) {
    if (hover) {
        return ;
    }

    if (themes == "lingmo-dark" || themes == "lingmo-black" || themes == "lingmo-default") {
        bgColorOff = QColor(OFF_BG_DARK_COLOR);
        bgColorOn = QColor(ON_BG_DARK_COLOR);
        rectColorEnabled = QColor(ENABLE_RECT_DARK_COLOR);
        rectColorDisabled = QColor(DISABLE_RECT_DARK_COLOR);
        sliderColorDisabled = QColor(DISABLE_RECT_DARK_COLOR);
        sliderColorEnabled = QColor(ENABLE_RECT_DARK_COLOR);
        bgHoverOnColor = QColor(ON_HOVER_BG_DARK_COLOR);
        bgHoverOffColor = QColor(OFF_HOVER_BG_DARK_COLOR);
        bgColorDisabled = QColor(DISABLE_DARK_COLOR);
    } else {
        bgColorOff = QColor(OFF_BG_LIGHT_COLOR);
        bgColorOn = QColor(ON_BG_LIGHT_COLOR);
        rectColorEnabled = QColor(ENABLE_RECT_LIGHT_COLOR);
        rectColorDisabled = QColor(DISABLE_RECT_LIGHT_COLOR);
        sliderColorDisabled = QColor(DISABLE_RECT_LIGHT_COLOR);
        sliderColorEnabled = QColor(ENABLE_RECT_LIGHT_COLOR);
        bgHoverOnColor = QColor(ON_HOVER_BG_LIGHT_COLOR);
        bgHoverOffColor = QColor(OFF_HOVER_BG_LIGHT_COLOR);
        bgColorDisabled = QColor(DISABLE_LIGHT_COLOR);
    }
}

void SwitchButton::drawBg(QPainter *painter){
    painter->save();
//    painter->setPen(Qt::NoPen);

    if (disabled) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(bgColorDisabled);
    } else {
        if (!checked){
            painter->setPen(Qt::NoPen);
            painter->setBrush(bgColorOff);
        }
        else {
            painter->setPen(Qt::NoPen);
            painter->setBrush(bgColorOn);
        }
    }
    //circle out
//    QRect rect(space, space, width() - space * 2, height() - space * 2);
//    painter->drawRoundedRect(rect, rectRadius, rectRadius);

    //circle in

    QRect rect(0, 0, width(), height());
    //半径为高度的一半
    int radius = rect.height() / 2;
    //圆的宽度为高度
    int circleWidth = rect.height();

    QPainterPath path;
    path.moveTo(radius, rect.left());
    path.arcTo(QRectF(rect.left(), rect.top(), circleWidth, circleWidth), 90, 180);
    path.lineTo(rect.width() - radius, rect.height());
    path.arcTo(QRectF(rect.width() - rect.height(), rect.top(), circleWidth, circleWidth), 270, 180);
    path.lineTo(radius, rect.top());

    painter->drawPath(path);

    painter->restore();
}

void SwitchButton::drawSlider(QPainter *painter){
    painter->save();
    painter->setPen(Qt::NoPen);

    if (!disabled){
        painter->setBrush(sliderColorEnabled);
    }
    else
        painter->setBrush(sliderColorDisabled);
    //circle out
//    QRect rect(0, 0, width() - space, height() - space);
//    int sliderwidth = rect.height();
//    QRect sliderRect(startX, space / 2, sliderwidth, sliderwidth);
//    painter->drawEllipse(sliderRect);

    //circle in

    if (disabled) {
        if (checked) {
            QRect smallRect(8, height() / 2 - 2, 10 , 4);
            painter->drawRoundedRect(smallRect,3,3);
        } else {
            QRect smallRect(width() - 8 * 2, height() / 2 - 2, 10 , 4);
            painter->drawRoundedRect(smallRect,3,3);
        }
    }

    QRect rect(0, 0, width(), height());
    int sliderWidth = rect.height() - space * 2;
    QRect sliderRect(startX + space, space, sliderWidth, sliderWidth);
    painter->drawEllipse(sliderRect);

    painter->restore();
}

void SwitchButton::mousePressEvent(QMouseEvent *event){
    if (timer->isActive()) {
        return ;
    }

    if (!disabled){
        checked = !checked;
        emit clickedButton(checked);

        step = width() / 40;

        if (checked){
            //circle out
    //        endX = width() - height() + space;
            //circle in
            endX = width() - height();
        }
        else {
            endX = 0;
        }
        timer->start();
    }
    else {
        endX = 0;
    }
    return QWidget::mousePressEvent(event);
}

void SwitchButton::resizeEvent(QResizeEvent *){
    //
    step = width() / 40;

    if (checked){
        //circle out
//        startX = width() - height() + space;
        //circle in
        startX = width() - height();
    }
    else
        startX = 0;

    update();
}

void SwitchButton::enterEvent(QEvent *event) {
    if (focused)
        return;
    bgColorOn = bgHoverOnColor;
    bgColorOff = bgHoverOffColor;

    hover = true;
    update();
    return QWidget::enterEvent(event);
}

void SwitchButton::leaveEvent(QEvent *event) {
    if (focused)
        return;
    hover = false;

    QString style = m_qtThemeSetting->get("styleName").toString();
    changeColor(style);

    update();
    return QWidget::leaveEvent(event);
}

void SwitchButton::setFocusStyle(bool val) {
    focused = val;
    if (val) {
        bgColorOn = bgHoverOnColor;
        bgColorOff = bgHoverOffColor;

        hover = true;
        update();
    } else {
        hover = false;

        QString style = m_qtThemeSetting->get("styleName").toString();
        changeColor(style);

        update();
    }
    this->update();
}

void SwitchButton::updatevalue(){
    if (disabled) {
        return ;
    }

    if (checked)
        if (startX < endX){
            startX = startX + step;
        }
        else {
            startX = endX;
            timer->stop();
        }
    else {
        if (startX > endX){
            startX = startX - step;
        }
        else {
            startX = endX;
            timer->stop();
        }
    }
    update();
}

void SwitchButton::setChecked(bool checked){
    if (this->checked != checked){
        this->checked = checked;
        emit checkedChanged(checked);
        update();
    }

    step = width() / 40;

    if (checked){
        //circle out
//        endX = width() - height() + space;
        //circle in
        endX = width() - height();
    }
    else {
        endX = 0;
    }
    timer->start();
}

bool SwitchButton::isChecked(){
    return this->checked;
}

void SwitchButton::setDisabledFlag(bool value)
{
    disabled = value;
    update();
}

bool SwitchButton::getDisabledFlag()
{
    return disabled;
}
