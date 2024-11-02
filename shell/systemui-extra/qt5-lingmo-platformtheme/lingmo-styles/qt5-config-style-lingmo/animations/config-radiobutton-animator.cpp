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

#include "config-radiobutton-animator.h"

#include <QRadioButton>
#include <QDebug>

using namespace LINGMOConfigStyleSpace;

ConfigRadioButtonAnimator::ConfigRadioButtonAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{

}

ConfigRadioButtonAnimator::~ConfigRadioButtonAnimator()
{
    if(m_off){
        m_off->deleteLater();
        m_off = nullptr;
    }
    if(m_sunkenon){
        m_sunkenon->deleteLater();
        m_sunkenon = nullptr;
    }
}

bool ConfigRadioButtonAnimator::bindWidget(QWidget *w)
{
    if (w->property("doNotAnimate").toBool())
        return false;

    if (qobject_cast<QRadioButton*>(w)) {
        m_widget = w;
    } else {
        return false;
    }

    m_sunkenon = new QVariantAnimation(this);
    m_sunkenon->setStartValue(0.0);
    m_sunkenon->setEndValue(1.0);
    m_sunkenon->setDuration(200);
    m_sunkenon->setEasingCurve(QEasingCurve::OutCubic);

    addAnimation(m_sunkenon);

    connect(m_sunkenon, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_sunkenon, &QVariantAnimation::finished, w, [=]() {
        w->update();
    });

    m_off = new QVariantAnimation(this);
    m_off->setStartValue(1.0);
    m_off->setEndValue(0.0);
    m_off->setDuration(100);
    m_off->setEasingCurve(QEasingCurve::OutCubic);

    addAnimation(m_off);

    connect(m_off, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_off, &QVariantAnimation::finished, w, [=]() {
        w->update();
    });
    return true;

}

bool ConfigRadioButtonAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    if(m_off){
        m_off->deleteLater();
        m_off = nullptr;
    }
    if(m_sunkenon){
        m_sunkenon->deleteLater();
        m_sunkenon = nullptr;
    }

    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;

}

QVariant ConfigRadioButtonAnimator::value(const QString &property)
{
    if(property == "SunKenOn")
        return m_sunkenon->currentValue();
    else if(property == "Off")
        return m_off->currentValue();
    else
        return QVariant();
}

bool ConfigRadioButtonAnimator::isRunning(const QString &property)
{
    if(property == "SunKenOn"){
        return m_sunkenon->state() == Running;
    }
    else if(property == "Off")
        return m_off->state() == Running;
    else
        return this->state() == Running;
}

bool ConfigRadioButtonAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if(property == "SunKenOn")
    {
        m_sunkenon->setStartValue(value);
        return true;
    }
    else if(property == "Off")
    {
        m_off->setStartValue(value);
        return true;
    }
    else
    {
        return false;
    }
}

bool ConfigRadioButtonAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if(property == "SunKenOn")
    {
        m_sunkenon->setEndValue(value);
        return true;
    }
    else if(property == "Off")
    {
        m_off->setEndValue(value);
        return true;
    }
    else
    {
        return false;
    }
}

bool ConfigRadioButtonAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if(property == "SunKenOn")
    {
        m_sunkenon->setDuration(duration);
        return true;
    }
    else if(property == "Off")
    {
        m_off->setDuration(duration);
        return true;
    }
    else
    {
        return false;
    }
}

void ConfigRadioButtonAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if(property == "SunKenOn")
        m_sunkenon->setDirection(d);
    else if(property == "Off")
    {
        m_off->setDirection(d);
    }
    else
        return;
}
void ConfigRadioButtonAnimator::startAnimator(const QString &property)
{
    if(property == "SunKenOn")
    {
        m_sunkenon->start();
    }
    else if(property == "Off")
    {
        m_off->start();
    }
    else
        this->start();
}

void ConfigRadioButtonAnimator::stopAnimator(const QString &property)
{
    if(property == "SunKenOn")
    {
        m_sunkenon->stop();
        m_sunkenon->setCurrentTime(0);
    }
    else if(property == "Off")
    {
        m_off->stop();
        m_off->setCurrentTime(0);
    }
    else
    {
        this->stop();
        this->setCurrentTime(0);
    }
}

int ConfigRadioButtonAnimator::currentAnimatorTime(const QString &property)
{
    if(property == "SunKenOn")
        return m_sunkenon->currentTime();
    if(property == "Off")
        return m_off->currentTime();
    else
        return this->currentTime();
}

void ConfigRadioButtonAnimator::setAnimatorCurrentTime(const QString &property, const int msecs)
{
    if ("SunKenOn" == property) {
        m_sunkenon->setCurrentTime(msecs);
    } else if("Off" == property) {
        m_off->setCurrentTime(msecs);
    }
}

int ConfigRadioButtonAnimator::totalAnimationDuration(const QString &property)
{
    if(property == "SunKenOn")
        return m_sunkenon->duration();
    if(property == "Off")
        return m_off->duration();
    else
        return this->duration();
}

