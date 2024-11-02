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

#include "config-box-animator.h"

#include <QComboBox>

using namespace LINGMOConfigStyleSpace;

ConfigBoxAnimator::ConfigBoxAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{

}

ConfigBoxAnimator::~ConfigBoxAnimator()
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

bool ConfigBoxAnimator::bindWidget(QWidget *w)
{
    if (w->property("doNotAnimate").toBool())
        return false;

    if (qobject_cast<QComboBox*>(w)) {
        m_widget = w;
    } else {
        return false;
    }

    m_mouseover = new QVariantAnimation(this);
    m_mouseover->setStartValue(0.0);
    m_mouseover->setEndValue(1.0);
    m_mouseover->setDuration(100);
    m_mouseover->setEasingCurve(QEasingCurve::OutCubic);
    addAnimation(m_mouseover);

    m_sunken = new QVariantAnimation(this);
    m_sunken->setStartValue(0.0);
    m_sunken->setEndValue(1.0);
    m_sunken->setDuration(75);
    m_sunken->setEasingCurve(QEasingCurve::InCubic);
    addAnimation(m_sunken);

    connect(m_sunken, &QVariantAnimation::valueChanged, w, [=]() {
       w->repaint();
    });
    connect(m_mouseover, &QVariantAnimation::valueChanged, w, [=]() {
       w->repaint();
    });
    connect(m_sunken, &QVariantAnimation::finished, w, [=]() {
       w->repaint();
    });
    connect(m_mouseover, &QVariantAnimation::finished, w, [=]() {
       w->repaint();
    });

    return true;

}

bool ConfigBoxAnimator::unboundWidget()
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

QVariant ConfigBoxAnimator::value(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->currentValue();
    else if(property == "SunKen")
        return m_sunken->currentValue();
    else
        return QVariant();
}

bool ConfigBoxAnimator::isRunning(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->state() == Running;
    else if(property == "SunKen")
        return m_sunken->state() == Running;
    else
        return this->state() == Running;
}

bool ConfigBoxAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
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

bool ConfigBoxAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
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

bool ConfigBoxAnimator::setAnimatorDuration(const QString &property, int duration)
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

void ConfigBoxAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if(property == "MouseOver")
        m_mouseover->setDirection(d);
    else if(property == "SunKen")
        m_sunken->setDirection(d);
    else
        return;
}
void ConfigBoxAnimator::startAnimator(const QString &property)
{
    if(property == "MouseOver")
        m_mouseover->start();
    else if(property == "SunKen")
        m_sunken->start();
    else
        this->start();
}

void ConfigBoxAnimator::stopAnimator(const QString &property)
{
    if(property == "MouseOver")
    {
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

int ConfigBoxAnimator::currentAnimatorTime(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->currentTime();
    else if(property == "SunKen")
        return m_sunken->currentTime();
    else
        return this->currentTime();
}

int ConfigBoxAnimator::totalAnimationDuration(const QString &property)
{
    if(property == "MouseOver")
        return m_mouseover->duration();
    else if(property == "SunKen")
        return m_sunken->duration();
    else
        return this->duration();
}

