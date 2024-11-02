/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "ukmedia_slider_tip_label_helper.h"
#include <QWidget>
#include <QMouseEvent>
#include <QApplication>
#include <QStyleOption>
#include <QStyle>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QStylePainter>
#include <QMouseEvent>
#include <QCoreApplication>
#include "lingmo_custom_style.h"

MediaSliderTipLabel::MediaSliderTipLabel(){
    setAttribute(Qt::WA_TranslucentBackground);
}

MediaSliderTipLabel::~MediaSliderTipLabel(){
}

void MediaSliderTipLabel::paintEvent(QPaintEvent *e)
{
    QStyleOptionFrame opt;
    initStyleOption(&opt);
    QStylePainter p(this);
//    p.setBrush(QBrush(QColor(0x1A,0x1A,0x1A,0x4C)));
    p.setBrush(QBrush(QColor(0xFF,0xFF,0xFF,0x33)));
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(this->rect(), 1, 1);
    QPainterPath path;
    path.addRoundedRect(opt.rect,6,6);
    p.setRenderHint(QPainter::Antialiasing);
    setProperty("blurRegion",QRegion(path.toFillPolygon().toPolygon()));
    p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
    this->setProperty("blurRegion", QRegion(QRect(0, 0, 1, 1)));
    QLabel::paintEvent(e);
}

SliderTipLabelHelper::SliderTipLabelHelper(QObject *parent) :QObject(parent)
{
    m_pTiplabel = new MediaSliderTipLabel();
    m_pTiplabel->setWindowFlags(Qt::ToolTip);
    qApp->installEventFilter(new AppEventFilter(this));
    m_pTiplabel->setFixedSize(52,30);
    m_pTiplabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
}

void SliderTipLabelHelper::registerWidget(QWidget *w)
{
    w->removeEventFilter(this);
    w->installEventFilter(this);
}

void SliderTipLabelHelper::unregisterWidget(QWidget *w)
{
    w->removeEventFilter(this);
}

bool SliderTipLabelHelper::eventFilter(QObject *obj, QEvent *e)
{
    auto slider = qobject_cast<UkmediaVolumeSlider *>(obj);
    if (obj == slider) {
        switch (e->type()) {
        case QEvent::MouseMove: {
            QMouseEvent *event = static_cast<QMouseEvent*>(e);
            mouseMoveEvent(obj, event);
            return false;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent *event = static_cast<QMouseEvent*>(e);
            mouseReleaseEvent(obj, event);
            return false;
        }
        case QEvent::MouseButtonPress:{
            QMouseEvent *event = static_cast<QMouseEvent*>(e);
            mousePressedEvent(obj,event);
        }

        default:
            return false;
        }
    }
    return QObject::eventFilter(obj,e);
}


void SliderTipLabelHelper::mouseMoveEvent(QObject *obj, QMouseEvent *e)
{
    Q_UNUSED(e);
    QRect rect;
    QStyleOptionSlider m_option;
    auto slider = qobject_cast<UkmediaVolumeSlider *>(obj);
    slider->initStyleOption(&m_option);
    rect = slider->style()->subControlRect(QStyle::CC_Slider, &m_option,QStyle::SC_SliderHandle,slider);
    QPoint gPos = slider->mapToGlobal(rect.topLeft());
    QString percent;
    percent = QString::number(slider->value());
    percent.append("%");
    m_pTiplabel->setText(percent);
    m_pTiplabel->move(gPos.x()-(m_pTiplabel->width()/2)+9,gPos.y()-m_pTiplabel->height()-1);
    m_pTiplabel->show();
}

void SliderTipLabelHelper::mouseReleaseEvent(QObject *obj, QMouseEvent *e)
{
    Q_UNUSED(obj);
    Q_UNUSED(e);
    m_pTiplabel->hide();
}

void SliderTipLabelHelper::mousePressedEvent(QObject *obj, QMouseEvent *e)
{
    Q_UNUSED(e);
    QStyleOptionSlider m_option;
    auto slider = qobject_cast<UkmediaVolumeSlider *>(obj);
    QRect rect;
    //获取鼠标的位置，这里并不能直接从ev中取值（因为如果是拖动的话，鼠标开始点击的位置没有意义了）
    double pos = e->pos().x() / (double)slider->width();
    slider->setValue(pos *(slider->maximum() - slider->minimum()) + slider->minimum());
    //向父窗口发送自定义事件event type，这样就可以在父窗口中捕获这个事件进行处理
    QEvent evEvent(static_cast<QEvent::Type>(QEvent::User + 1));
    QCoreApplication::sendEvent(obj, &evEvent);
    int value = pos *(slider->maximum() - slider->minimum()) + slider->minimum();

    slider->initStyleOption(&m_option);
    rect = slider->style()->subControlRect(QStyle::CC_Slider, &m_option,QStyle::SC_SliderHandle,slider);
    QPoint gPos = slider->mapToGlobal(rect.topLeft());
    QString percent;

    percent = QString::number(slider->value());//(m_option.sliderValue);
    percent.append("%");

    m_pTiplabel->setText(percent);
    m_pTiplabel->move(gPos.x()-(m_pTiplabel->width()/2)+9,gPos.y()-m_pTiplabel->height()-1);
    m_pTiplabel->show();
}

// AppEventFilter
AppEventFilter::AppEventFilter(SliderTipLabelHelper *parent) : QObject(parent)
{
    m_wm = parent;
}

bool AppEventFilter::eventFilter(QObject *obj, QEvent *e)
{
    Q_UNUSED(obj);
    Q_UNUSED(e);
    return false;
}

