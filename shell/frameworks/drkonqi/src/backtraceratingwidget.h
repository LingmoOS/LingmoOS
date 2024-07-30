/*******************************************************************
 * backtraceratingwidget.h
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 ******************************************************************/

#ifndef BACKTRACERATINGWIDGET__H
#define BACKTRACERATINGWIDGET__H

#include <QWidget>

#include "backtracegenerator.h"
#include "parser/backtraceparser.h"

class QPixmap;

class BacktraceRatingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BacktraceRatingWidget(QWidget *);
    void setUsefulness(BacktraceParser::Usefulness);
    void setState(BacktraceGenerator::State s)
    {
        m_state = s;
        update();
    }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    BacktraceGenerator::State m_state;

    int m_numStars;
    QPixmap m_errorPixmap;

    QPixmap m_starPixmap;
    QPixmap m_disabledStarPixmap;
};

#endif
