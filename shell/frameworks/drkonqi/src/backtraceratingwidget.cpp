/*******************************************************************
 * backtraceratingwidget.cpp
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#include "backtraceratingwidget.h"

#include <QIcon>
#include <QPainter>

BacktraceRatingWidget::BacktraceRatingWidget(QWidget *parent)
    : QWidget(parent)
    , m_state(BacktraceGenerator::NotLoaded)
{
    setMinimumSize(105, 24);

    m_starPixmap = QIcon::fromTheme(QStringLiteral("rating")).pixmap(QSize(22, 22));
    m_disabledStarPixmap = QIcon::fromTheme(QStringLiteral("rating")).pixmap(QSize(22, 22), QIcon::Disabled);
    m_errorPixmap = QIcon::fromTheme(QStringLiteral("dialog-error")).pixmap(QSize(22, 22));
}

void BacktraceRatingWidget::setUsefulness(BacktraceParser::Usefulness usefulness)
{
    switch (usefulness) {
    case BacktraceParser::ReallyUseful:
        m_numStars = 3;
        break;
    case BacktraceParser::MayBeUseful:
        m_numStars = 2;
        break;
    case BacktraceParser::ProbablyUseless:
        m_numStars = 1;
        break;
    case BacktraceParser::Useless:
    case BacktraceParser::InvalidUsefulness:
        m_numStars = 0;
        break;
    }
    update();
}

void BacktraceRatingWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    p.drawPixmap(QPoint(30, 1), m_numStars >= 1 ? m_starPixmap : m_disabledStarPixmap);
    p.drawPixmap(QPoint(55, 1), m_numStars >= 2 ? m_starPixmap : m_disabledStarPixmap);
    p.drawPixmap(QPoint(80, 1), m_numStars >= 3 ? m_starPixmap : m_disabledStarPixmap);

    switch (m_state) {
    case BacktraceGenerator::Failed:
    case BacktraceGenerator::FailedToStart: {
        p.drawPixmap(QPoint(0, 1), m_errorPixmap);
        break;
    }
    default:
        break;
    }

    p.end();
}

#include "moc_backtraceratingwidget.cpp"
