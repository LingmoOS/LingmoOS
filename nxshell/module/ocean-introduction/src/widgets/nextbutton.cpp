// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nextbutton.h"
#include <QDebug>

#define DEFAULT_BG_COLOR  255, 255, 255, int(1.0 * 255)
#define HOVER_BG_COLOR    255, 255, 255, int(0.3 * 255)
#define PRESS_BG_COLOR    255, 255, 255, int(0.1 * 255)

NextButton::NextButton(const QString &text, QWidget *parent)
    : DPushButton(parent)
    , m_text(text)
    , m_mode(Normal)
    , m_currentColor(QColor(DEFAULT_BG_COLOR))
{
    setText(text);
//    setFocusPolicy(Qt::NoFocus);
    update();
}

void NextButton::setMode(NextButton::Mode mode)
{
    m_mode = mode;

    update();
}

void NextButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);

    m_currentColor = QColor(HOVER_BG_COLOR);

    update();
}

void NextButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);

    m_currentColor = QColor(DEFAULT_BG_COLOR);

    update();
}

void NextButton::mousePressEvent(QMouseEvent *event)
{
    QPushButton::mousePressEvent(event);

    m_currentColor = QColor(PRESS_BG_COLOR);

    update();
}

void NextButton::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left ||
        event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Up ||
        event->key() == Qt::Key_Down)
        return;

    if (event->key() == Qt::Key_Return){
        this->click();
    }
}
