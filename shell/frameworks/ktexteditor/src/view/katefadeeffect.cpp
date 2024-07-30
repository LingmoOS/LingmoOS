/*
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katefadeeffect.h"

#include <QGraphicsOpacityEffect>
#include <QTimeLine>
#include <QWidget>

KateFadeEffect::KateFadeEffect(QWidget *widget)
    : QObject(widget)
    , m_widget(widget)
    , m_effect(nullptr) // effect only exists during fading animation
{
    m_timeLine = new QTimeLine(500, this);
    m_timeLine->setUpdateInterval(40);

    connect(m_timeLine, &QTimeLine::valueChanged, this, &KateFadeEffect::opacityChanged);
    connect(m_timeLine, &QTimeLine::finished, this, &KateFadeEffect::animationFinished);
}

bool KateFadeEffect::isHideAnimationRunning() const
{
    return (m_timeLine->direction() == QTimeLine::Backward) && (m_timeLine->state() == QTimeLine::Running);
}

bool KateFadeEffect::isShowAnimationRunning() const
{
    return (m_timeLine->direction() == QTimeLine::Forward) && (m_timeLine->state() == QTimeLine::Running);
}

void KateFadeEffect::fadeIn()
{
    // stop time line if still running
    if (m_timeLine->state() == QTimeLine::Running) {
        QTimeLine::Direction direction = m_timeLine->direction();
        m_timeLine->stop();
        if (direction == QTimeLine::Backward) {
            // fadeOut animation interrupted
            Q_EMIT hideAnimationFinished();
        }
    }

    // assign new graphics effect, old one is deleted in setGraphicsEffect()
    m_effect = new QGraphicsOpacityEffect(this);
    m_effect->setOpacity(0.0);
    m_widget->setGraphicsEffect(m_effect);

    // show widget and start fade in animation
    m_widget->show();
    m_timeLine->setDirection(QTimeLine::Forward);
    m_timeLine->start();
}

void KateFadeEffect::fadeOut()
{
    // stop time line if still running
    if (m_timeLine->state() == QTimeLine::Running) {
        QTimeLine::Direction direction = m_timeLine->direction();
        m_timeLine->stop();
        if (direction == QTimeLine::Forward) {
            // fadeIn animation interrupted
            Q_EMIT showAnimationFinished();
        }
    }

    // assign new graphics effect, old one is deleted in setGraphicsEffect()
    m_effect = new QGraphicsOpacityEffect(this);
    m_effect->setOpacity(1.0);
    m_widget->setGraphicsEffect(m_effect);

    // start fade out animation
    m_timeLine->setDirection(QTimeLine::Backward);
    m_timeLine->start();
}

void KateFadeEffect::opacityChanged(qreal value)
{
    Q_ASSERT(m_effect);
    m_effect->setOpacity(value);
}

void KateFadeEffect::animationFinished()
{
    // fading finished: remove graphics effect, deletes the effect as well
    m_widget->setGraphicsEffect(nullptr);
    Q_ASSERT(!m_effect);

    if (m_timeLine->direction() == QTimeLine::Backward) {
        m_widget->hide();
        Q_EMIT hideAnimationFinished();
    } else {
        Q_EMIT showAnimationFinished();
    }
}

#include "moc_katefadeeffect.cpp"
