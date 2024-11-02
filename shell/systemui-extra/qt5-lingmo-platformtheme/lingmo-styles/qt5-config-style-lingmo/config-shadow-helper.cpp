/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */


#include "config-shadow-helper.h"

#include <QPainter>
#include <QPainterPath>
#include <QPoint>
#include <QImage>

#include <QWidget>
#include <QVector4D>
#include <QMargins>

#include <QX11Info>
#include <QStyle>
#include <QApplication>

#include <QDebug>
#include <QMenu>
#include "settings/lingmo-style-settings.h"

#define INNERRECT_WIDTH 1

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

using namespace LINGMOConfigStyleSpace;

ConfigShadowHelper::ConfigShadowHelper(QObject *parent) : QObject(parent)
{

}

ConfigShadowHelper::~ConfigShadowHelper()
{
    m_shadows.clear();
}

bool ConfigShadowHelper::isWidgetNeedDecoShadow(const QWidget *widget)
{
    if (!widget)
        return false;

    auto value = widget->property("useCustomShadow");
    if (!value.isValid()) {
        if (widget->inherits("QComboBoxPrivateContainer") || widget->inherits("QMenu")) {
            return true;
        }
    } else {
        return value.toBool();
    }

    return false;
}

void ConfigShadowHelper::registerWidget(QWidget *widget)
{
    widget->removeEventFilter(this);

    bool needCreateShadowInstantly = false;
    if (isWidgetNeedDecoShadow(widget)) {
        widget->installEventFilter(this);
        needCreateShadowInstantly = true;
    } else {
        if (widget && (widget->inherits("QComboBoxPrivateContainer") || widget->inherits("QMenu"))) {
            widget->installEventFilter(this);
            needCreateShadowInstantly = true;
        }
    }
    if (!widget->isVisible()) {
        needCreateShadowInstantly = false;
    }
    if (needCreateShadowInstantly) {
        auto shadowColor = widget->palette().text().color();

        int shadowBorder = widget->property("customShadowWidth").toInt();
        bool ok = false;
        qreal darkness = widget->property("customShadowDarkness").toReal(&ok);
        if (!ok) {
            darkness = 1.0;
        }
        QVector4D radius = qvariant_cast<QVector4D>(widget->property("customShadowRadius"));
        QVector4D margins = qvariant_cast<QVector4D>(widget->property("customShadowMargins"));

        if (auto tmp = m_shadows.value(widget)) {
            if (tmp->isCreated()) {
                m_shadows.remove(widget);
                tmp->destroy();
                tmp->deleteLater();
            }
        }

        auto shadow = getShadow(shadowColor, shadowBorder, darkness, widget, radius.x(), radius.y(), radius.z(), radius.w());
        shadow->setPadding(QMargins(margins.x(), margins.y(), margins.z(), margins.w()));
        shadow->setWindow(widget->windowHandle());
        shadow->create();

        //qInfo()<<"try set custom shadow"<<shadowBorder<<darkness<<radius<<margins;

        m_shadows.insert(widget, shadow);

        connect(widget, &QWidget::destroyed, this, [=](){
            if (auto shadowToBeDelete = m_shadows.value(widget)) {
                if (shadowToBeDelete->isCreated())
                    shadowToBeDelete->destroy();
                shadowToBeDelete->deleteLater();
                m_shadows.remove(widget);
            }
        });
    }
}

void ConfigShadowHelper::unregisterWidget(const QWidget *widget)
{
    if (m_shadows.contains(widget)) {
        auto shadow = m_shadows.value(widget);
        if (shadow->isCreated())
            shadow->destroy();
        shadow->deleteLater();
        m_shadows.remove(widget);
    }
}

QPixmap ConfigShadowHelper::getShadowPixmap(QColor color, /*ConfigShadowHelper::State state,*/ int shadow_border, qreal darkness, QWidget* widget, int borderRadiusTopLeft, int borderRadiusTopRight, int borderRadiusBottomLeft, int borderRadiusBottomRight)
{
    int maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
    int maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
    int maxRadius = qMax(maxTopRadius, maxBottomRadius);
    maxRadius = qMax(12, maxRadius);
    QPixmap pix(QSize(2 * maxRadius + 2 * shadow_border + INNERRECT_WIDTH, 2 * maxRadius + 2 * shadow_border + INNERRECT_WIDTH));
    pix.fill(Qt::transparent);

    int squareWidth = 2 * maxRadius + INNERRECT_WIDTH;

    QPainterPath windowRelativePath;
    windowRelativePath.setFillRule(Qt::WindingFill);
    QPoint currentPos;

    int qAppMaxradius = maxRadius;
    if(widget){
        if(widget->inherits("QComboBoxPrivateContainer") || widget->inherits("QMenu")){
            auto settings = LINGMOStyleSettings::globalInstance();
            if(settings && settings->keys().contains("windowRadius"))
                qAppMaxradius = settings->get("windowRadius").toInt();
        }
        else if(widget->property("maxRadius").isValid())
            qAppMaxradius = widget->property("maxRadius").toInt();
        //        qDebug() << "radius...." << qAppMaxradius;
    }
    // move to top left arc start point
    windowRelativePath.moveTo(borderRadiusTopLeft, 0);
    // top left arc
    auto topLeftBorderRadiusRect = QRect(0, 0, 2 * borderRadiusTopLeft, 2 * borderRadiusTopLeft);
    if(qAppMaxradius > 0)
        windowRelativePath.arcTo(topLeftBorderRadiusRect, 90, 90);
    else
        windowRelativePath.addRect(topLeftBorderRadiusRect);
    // move to bottom left arc start point
    currentPos = QPoint(0, maxRadius + INNERRECT_WIDTH + maxRadius - borderRadiusBottomLeft);
    //windowRelativePath.moveTo(currentPos);
    // bottom left arc
    auto bottomLeftRect = QRect(0, currentPos.y() - borderRadiusBottomLeft, 2 * borderRadiusBottomLeft, 2 * borderRadiusBottomLeft);
    if(qAppMaxradius > 0)
        windowRelativePath.arcTo(bottomLeftRect, 180, 90);
    else
        windowRelativePath.addRect(bottomLeftRect);

    // move to bottom right arc start point
    currentPos = QPoint(2 * maxRadius + INNERRECT_WIDTH - borderRadiusBottomRight, 2 * maxRadius + INNERRECT_WIDTH);
    //windowRelativePath.moveTo(currentPos);
    // bottom right arc
    auto bottomRightRect = QRect(currentPos.x() - borderRadiusBottomRight, currentPos.y() - 2 * borderRadiusBottomRight, 2 * borderRadiusBottomRight, 2 * borderRadiusBottomRight);
    if(qAppMaxradius > 0)
        windowRelativePath.arcTo(bottomRightRect, 270, 90);
    else
        windowRelativePath.addRect(bottomRightRect);

    // move to top right arc start point
    currentPos = QPoint(2 * maxRadius + INNERRECT_WIDTH, borderRadiusTopRight);
    //windowRelativePath.moveTo(currentPos);
    // top right arc
    auto topRightRect = QRect(squareWidth - 2 * borderRadiusTopRight, 0, 2 * borderRadiusTopRight, 2 * borderRadiusTopRight);
    if(qAppMaxradius > 0)
        windowRelativePath.arcTo(topRightRect, 0, 90);
    else
        windowRelativePath.addRect(topRightRect);

    QPainter painter(&pix);
    painter.save();
    painter.translate(shadow_border, shadow_border);
    painter.fillPath(windowRelativePath, QColor(26,26,26));
    painter.restore();

    QImage rawImg = pix.toImage();
    qt_blurImage(rawImg, shadow_border, true, true);

    QPixmap target = QPixmap::fromImage(rawImg);
    QPainter painter2(&target);
    painter2.save();
    painter2.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter2.translate(shadow_border, shadow_border);
    painter2.setCompositionMode(QPainter::CompositionMode_Clear);
    painter2.fillPath(windowRelativePath, Qt::transparent);
    painter2.restore();
    painter2.end();

    // handle darkness
    QImage newImg = target.toImage();
    for (int x = 0; x < newImg.width(); x++) {
        for (int y = 0; y < newImg.height(); y++) {
            auto color = newImg.pixelColor(x, y);
            if (color.alpha() == 0)
                continue;
            color.setAlphaF(darkness * color.alphaF());
            newImg.setPixelColor(x, y, color);
        }
    }

    QPixmap darkerTarget = QPixmap::fromImage(newImg);
    painter2.begin(&darkerTarget);

    auto borderPath = caculateRelativePainterPath(widget, borderRadiusTopLeft + 0.5, borderRadiusTopRight + 0.5, borderRadiusBottomLeft + 0.5, borderRadiusBottomRight + 0.5);
    painter2.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    painter2.setRenderHint(QPainter::HighQualityAntialiasing);
    QColor borderColor = color;
    borderColor.setAlphaF(0.05);
    painter2.setPen(borderColor);
    painter2.setBrush(Qt::NoBrush);
    painter2.translate(shadow_border, shadow_border);
    painter2.translate(-0.5, -0.5);
//    painter2.drawPath(borderPath);

    return darkerTarget;
}

QPainterPath ConfigShadowHelper::caculateRelativePainterPath(QWidget *widget, qreal borderRadiusTopLeft, qreal borderRadiusTopRight, qreal borderRadiusBottomLeft, qreal borderRadiusBottomRight)
{
    qreal maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
    qreal maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
    qreal maxRadius = qMax(maxTopRadius, maxBottomRadius);
    maxRadius = qMax(12.0, maxRadius);

    qreal squareWidth = 2 * maxRadius + INNERRECT_WIDTH;

    QPainterPath windowRelativePath;
    windowRelativePath.setFillRule(Qt::WindingFill);
    QPointF currentPos;

    int qAppMaxradius = maxRadius;
    if(widget){
        if(widget->inherits("QComboBoxPrivateContainer") || widget->inherits("QMenu")){
            auto settings = LINGMOStyleSettings::globalInstance();
            if(settings && settings->keys().contains("windowRadius"))
                qAppMaxradius = settings->get("windowRadius").toInt();

        }
        else if(widget->property("maxRadius").isValid())
            qAppMaxradius = widget->property("maxRadius").toInt();
    }

    // move to top left arc start point
    windowRelativePath.moveTo(borderRadiusTopLeft, 0);
    // top left arc
    auto topLeftBorderRadiusRect = QRectF(0, 0, 2 * borderRadiusTopLeft, 2 * borderRadiusTopLeft);
    if(qAppMaxradius > 0)
        windowRelativePath.arcTo(topLeftBorderRadiusRect, 90, 90);
    else
        windowRelativePath.addRect(topLeftBorderRadiusRect);

    // move to bottom left arc start point
    currentPos = QPointF(0, maxRadius + INNERRECT_WIDTH + maxRadius - borderRadiusBottomLeft);
    //windowRelativePath.moveTo(currentPos);
    // bottom left arc
    auto bottomLeftRect = QRectF(0, currentPos.y() - borderRadiusBottomLeft, 2 * borderRadiusBottomLeft, 2 * borderRadiusBottomLeft);
    if(qAppMaxradius > 0)
        windowRelativePath.arcTo(bottomLeftRect, 180, 90);
    else
        windowRelativePath.addRect(bottomLeftRect);

    // move to bottom right arc start point
    currentPos = QPointF(2 * maxRadius + INNERRECT_WIDTH - borderRadiusBottomRight, 2 * maxRadius + INNERRECT_WIDTH);
    //windowRelativePath.moveTo(currentPos);
    // bottom right arc
    auto bottomRightRect = QRectF(currentPos.x() - borderRadiusBottomRight, currentPos.y() - 2 * borderRadiusBottomRight, 2 * borderRadiusBottomRight, 2 * borderRadiusBottomRight);
    if(qAppMaxradius > 0)
        windowRelativePath.arcTo(bottomRightRect, 270, 90);
    else
        windowRelativePath.addRect(bottomRightRect);

    // move to top right arc start point
    currentPos = QPointF(2 * maxRadius + INNERRECT_WIDTH, borderRadiusTopRight);
    //windowRelativePath.moveTo(currentPos);
    // top right arc
    auto topRightRect = QRectF(squareWidth - 2 * borderRadiusTopRight, 0, 2 * borderRadiusTopRight, 2 * borderRadiusTopRight);
    if(qAppMaxradius > 0)
        windowRelativePath.arcTo(topRightRect, 0, 90);
    else
        windowRelativePath.addRect(topRightRect);

    return windowRelativePath;
}

KWindowShadow *ConfigShadowHelper::getShadow(QColor color, int shadow_border, qreal darkness, QWidget* widget, int borderRadiusTopLeft, int borderRadiusTopRight, int borderRadiusBottomLeft, int borderRadiusBottomRight)
{
    QPixmap shadowPixmap = getShadowPixmap(color, shadow_border, darkness, widget, borderRadiusTopLeft, borderRadiusTopRight, borderRadiusBottomLeft, borderRadiusBottomRight);
    qreal maxTopRadius = qMax(borderRadiusTopLeft, borderRadiusTopRight);
    qreal maxBottomRadius = qMax(borderRadiusBottomLeft, borderRadiusBottomRight);
    int maxRadius = qMax(maxTopRadius, maxBottomRadius);
    maxRadius = qMax(12, maxRadius);

    QPixmap topLeftPixmap = shadowPixmap.copy(0, 0, maxRadius + shadow_border, maxRadius + shadow_border);
    QPixmap topPixmap = shadowPixmap.copy(maxRadius + shadow_border, 0, INNERRECT_WIDTH, maxRadius + shadow_border);
    QPixmap topRightPixmap = shadowPixmap.copy(maxRadius + shadow_border + INNERRECT_WIDTH, 0, maxRadius + shadow_border, maxRadius + shadow_border);
    QPixmap leftPixmap = shadowPixmap.copy(0, maxRadius + shadow_border, maxRadius + shadow_border, INNERRECT_WIDTH);
    QPixmap rightPixmap = shadowPixmap.copy(maxRadius + shadow_border + INNERRECT_WIDTH, maxRadius + shadow_border, maxRadius + shadow_border, INNERRECT_WIDTH);
    QPixmap bottomLeftPixmap = shadowPixmap.copy(0, maxRadius + shadow_border + INNERRECT_WIDTH, maxRadius + shadow_border, maxRadius + shadow_border);
    QPixmap bottomPixmap = shadowPixmap.copy(maxRadius + shadow_border, maxRadius + shadow_border + INNERRECT_WIDTH, INNERRECT_WIDTH, maxRadius + shadow_border);
    QPixmap bottomRightPixmap = shadowPixmap.copy(maxRadius + shadow_border + INNERRECT_WIDTH, maxRadius + shadow_border + INNERRECT_WIDTH, maxRadius + shadow_border, maxRadius + shadow_border);

    KWindowShadow *shadow = new KWindowShadow;
    KWindowShadowTile::Ptr topLeftTile = KWindowShadowTile::Ptr::create();
    topLeftTile.get()->setImage(topLeftPixmap.toImage());
    shadow->setTopLeftTile(topLeftTile);
    KWindowShadowTile::Ptr topTile = KWindowShadowTile::Ptr::create();
    topTile.get()->setImage(topPixmap.toImage());
    shadow->setTopTile(topTile);
    KWindowShadowTile::Ptr topRightTile = KWindowShadowTile::Ptr::create();
    topRightTile.get()->setImage(topRightPixmap.toImage());
    shadow->setTopRightTile(topRightTile);
    KWindowShadowTile::Ptr leftTile = KWindowShadowTile::Ptr::create();
    leftTile.get()->setImage(leftPixmap.toImage());
    shadow->setLeftTile(leftTile);
    KWindowShadowTile::Ptr rightTile = KWindowShadowTile::Ptr::create();
    rightTile.get()->setImage(rightPixmap.toImage());
    shadow->setRightTile(rightTile);
    KWindowShadowTile::Ptr bottomLeftTile = KWindowShadowTile::Ptr::create();
    bottomLeftTile.get()->setImage(bottomLeftPixmap.toImage());
    shadow->setBottomLeftTile(bottomLeftTile);
    KWindowShadowTile::Ptr bottomTile = KWindowShadowTile::Ptr::create();
    bottomTile.get()->setImage(bottomPixmap.toImage());
    shadow->setBottomTile(bottomTile);
    KWindowShadowTile::Ptr bottomRightTile = KWindowShadowTile::Ptr::create();
    bottomRightTile.get()->setImage(bottomRightPixmap.toImage());
    shadow->setBottomRightTile(bottomRightTile);

    return shadow;
}

bool ConfigShadowHelper::eventFilter(QObject *watched, QEvent *event)
{
    if (watched->isWidgetType()) {
        auto widget = qobject_cast<QWidget *>(watched);
        if (QX11Info::isPlatformX11() && event->type() == QEvent::Show) {
            if (watched->property("useCustomShadow").toBool() && widget->isTopLevel()) {
                auto shadowColor = widget->palette().text().color();

                int shadowBorder = widget->property("customShadowWidth").toInt();
                bool ok = false;
                qreal darkness = widget->property("customShadowDarkness").toReal(&ok);
                if (!ok) {
                    darkness = 1.0;
                }
                QVector4D radius = qvariant_cast<QVector4D>(widget->property("customShadowRadius"));
                QVector4D margins = qvariant_cast<QVector4D>(widget->property("customShadowMargins"));

                if (auto tmp = m_shadows.value(widget)) {
                    if (tmp->isCreated())
                        return false;
                }

                auto shadow = getShadow(shadowColor, shadowBorder, darkness, widget, radius.x(), radius.y(), radius.z(), radius.w());
                shadow->setPadding(QMargins(margins.x(), margins.y(), margins.z(), margins.w()));
                shadow->setWindow(widget->windowHandle());
                shadow->create();

                //qInfo()<<"try set custom shadow"<<shadowBorder<<darkness<<radius<<margins;

                m_shadows.insert(widget, shadow);

                connect(widget, &QWidget::destroyed, this, [=](){
                    if (auto shadowToBeDelete = m_shadows.value(widget)) {
                        if (shadowToBeDelete->isCreated())
                            shadowToBeDelete->destroy();
                        shadowToBeDelete->deleteLater();
                        m_shadows.remove(widget);
                    }
                });
            } else {
                if (!widget->testAttribute(Qt::WA_WState_Created) && !widget->isTopLevel())
                    return false;

                if (auto tmp = m_shadows.value(widget)) {
                    if (tmp->isCreated())
                        tmp->destroy();
                    tmp->deleteLater();
                    m_shadows.remove(widget);
                }

                auto shadowColor = widget->palette().text().color();

                int radius = 10;

                auto shadow = getShadow(shadowColor, 15, 0.5, widget, radius, radius, radius, radius);
                shadow->setPadding(QMargins(15, 15, 15, 15));
                shadow->setWindow(widget->windowHandle());
                shadow->create();
                m_shadows.insert(widget, shadow);

                connect(widget, &QWidget::destroyed, this, [=](){
                    if (auto shadowToBeDelete = m_shadows.value(widget)) {
                        if (shadowToBeDelete->isCreated())
                            shadowToBeDelete->destroy();
                        shadowToBeDelete->deleteLater();
                        m_shadows.remove(widget);
                    }
                });
            }
        }
    }

    return false;
}
