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

#include "config-tree-animator.h"
#include <QTreeView>

#include <QVariantAnimation>

#include <QDebug>

using namespace LINGMOConfigStyleSpace;

ConfigTreeAnimator::ConfigTreeAnimator(QObject *parent) : QParallelAnimationGroup (parent)
{

}

ConfigTreeAnimator::~ConfigTreeAnimator()
{
    if(m_expand){
        m_expand->deleteLater();
        m_expand = nullptr;
    }
    if(m_collaps){
        m_collaps->deleteLater();
        m_collaps = nullptr;
    }
}
/*!
 * \brief ConfigTreeAnimator::bindWidget
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
bool ConfigTreeAnimator::bindWidget(QWidget *w)
{
    if (w->property("doNotAnimate").toBool())
        return false;

    if (qobject_cast<QTreeView*>(w)) {
        m_widget = w;
    } else {
        return false;
    }

    m_expand = new QVariantAnimation(this);
    m_expand->setStartValue(0.0);
    m_expand->setEndValue(1.0);
    m_expand->setDuration(250);
    m_expand->setEasingCurve(QEasingCurve::InOutCubic);
    addAnimation(m_expand);

    m_collaps = new QVariantAnimation(this);
    m_collaps->setStartValue(0.0);
    m_collaps->setEndValue(1.0);
    m_collaps->setDuration(250);
    m_collaps->setEasingCurve(QEasingCurve::InOutCubic);
    addAnimation(m_collaps);

    connect(m_expand, &QVariantAnimation::valueChanged, w, [=]() {
       qobject_cast<QTreeView*>(w)->viewport()->update();
    });
    connect(m_collaps, &QVariantAnimation::valueChanged, w, [=]() {
        qobject_cast<QTreeView*>(w)->viewport()->update();
    });
    connect(m_expand, &QVariantAnimation::finished, w, [=]() {
        qobject_cast<QTreeView*>(w)->viewport()->update();
    });
    connect(m_collaps, &QVariantAnimation::finished, w, [=]() {
        qobject_cast<QTreeView*>(w)->viewport()->update();
    });

    return true;
}

bool ConfigTreeAnimator::unboundWidget()
{
    this->stop();
    this->setDirection(QAbstractAnimation::Forward);
    if(m_expand){
        m_expand->deleteLater();
        m_expand = nullptr;
    }
    if(m_collaps){
        m_collaps->deleteLater();
        m_collaps = nullptr;
    }

    if (m_widget) {
        this->setParent(nullptr);
        return true;
    }
    return false;
}

QVariant ConfigTreeAnimator::value(const QString &property)
{
    if (property == "expand") {
        return m_expand->currentValue();
    } else if (property == "collaps") {
        return m_collaps->currentValue();
    } else {
        return QVariant();
    }
}

bool ConfigTreeAnimator::setAnimatorStartValue(const QString &property, const QVariant &value)
{
    if (property == "expand") {
        m_expand->setStartValue(value);
        return true;
    } else if (property == "collaps")
    {
        m_collaps->setStartValue(value);
        return true;
    }  else {
        return false;
    }
}

bool ConfigTreeAnimator::setAnimatorEndValue(const QString &property, const QVariant &value)
{
    if (property == "expand") {
        m_expand->setEndValue(value);
        return true;
    } else if (property == "collaps") {
        m_collaps->setEndValue(value);
        return true;
    } else {
        return false;
    }
}

QVariant ConfigTreeAnimator::animatorStartValue(const QString &property)
{
    if (property == "expand") {
        return m_expand->startValue();
    } else if (property == "collaps") {
        return m_collaps->startValue();
    } else {
        return QVariant();
    }
}

QVariant ConfigTreeAnimator::animatorEndValue(const QString &property)
{
    if (property == "expand") {
        return m_expand->endValue();
    } else if (property == "collaps") {
        return m_collaps->endValue();
    } else {
        return QVariant();
    }
}


bool ConfigTreeAnimator::setAnimatorDuration(const QString &property, int duration)
{
    if (property == "expand") {
        m_expand->setDuration(duration);
        return true;
    } else if (property == "collaps") {
        m_collaps->setDuration(duration);
        return true;
    } else {
        return false;
    }
}

void ConfigTreeAnimator::setAnimatorDirectionForward(const QString &property, bool forward)
{
    auto d = forward? QAbstractAnimation::Forward: QAbstractAnimation::Backward;
    if (property == "expand") {
        m_expand->setDirection(d);
    } else if (property == "collaps") {
        m_collaps->setDirection(d);
    } else {
        return;
    }
}

bool ConfigTreeAnimator::isRunning(const QString &property)
{
    if (property == "expand") {
        return m_expand->state() == Running;
    } else if (property == "collaps") {
        return m_collaps->state() == Running;
    } else {
        return this->state() == Running;
    }
}

void ConfigTreeAnimator::startAnimator(const QString &property)
{
    if (property == "expand") {
        m_expand->start();
    } else if (property == "collaps") {
        m_collaps->start();
    } else {
        this->start();
    }
}

void ConfigTreeAnimator::stopAnimator(const QString &property)
{
    if (property == "expand") {
        m_expand->stop();
    } else if (property == "collaps") {
        m_collaps->stop();
    } else {
        this->stop();
    }
}

int ConfigTreeAnimator::currentAnimatorTime(const QString &property)
{
    if (property == "expand") {
        return m_expand->currentTime();
    } else if (property == "collaps") {
        return m_collaps->currentTime();
    } else {
        return this->currentTime();
    }
}

void ConfigTreeAnimator::setAnimatorCurrentTime(const QString &property, const int msecs)
{
    if (property == "expand") {
        return m_expand->setCurrentTime(msecs);
    } else if (property == "collaps") {
        return m_collaps->setCurrentTime(msecs);
    }
}


int ConfigTreeAnimator::totalAnimationDuration(const QString &property)
{
    if (property == "expand") {
        return m_expand->duration();
    } else if (property == "collaps") {
        return m_collaps->duration();
    } else {
        return this->duration();
    }
}

void ConfigTreeAnimator::setExtraProperty(const QString &property, const QVariant &value)
{
    if(property == "expandItem")
        m_expandIndex = value.toModelIndex();
    else if(property == "collapsItem")
        m_collapsIndex = value.toModelIndex();
}

QVariant ConfigTreeAnimator::getExtraProperty(const QString &property)
{
    if(property == "expandItem")
        return m_expandIndex;
    else if(property == "collapsItem")
        return m_collapsIndex;
    return QVariant();
}

