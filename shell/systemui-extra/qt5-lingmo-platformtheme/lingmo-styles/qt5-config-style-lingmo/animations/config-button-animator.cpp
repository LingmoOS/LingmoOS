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

#include "config-button-animator.h"
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>

using namespace LINGMOConfigStyleSpace;

ConfigButtonAnimator::ConfigButtonAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{

}

ConfigButtonAnimator::~ConfigButtonAnimator()
{
    if(m_mouseover){
        m_mouseover->deleteLater();
        m_mouseover = nullptr;
    }
    if(m_sunken){
        m_sunken->deleteLater();
        m_sunken = nullptr;
    }
}

bool ConfigButtonAnimator::bindWidget(QWidget *w)
{
    if (w->property("doNotAnimate").toBool())
        return false;
    if (qobject_cast<QToolButton*>(w) || qobject_cast<QPushButton*>(w) || qobject_cast<QComboBox*>(w)
            || qobject_cast<QSpinBox*>(w) || qobject_cast<QDoubleSpinBox*>(w)) {
        m_widget = w;
    } else {
        return false;
    }

    m_mouseover = new QVariantAnimation();
    m_mouseover->setStartValue(0.0);
    m_mouseover->setEndValue(1.0);
    m_mouseover->setDuration(100);
    m_mouseover->setEasingCurve(QEasingCurve::OutCubic);
    addAnimation(m_mouseover);

    m_sunken = new QVariantAnimation();
    m_sunken->setStartValue(0.0);
    m_sunken->setEndValue(1.0);
    m_sunken->setDuration(75);
    m_sunken->setEasingCurve(QEasingCurve::InCubic);
    addAnimation(m_sunken);

    connect(m_sunken, &QVariantAnimation::valueChanged, w, [=]() {
       w->update();
    });
    connect(m_mouseover, &QVariantAnimation::valueChanged, w, [=]() {
       w->update();
    });
    connect(m_sunken, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });
    connect(m_mouseover, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });
    return true;
}

bool ConfigButtonAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    if(m_mouseover){
        m_mouseover->deleteLater();
        m_mouseover = nullptr;
    }
    if(m_sunken){
        m_sunken->deleteLater();
        m_sunken = nullptr;
    }

    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;

}

QVariant ConfigButtonAnimator::value(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->currentValue();
    else if(property == "SunKen")
        return m_sunken->currentValue();
    else
        return QVariant();
}

bool ConfigButtonAnimator::isRunning(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->state() == Running;
    else if(property == "SunKen")
        return m_sunken->state() == Running;
    else
        return this->state() == Running;
}

bool ConfigButtonAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if(property == "MouseOver")
    {
        m_mouseover->setStartValue(value);
        return true;
    }
    else if(property == "SunKen")
    {
        m_sunken->setStartValue(value);
        return true;
    }
    else
    {
        return false;
    }
}

bool ConfigButtonAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if(property == "MouseOver")
    {
        m_mouseover->setEndValue(value);
        return true;
    }
    else if(property == "SunKen")
    {
        m_sunken->setEndValue(value);
        return true;
    }
    else
    {
        return false;
    }
}

bool ConfigButtonAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if(property == "MouseOver")
    {
        m_mouseover->setDuration(duration);
        return true;
    }
    else if(property == "SunKen")
    {
        m_sunken->setDuration(duration);
        return true;
    }
    else
    {
        return false;
    }
}

void ConfigButtonAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if(property == "MouseOver")
        m_mouseover->setDirection(d);
    else if(property == "SunKen")
        m_sunken->setDirection(d);
    else
        return;
}
void ConfigButtonAnimator::startAnimator(const QString &property)
{
    if(property == "MouseOver")
        m_mouseover->start();
    else if(property == "SunKen")
        m_sunken->start();
    else
        this->start();
}

void ConfigButtonAnimator::stopAnimator(const QString &property)
{
    if(property == "MouseOver") {
        m_mouseover->stop();
        m_mouseover->setCurrentTime(0);
    }
    else if(property == "SunKen")
    {
        m_sunken->stop();
        m_sunken->setCurrentTime(0);
    }
    else
    {
        this->stop();
        this->setCurrentTime(0);
    }
}

int ConfigButtonAnimator::currentAnimatorTime(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->currentTime();
    else if(property == "SunKen")
        return m_sunken->currentTime();
    else
        return this->currentTime();
}

void ConfigButtonAnimator::setAnimatorCurrentTime(const QString &property, const int msecs)
{
    if ("MouseOver" == property) {
        m_mouseover->setCurrentTime(msecs);
    } else if("SunKen" == property) {
        m_sunken->setCurrentTime(msecs);
    }
}

int ConfigButtonAnimator::totalAnimationDuration(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->duration();
    else if(property == "SunKen")
        return m_sunken->duration();
    else
        return this->duration();
}

QVariant ConfigButtonAnimator::endValue(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->endValue();
    else if(property == "SunKen")
        return m_sunken->endValue();
    else
        return this->endValue();
}
