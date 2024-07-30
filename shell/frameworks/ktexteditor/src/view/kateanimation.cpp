/*
    SPDX-FileCopyrightText: 2013 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kateanimation.h"

#include "katefadeeffect.h"

#include <KMessageWidget>

#include <QStandardPaths>
#include <QStyle>
#include <QTimer>

KateAnimation::KateAnimation(KMessageWidget *widget, EffectType effect)
    : QObject(widget)
    , m_widget(widget)
    , m_fadeEffect(nullptr)
{
    Q_ASSERT(m_widget != nullptr);

    // create wanted effect
    if (effect == FadeEffect) {
        m_fadeEffect = new KateFadeEffect(widget);

        connect(m_fadeEffect, &KateFadeEffect::hideAnimationFinished, this, &KateAnimation::widgetHidden);
        connect(m_fadeEffect, &KateFadeEffect::showAnimationFinished, this, &KateAnimation::widgetShown);
    } else {
        connect(m_widget.data(), &KMessageWidget::hideAnimationFinished, this, &KateAnimation::widgetHidden);
        connect(m_widget.data(), &KMessageWidget::showAnimationFinished, this, &KateAnimation::widgetShown);
    }
}

bool KateAnimation::isHideAnimationRunning() const
{
    return m_fadeEffect ? m_fadeEffect->isHideAnimationRunning() : m_widget->isHideAnimationRunning();
}

bool KateAnimation::isShowAnimationRunning() const
{
    return m_fadeEffect ? m_fadeEffect->isShowAnimationRunning() : m_widget->isShowAnimationRunning();
}

void KateAnimation::show()
{
    Q_ASSERT(m_widget != nullptr);

    // show according to effects config
    if (m_widget->style()->styleHint(QStyle::SH_Widget_Animate, nullptr, m_widget)) {
        // launch show effect
        // NOTE: use a singleShot timer to avoid resizing issues when showing the message widget the first time (bug #316666)
        if (m_fadeEffect) {
            QTimer::singleShot(0, m_fadeEffect, &KateFadeEffect::fadeIn);
        } else {
            QTimer::singleShot(0, m_widget.data(), &KMessageWidget::animatedShow);
        }
    } else {
        m_widget->show();
        Q_EMIT widgetShown();
    }
}

void KateAnimation::hide()
{
    Q_ASSERT(m_widget != nullptr);

    // hide according to effects config
    if (m_widget->style()->styleHint(QStyle::SH_Widget_Animate, nullptr, m_widget)
        || QStandardPaths::isTestModeEnabled() // due to timing issues in the unit test
    ) {
        // hide depending on effect
        if (m_fadeEffect) {
            m_fadeEffect->fadeOut();
        } else {
            m_widget->animatedHide();
        }
    } else {
        m_widget->hide();
        Q_EMIT widgetHidden();
    }
}

#include "moc_kateanimation.cpp"
