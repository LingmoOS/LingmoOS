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

#include "config-slider-animator.h"
#include <QSlider>

#include <QVariantAnimation>

#include <QDebug>

using namespace LINGMOConfigStyleSpace;

ConfigSliderAnimator::ConfigSliderAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{

}

ConfigSliderAnimator::~ConfigSliderAnimator()
{
    if(m_slider_opacity){
        m_slider_opacity->deleteLater();
        m_slider_opacity = nullptr;
    }
    if(m_hover_bg_width){
        m_hover_bg_width->deleteLater();
        m_hover_bg_width = nullptr;
    }
    if(m_sunken_silder_additional_opacity){
        m_sunken_silder_additional_opacity->deleteLater();
        m_sunken_silder_additional_opacity = nullptr;
    }
    if(m_silder_move_position){
        m_silder_move_position->deleteLater();
        m_silder_move_position = nullptr;
    }
}

/*!
 * \brief ConfigSliderAnimator::bindWidget
 * \param w
 * \return
 *
 * \details
 * QObject has a feature that parent object can use findChild() method
 * getting a specific named child.
 *
 * I use QObject::setObjectName() set my animator and bind to a scroll bar.
 * So that i could not cache a hash or map to manage animators.
 *
 * \bug
 * Cause I use named QObject child to cache the animator for a scrollbar,
 * However there are some troubles for my unexcepted.
 *
 * For example, qt5 assistant's main view can not find child correctly.
 * I don't know if animator bind with child was been removed at some times.
 */
bool ConfigSliderAnimator::bindWidget(QWidget *w)
{
    if (w->property("doNotAnimate").toBool())
        return false;

    if (qobject_cast<QSlider*>(w)) {
        m_widget = w;
    } else {
        return false;
    }

    m_slider_opacity = new QVariantAnimation(this);
    m_slider_opacity->setStartValue(0.0);
    m_slider_opacity->setEndValue(1.0);
    m_slider_opacity->setDuration(200);
    m_slider_opacity->setEasingCurve(QEasingCurve::InOutCubic);
    addAnimation(m_slider_opacity);

    m_hover_bg_width = new QVariantAnimation(this);
    m_hover_bg_width->setStartValue(0.0);
    m_hover_bg_width->setEndValue(1.0);
    m_hover_bg_width->setDuration(200);
    m_hover_bg_width->setEasingCurve(QEasingCurve::InOutCubic);
    addAnimation(m_hover_bg_width);

    m_sunken_silder_additional_opacity = new QVariantAnimation(this);
    m_sunken_silder_additional_opacity->setStartValue(0.0);
    m_sunken_silder_additional_opacity->setEndValue(0.10);
    m_sunken_silder_additional_opacity->setDuration(150);
    addAnimation(m_sunken_silder_additional_opacity);

    m_silder_move_position = new QVariantAnimation(this);
    m_silder_move_position->setStartValue(0.0);
    m_silder_move_position->setEndValue(1.0);
    m_silder_move_position->setDuration(150);
    m_silder_move_position->setEasingCurve(QEasingCurve::InOutCubic);
    addAnimation(m_silder_move_position);

    connect(m_slider_opacity, &QVariantAnimation::valueChanged, w, [=]() {
       w->update();
    });
    connect(m_sunken_silder_additional_opacity, &QVariantAnimation::valueChanged, w, [=]() {
       w->update();
    });
    connect(m_silder_move_position, &QVariantAnimation::valueChanged, w, [=]() {
       w->update();
    });
    connect(m_hover_bg_width, &QVariantAnimation::valueChanged, w, [=]() {
       w->update();
    });
    connect(m_slider_opacity, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });
    connect(m_sunken_silder_additional_opacity, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });
    connect(m_silder_move_position, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });
    connect(m_hover_bg_width, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });

    return true;
}

bool ConfigSliderAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    if(m_slider_opacity){
        m_slider_opacity->deleteLater();
        m_slider_opacity = nullptr;
    }
    if(m_hover_bg_width){
        m_hover_bg_width->deleteLater();
        m_hover_bg_width = nullptr;
    }
    if(m_sunken_silder_additional_opacity){
        m_sunken_silder_additional_opacity->deleteLater();
        m_sunken_silder_additional_opacity = nullptr;
    }
    if(m_silder_move_position){
        m_silder_move_position->deleteLater();
        m_silder_move_position = nullptr;
    }

    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;
}

QVariant ConfigSliderAnimator::value(const QString &property)
{
    if (property == "slider_opacity") {
        return m_slider_opacity->currentValue();
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->currentValue();
    } else if (property == "move_position") {
        return m_silder_move_position->currentValue();
    } else if (property == "bg_width") {
        return m_hover_bg_width->currentValue();
    } else {
        return QVariant();
    }
}

bool ConfigSliderAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if (property == "slider_opacity") {
        m_slider_opacity->setStartValue(value);
        return true;
    } else if (property == "additional_opacity")
    {
        m_sunken_silder_additional_opacity->setStartValue(value);
        return true;
    }  else if (property == "move_position") {
        m_silder_move_position->setStartValue(value);
        return true;
    } else if (property == "bg_width") {
        m_hover_bg_width->setStartValue(value);
        return true;
    } else {
        return false;
    }
}

bool ConfigSliderAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if (property == "slider_opacity") {
        m_slider_opacity->setEndValue(value);
        return true;
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->setEndValue(value);
        return true;
    } else if (property == "move_position") {
        m_silder_move_position->setEndValue(value);
        return true;
    } else if (property == "bg_width") {
        m_hover_bg_width->setEndValue(value);
        return true;
    } else {
        return false;
    }
}

QVariant ConfigSliderAnimator::animatorStartValue(const QString &property)
{
    if (property == "slider_opacity") {
        return m_slider_opacity->startValue();
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->startValue();
    } else if (property == "move_position") {
        return m_silder_move_position->startValue();
    } else if (property == "bg_width") {
        return m_hover_bg_width->startValue();
    } else {
        return QVariant();
    }
}

QVariant ConfigSliderAnimator::animatorEndValue(const QString &property)
{
    if (property == "slider_opacity") {
        return m_slider_opacity->endValue();
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->endValue();
    } else if (property == "move_position") {
        return m_silder_move_position->endValue();
    } else if (property == "bg_width") {
        return m_hover_bg_width->endValue();
    } else {
        return QVariant();
    }
}


bool ConfigSliderAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if (property == "slider_opacity") {
        m_slider_opacity->setDuration(duration);
        return true;
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->setDuration(duration);
        return true;
    } else if (property == "move_position") {
        m_silder_move_position->setDuration(duration);
        return true;
    } else if (property == "bg_width") {
        m_hover_bg_width->setDuration(duration);
        return true;
    } else {
        return false;
    }
}

void ConfigSliderAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if (property == "slider_opacity") {
        m_slider_opacity->setDirection(d);
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->setDirection(d);
    } else if (property == "move_position") {
        m_silder_move_position->setDirection(d);
    } else if (property == "bg_width") {
        m_hover_bg_width->setDirection(d);
    } else {
        return;
    }
}

bool ConfigSliderAnimator::isRunning(const QString &property)
{
    if (property == "slider_opacity") {
        return m_slider_opacity->state() == Running;
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->state() == Running;
    } else if (property == "move_position") {
        return m_silder_move_position->state() == Running;
    } else if (property == "bg_width") {
        return m_hover_bg_width->state() == Running;
    } else {
        return this->state() == Running;
    }
}

void ConfigSliderAnimator::startAnimator(const QString &property)
{
    if (property == "slider_opacity") {
        m_slider_opacity->start();
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->start();
    } else if (property == "move_position") {
        m_silder_move_position->start();
    } else if (property == "bg_width") {
        m_hover_bg_width->start();
    } else {
        this->start();
    }
}

void ConfigSliderAnimator::stopAnimator(const QString &property)
{
    if (property == "slider_opacity") {
        m_slider_opacity->stop();
    } else if (property == "additional_opacity") {
        m_sunken_silder_additional_opacity->stop();
    } else if (property == "move_position") {
        m_silder_move_position->stop();
    } else if (property == "bg_width") {
        m_hover_bg_width->stop();
    } else {
        this->stop();
    }
}

int ConfigSliderAnimator::currentAnimatorTime(const QString &property)
{
    if (property == "slider_opacity") {
        return m_slider_opacity->currentTime();
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->currentTime();
    } else if (property == "move_position") {
        return m_silder_move_position->currentTime();
    } else if (property == "bg_width") {
        return m_hover_bg_width->currentTime();
    } else {
        return this->currentTime();
    }
}

void ConfigSliderAnimator::setAnimatorCurrentTime(const QString &property, const int msecs)
{
    if ("move_position" == property) {
        m_silder_move_position->setCurrentTime(msecs);
    }
}


int ConfigSliderAnimator::totalAnimationDuration(const QString &property)
{
    if (property == "slider_opacity") {
        return m_slider_opacity->duration();
    } else if (property == "additional_opacity") {
        return m_sunken_silder_additional_opacity->duration();
    } else if (property == "move_position") {
        return m_silder_move_position->duration();
    } else if (property == "bg_width") {
        return m_hover_bg_width->duration();
    } else {
        return this->duration();
    }
}

void ConfigSliderAnimator::setExtraProperty(const QString &property, const QVariant &value)
{
    if(property == "end_position")
        m_endPosition = value.toInt();
    else if(property == "start_position")
        m_startPosition = value.toInt();
    else if(property == "addValue")
        m_addValue = value.toBool();
}

QVariant ConfigSliderAnimator::getExtraProperty(const QString &property)
{
    if(property == "end_position")
        return m_endPosition;
    else if(property == "start_position")
        return m_startPosition;
    else if(property == "addValue")
        return m_addValue;
    return QVariant();
}

