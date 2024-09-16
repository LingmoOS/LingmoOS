// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "filterpreviewbutton.h"

extern "C" {
#include <libimagevisualresult/visualresult.h>
}

#include <QPainter>
#include <QPainterPath>

#include <DApplicationHelper>

DWIDGET_USE_NAMESPACE;

#define MARGIN 5
#define IMAGE_SIZE 40
#define BUTTON_SIZE (IMAGE_SIZE + MARGIN * 2)

filterPreviewButton::filterPreviewButton(QWidget *parent, efilterType filter/* = filter_Normal*/): QWidget(parent)
  , m_filterType(filter)
{
    QString objName = filterName_CUBE(filter);
    if (objName.isEmpty())
        objName = "normal";
    setObjectName(objName);

    m_color.setRgb(44, 44, 44);

    m_hover = false;
    m_mousePress = false;
    m_isSelected = false;
    m_disableSelect = false;

    setWindowOpacity(0.1);
    resize(BUTTON_SIZE, BUTTON_SIZE);
}

void filterPreviewButton::setbackground(QColor color)
{
    m_color = color;
}

void filterPreviewButton::setImage(QImage *img)
{
    Q_ASSERT(img);

    if (isHidden() && !img->isNull())
        return;

    uint8_t* frame = img->bits();
    int width = img->width();
    int height = img->height();
    QString filterName_CUBE = filterPreviewButton::filterName_CUBE(m_filterType);
    if (!filterName_CUBE.isEmpty())
        imageFilter24(frame, width, height, filterName_CUBE.toStdString().c_str(), 100);

    m_pixmap = QPixmap::fromImage(QImage(frame, width, height, QImage::Format_RGB888));
    update();
}

void filterPreviewButton::setSelected(bool selected)
{
    m_isSelected = selected;
    update();
}

bool filterPreviewButton::isSelected()
{
    return m_isSelected;
}

void filterPreviewButton::setButtonRadius(int radius)
{
    m_radius = radius;
    update();
}

void filterPreviewButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHints(QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform | QPainter::Antialiasing);

    QRect imageRect = QRect(MARGIN, MARGIN, IMAGE_SIZE, IMAGE_SIZE);

    QPainterPath roundPixmapRectPath;
    roundPixmapRectPath.addRoundedRect(imageRect, m_radius, m_radius);

    //绘制滤镜预览图
    if (!m_pixmap.isNull()) {
        painter.save();
        painter.setClipPath(roundPixmapRectPath);
        painter.drawPixmap(imageRect, m_pixmap);
        painter.restore();
    } else {
        painter.fillPath(roundPixmapRectPath, QBrush(QColor(m_color)));
    }

    // 预览图边缘绘制透明描边
    painter.save();
    QColor borderPenColor("#000000");
    borderPenColor.setAlphaF(0.12);
    QPen borderPen(borderPenColor);
    borderPen.setWidthF(2);
    painter.setPen(borderPen);
    painter.drawRoundedRect(imageRect.adjusted(-1, -1, 1, 1), m_radius, m_radius);
    painter.restore();

    QColor highlightColor = DGuiApplicationHelper::instance()->applicationPalette().highlight().color();

    //绘制点击效果
    QRect grayRect = imageRect;
    if (m_mousePress) {
        if (m_pixmap.isNull())
            painter.setBrush(QBrush(QColor(0, 0, 0, 255 * 1)));
        else
            painter.setBrush(QBrush(QColor(0, 0, 0, 255 * 0.4)));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(grayRect, m_radius, m_radius);
    } else if (m_hover) {
        if (m_pixmap.isNull())
            painter.setBrush(QBrush(QColor(0, 0, 0, 255 * 0)));
        else
            painter.setBrush(QBrush(QColor(0, 0, 0, 255 * 0.2)));
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(grayRect, m_radius, m_radius);
    }

    QRect focusWhite = imageRect;
    QRect focusBlue = imageRect.adjusted(-2, -2, 2, 2);
    if (m_hover || m_isSelected && !m_disableSelect) {
        painter.setPen(QPen(QColor(Qt::white), m_radius / 4));
        painter.drawRoundedRect(focusWhite, m_radius, m_radius);

        if (m_hover)
            highlightColor = Qt::white;
        painter.setPen(QPen(QColor(highlightColor), m_radius / 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(focusBlue, m_radius, m_radius);
    }

    QWidget::paintEvent(event);
}

void filterPreviewButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    this->setFocus();
    m_hover = true;
    update();
}

void filterPreviewButton::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    m_hover = true;
    update();
}

void filterPreviewButton::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    m_hover = false;
    update();
}

void filterPreviewButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    this->clearFocus();
    m_hover = false;
    m_mousePress = false;
    update();
}

void filterPreviewButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_mousePress = true;
    setSelected(true);
    update();
    DWidget::mousePressEvent(event);
}

void filterPreviewButton::mouseMoveEvent(QMouseEvent *event)
{
    //解决bug 在按钮中可拖动相机界面，https://pms.uniontech.com/zentao/bug-view-100647.html
    Q_UNUSED(event);
    return;
}

void filterPreviewButton::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_mousePress = false;
    update();
    emit clicked(false);
    DWidget::mouseReleaseEvent(event);
}

void filterPreviewButton::setDisableSelect(bool disable)
{
    m_disableSelect = disable;
}

void filterPreviewButton::setOpacity(int opacity)
{
    m_opacity = opacity;
    m_color.setAlpha(opacity);
    update();
}

efilterType filterPreviewButton::getFiltertype()
{
    return m_filterType;
}

filterPreviewButton::~filterPreviewButton()
{

}

QString filterPreviewButton::filterName(efilterType type)
{
    QString name = tr("Normal");
    switch (type) {
    case filter_Normal:
        name = tr("Normal");
        break;
    case filter_Vivid:
        name = tr("Vivid");
        break;
    case filter_Warm:
        name = tr("Warm");
        break;
    case filter_Cold:
        name = tr("Cold");
        break;
    case filter_Retro:
        name = tr("Retro");
        break;
    case filter_Film:
        name = tr("Film");
        break;
    case filter_Gray:
        name = tr("Gray");
        break;
    case filter_BAndW:
        name = tr("B&W");
        break;
    default:
        name = tr("Normal");
        break;
    }

    return name;
}

QString filterPreviewButton::filterName_CUBE(efilterType type)
{
    QString name = "";
    switch (type) {
    case filter_Normal:
        name = "";
        break;
    case filter_Vivid:
        name = "bright";
        break;
    case filter_Warm:
        name = "warm";
        break;
    case filter_Cold:
        name = "cold";
        break;
    case filter_Retro:
        name = "classic";
        break;
    case filter_Film:
        name = "print";
        break;
    case filter_Gray:
        name = "gray";
        break;
    case filter_BAndW:
        name = "black";
        break;
    default:
        name = "";
        break;
    }

    return name;
}
