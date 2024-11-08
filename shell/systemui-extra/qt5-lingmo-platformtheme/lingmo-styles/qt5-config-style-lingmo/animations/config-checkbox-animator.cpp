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

#include "config-checkbox-animator.h"

#include <QCheckBox>
#include <QDebug>

using namespace LINGMOConfigStyleSpace;

ConfigCheckBoxAnimator::ConfigCheckBoxAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{

}

ConfigCheckBoxAnimator::~ConfigCheckBoxAnimator()
{
    if(m_onScale){
        m_onScale->deleteLater();
        m_onScale = nullptr;
    }
    if(m_onOpacity){
        m_onOpacity->deleteLater();
        m_onOpacity = nullptr;
    }
    if(m_onBase){
        m_onBase->deleteLater();
        m_onBase = nullptr;
    }
    if(m_off){
        m_off->deleteLater();
        m_off = nullptr;
    }
}

bool ConfigCheckBoxAnimator::bindWidget(QWidget *w)
{
    if (w->property("doNotAnimate").toBool())
        return false;

    if (qobject_cast<QCheckBox*>(w)) {
        m_widget = w;
    } else {
        return false;
    }

    m_onScale = new QVariantAnimation(this);
    m_onScale->setStartValue(0.0);
    m_onScale->setEndValue(1.0);
    m_onScale->setDuration(260);
    m_onScale->setEasingCurve(QEasingCurve::OutCubic);
    addAnimation(m_onScale);
    connect(m_onScale, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_onScale, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });

    m_onOpacity = new QVariantAnimation(this);
    m_onOpacity->setStartValue(0.0);
    m_onOpacity->setEndValue(1.0);
    m_onOpacity->setDuration(120);
    m_onOpacity->setEasingCurve(QEasingCurve::OutCubic);
    addAnimation(m_onOpacity);
    connect(m_onOpacity, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_onOpacity, &QVariantAnimation::finished, w, [=]() {
       w->update();
    });

    m_onBase = new QVariantAnimation(this);
    m_onBase->setStartValue(0.0);
    m_onBase->setEndValue(1.0);
    m_onBase->setDuration(180);
    m_onBase->setEasingCurve(QEasingCurve::OutCubic);
    addAnimation(m_onBase);
    connect(m_onBase, &QVariantAnimation::valueChanged, w, [=](const QVariant &value) {
       w->update();
    });
    connect(m_onBase, &QVariantAnimation::finished, w, [=]() {
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

bool ConfigCheckBoxAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    if(m_onScale){
        m_onScale->deleteLater();
        m_onScale = nullptr;
    }
    if(m_onOpacity){
        m_onOpacity->deleteLater();
        m_onOpacity = nullptr;
    }
    if(m_onBase){
        m_onBase->deleteLater();
        m_onBase = nullptr;
    }
    if(m_off){
        m_off->deleteLater();
        m_off = nullptr;
    }

    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;

}

QVariant ConfigCheckBoxAnimator::value(const QString &property)
{
    if(property == "OnScale")
        return m_onScale->currentValue();
    else if(property == "OnOpacity")
        return m_onOpacity->currentValue();
    else if(property == "OnBase")
        return m_onBase->currentValue();
    else if(property == "Off")
        return m_off->currentValue();
    else
        return QVariant();
}

bool ConfigCheckBoxAnimator::isRunning(const QString &property)
{
    if(property == "OnScale")
        return m_onScale->state() == Running;
    else if(property == "OnOpacity")
        return m_onOpacity->state() == Running;
    else if(property == "OnBase")
        return m_onBase->state() == Running;
    else if(property == "Off")
        return m_off->state() == Running;
    else
        return this->state() == Running;
}

bool ConfigCheckBoxAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if(property == "OnScale")
    {
        m_onScale->setStartValue(value);
        return true;
    }
    else if(property == "OnOpacity")
    {
        m_onOpacity->setStartValue(value);
        return true;
    }
    else if(property == "OnBase")
    {
        m_onBase->setStartValue(value);
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

bool ConfigCheckBoxAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if(property == "OnScale")
    {
        m_onScale->setEndValue(value);
        return true;
    }
    else if(property == "OnOpacity")
    {
        m_onOpacity->setEndValue(value);
        return true;
    }
    else if(property == "OnBase")
    {
        m_onBase->setEndValue(value);
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

bool ConfigCheckBoxAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if(property == "OnScale")
    {
        m_onScale->setDuration(duration);
        return true;
    }
    else if(property == "OnOpacity")
    {
        m_onOpacity->setDuration(duration);
        return true;
    }
    else if(property == "OnBase")
    {
        m_onBase->setDuration(duration);
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

void ConfigCheckBoxAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if(property == "OnScale")
        m_onScale->setDirection(d);
    else if(property == "OnOpacity")
    {
        m_onOpacity->setDirection(d);
    }
    else if(property == "OnBase")
    {
        m_onBase->setDirection(d);
    }
    else if(property == "Off")
    {
        m_off->setDirection(d);
    }
    else
        return;
}
void ConfigCheckBoxAnimator::startAnimator(const QString &property)
{
    if(property == "OnScale")
        m_onScale->start();
    else if(property == "OnOpacity")
    {
        m_onOpacity->start();
    }
    else if(property == "OnBase")
    {
        m_onBase->start();
    }
    else if(property == "Off")
    {
        m_off->start();
    }
    else
        this->start();
}

void ConfigCheckBoxAnimator::stopAnimator(const QString &property)
{
    if(property == "OnScale")
    {
        m_onScale->stop();
        m_onScale->setCurrentTime(0);
    }
    else if(property == "OnOpacity")
    {
        m_onOpacity->stop();
        m_onOpacity->setCurrentTime(0);
    }
    else if(property == "OnBase")
    {
        m_onBase->stop();
        m_onBase->setCurrentTime(0);
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

int ConfigCheckBoxAnimator::currentAnimatorTime(const QString &property)
{
    if(property == "OnScale")
        return m_onScale->currentTime();
    else if(property == "OnOpacity")
    {
        return m_onOpacity->currentTime();
    }
    else if(property == "OnBase")
    {
        return m_onBase->currentTime();
    }
    else if(property == "Off")
        return m_off->currentTime();
    else
        return this->currentTime();
}

void ConfigCheckBoxAnimator::setAnimatorCurrentTime(const QString &property, const int msecs)
{
    if ("OnScale" == property) {
        m_onScale->setCurrentTime(msecs);
    }
    else if(property == "OnOpacity")
    {
        m_onOpacity->setCurrentTime(msecs);
    }
    else if(property == "OnBase")
    {
        m_onBase->setCurrentTime(msecs);
    }
    else if("Off" == property) {
        m_off->setCurrentTime(msecs);
    }
}

int ConfigCheckBoxAnimator::totalAnimationDuration(const QString &property)
{
    if(property == "OnScale")
        return m_onScale->duration();
    else if(property == "OnOpacity")
    {
        return m_onOpacity->duration();
    }
    else if(property == "OnBase")
    {
        return m_onBase->duration();
    }
    else if(property == "Off")
        return m_off->duration();
    else
        return this->duration();
}

void ConfigCheckBoxAnimator::setExtraProperty(const QString &property, const QVariant &value)
{
    if(property == "nochange")
        m_noChange = value.toBool();
}

QVariant ConfigCheckBoxAnimator::getExtraProperty(const QString &property)
{
    if(property == "nochange")
        return m_noChange;
    return QVariant();
}
