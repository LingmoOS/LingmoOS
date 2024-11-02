/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#include "radioitembutton.h"
#include <QPainter>
#include <QPainterPath>
#include <QSvgRenderer>
#include <QApplication>
#include <QStyle>
#include <QDebug>
#include "themepalette.h"

#define FLASH_SPEED 100
#define TIMEOUT_TIMER 90*1000
#define BUTTON_SIZE 36,36
#define ICON_SIZE 16,16
#define BACKGROUND_COLOR QColor(0,0,0,0)
#define FOREGROUND_COLOR_NORMAL_INACTIVE_LIGHT QColor(230,230,230,255)
#define FOREGROUND_COLOR_NORMAL_INACTIVE_DARK QColor(55,55,55,255)
#define FOREGROUND_COLOR_PRESS_INACTIVE_LIGHT QColor(217,217,217,255)
#define FOREGROUND_COLOR_PRESS_INACTIVE_DARK QColor(70,70,70,255)
#define FOREGROUND_COLOR_NORMAL_ACTIVE QColor(55,144,250,255)
#define FOREGROUND_COLOR_PRESS_ACTIVE QColor(36,109,212,255)

#define COLOR_BRIGHT_TEXT this->palette().brightText().color()
#define COLOR_HIGH_LIGHT this->palette().highlight().color()
#define THEME_SCHAME  "org.lingmo.style"
#define COLOR_THEME   "styleName"

QColor mixColor(const QColor &c1, const QColor &c2, qreal bias);

RadioItemButton::RadioItemButton(QWidget *parent) : QPushButton(parent)
{
    this->setAutoFillBackground(false);
    m_iconLabel = new QLabel(this);

    this->setFixedSize(BUTTON_SIZE);
    m_iconLabel->setFixedSize(BUTTON_SIZE);
    m_iconLabel->setAlignment(Qt::AlignCenter);

    setActive(false);

    const QByteArray id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(id)) {
        m_styleGSettings = new QGSettings(id);
        connect(m_styleGSettings, &QGSettings::changed, this, [=](QString key){
            if ("themeColor" == key) {
                onPaletteChanged();
            }
        });
    }

    onPaletteChanged();

    //JXJ_TODO loading动画
    connect(this, &RadioItemButton::requestStartLoading, this, &RadioItemButton::onLoadingStarted);
    connect(this , &RadioItemButton::requestStopLoading, this, &RadioItemButton::onLoadingStopped);
    connect(qApp, &QApplication::paletteChanged, this, &RadioItemButton::onPaletteChanged);
}

RadioItemButton::~RadioItemButton()
{
    if (m_styleGSettings != nullptr) {
        delete m_styleGSettings;
        m_styleGSettings = nullptr;
    }
}

void RadioItemButton::startLoading()
{
    Q_EMIT this->requestStartLoading();
}

void RadioItemButton::stopLoading()
{
    Q_EMIT this->requestStopLoading();
}
//设置图标
void RadioItemButton::setButtonIcon(const QIcon &icon)
{
    if (icon.isNull()) {
        return;
    }
    m_pixmap = icon.pixmap(ICON_SIZE);
//    m_iconLabel->setPixmap(m_pixmap);
}
//显示默认图标
void RadioItemButton::setDefaultPixmap()
{
    m_iconLabel->setPixmap(m_pixmap);
}
//根据连接状态更改图标颜色
void RadioItemButton::setActive(const bool &isActive)
{
    m_isActivated = isActive;
    refreshButtonIcon();
}
void RadioItemButton::onLoadingStarted()
{
#define ANIMATION_SPEED 0.5*1000
#define START_ROTATION 0
#define END_ROTATION 360
#define ANIMATION_LOOP -1 //无限旋转
    if (!m_iconLabel) {
        qWarning() << "Start loading failed, iconLabel is null pointer!" << Q_FUNC_INFO << __LINE__;
    }
    if (!m_animation) {
        m_animation = new QVariantAnimation(m_iconLabel);
    }
    m_animation->setDuration(ANIMATION_SPEED);
    m_animation->setStartValue(START_ROTATION);
    m_animation->setEndValue(END_ROTATION);
    m_animation->setLoopCount(ANIMATION_LOOP);
    connect(m_animation, &QVariantAnimation::valueChanged, this, &RadioItemButton::onAnimationValueChanged);
    m_animation->start();
}

void RadioItemButton::onLoadingStopped()
{
    if (!m_animation) {
        qWarning() << "Stop loading failed, m_animation is null pointer!" << Q_FUNC_INFO << __LINE__;
        return;
    } else {
        m_iconLabel->setPixmap(m_pixmap);
        m_animation->stop();
        Q_EMIT this->animationStoped();
    }

}

void RadioItemButton::onPaletteChanged()
{
    QPalette pal = qApp->palette();
    QGSettings * styleGsettings = nullptr;
    const QByteArray styleId(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(styleId)) {
       styleGsettings = new QGSettings(styleId, QByteArray(), this);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           pal = themePalette(true, this);
       }
    }
    pal.setColor(QPalette::Base, pal.color(QPalette::Base)); //解决Wayland环境this->setPalette(pal)不生效问题
    pal.setColor(QPalette::BrightText, pal.color(QPalette::BrightText));
    pal.setColor(QPalette::Highlight, pal.color(QPalette::Highlight));

    this->setPalette(pal);
    refreshButtonIcon();
}

void RadioItemButton::onAnimationValueChanged(const QVariant& value)
{
    if (!m_iconLabel) {
        return;
    }
    QTransform t;
    t.rotate(value.toReal());
    m_iconLabel->setPixmap(QIcon(":/res/s/conning-a/1.png").pixmap(ICON_SIZE).transformed(t));
}

void RadioItemButton::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();
    pal.setColor(QPalette::Base, BACKGROUND_COLOR);
    pal.setColor(QPalette::Text, m_backgroundColor);

    QPainterPath cPath;
    cPath.addRect(0, 0, this->width(), this->height());
    cPath.addEllipse(0, 0, this->width(), this->width());

    QPainterPath innerPath;
    innerPath.addEllipse(0, 0, this->width(), this->width());

    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);

    painter.setBrush(pal.color(QPalette::Base));
    painter.drawPath(cPath);

    painter.fillPath(innerPath, pal.color(QPalette::Text));
}

void RadioItemButton::mousePressEvent(QMouseEvent *event)
{
    if (m_isActivated) {
//        m_backgroundColor = qApp->palette().highlight().color();
        m_backgroundColor = mixColor(COLOR_HIGH_LIGHT, COLOR_BRIGHT_TEXT, 0.2);
    } else {
        m_backgroundColor = COLOR_BRIGHT_TEXT;
        m_backgroundColor.setAlphaF(0.21);
    }
    this->update();
    return QPushButton::mousePressEvent(event);
}

void RadioItemButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isActivated) {
        m_backgroundColor = COLOR_HIGH_LIGHT;
    } else {
        m_backgroundColor = COLOR_BRIGHT_TEXT;
        m_backgroundColor.setAlphaF(0.12);
    }
    this->update();
    return QPushButton::mouseReleaseEvent(event);
}

void RadioItemButton::enterEvent(QEvent *event)
{
    if (m_isActivated) {
        m_backgroundColor = COLOR_HIGH_LIGHT;
    } else {
        m_backgroundColor = COLOR_BRIGHT_TEXT;
        m_backgroundColor.setAlphaF(0.15);
    }
    this->update();
    return QPushButton::enterEvent(event);
}

void RadioItemButton::leaveEvent(QEvent *event)
{
    if (m_isActivated) {
        m_backgroundColor = COLOR_HIGH_LIGHT;
    } else {
        m_backgroundColor = COLOR_BRIGHT_TEXT;
        m_backgroundColor.setAlphaF(0.12);
    }
    this->update();
    return QPushButton::leaveEvent(event);
}

void RadioItemButton::refreshButtonIcon()
{
    if (m_isActivated) {
        m_backgroundColor = COLOR_HIGH_LIGHT;
        m_iconLabel->setPixmap(loadSvg(m_pixmap, PixmapColor::WHITE));
    } else {
        m_backgroundColor = COLOR_BRIGHT_TEXT;
        m_backgroundColor.setAlphaF(0.12);
        if (this->palette().base().color().red() > MIDDLE_COLOR) {
            m_iconLabel->setPixmap(m_pixmap);
        } else {
            m_iconLabel->setPixmap(loadSvg(m_pixmap, PixmapColor::WHITE));
        }
    }

    return;
}

const QPixmap RadioItemButton::loadSvg(const QPixmap &source, const PixmapColor &cgColor)
{
    QImage img = source.toImage();
    for (int x = 0; x < img.width(); x++) {
        for (int y = 0; y < img.height(); y++) {
            auto color = img.pixelColor(x, y);
            if (color.alpha() > 0) {
                switch (cgColor) {
                case PixmapColor::WHITE:
                    color.setRed(255);
                    color.setGreen(255);
                    color.setBlue(255);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLACK:
                    color.setRed(0);
                    color.setGreen(0);
                    color.setBlue(0);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::GRAY:
                    color.setRed(152);
                    color.setGreen(163);
                    color.setBlue(164);
                    img.setPixelColor(x, y, color);
                    break;
                case PixmapColor::BLUE:
                    color.setRed(61);
                    color.setGreen(107);
                    color.setBlue(229);
                    img.setPixelColor(x, y, color);
                    break;
                default:
                    return source;
                    break;
                }
            }
        }
    }
    return QPixmap::fromImage(img);
}
