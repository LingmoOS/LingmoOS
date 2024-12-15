// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logiconbutton.h"

#include <QEvent>
#include <QResizeEvent>
#include <QDebug>
#define ICON_SIZE 32

LogIconButton::LogIconButton(QWidget *parent)
    : QPushButton(parent)
{
    this->setFlat(true);
    this->setFocusPolicy(Qt::NoFocus);
    this->setIconSize(QSize(32, 32));
}

LogIconButton::LogIconButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    this->setFlat(true);
    this->setFocusPolicy(Qt::NoFocus);
    this->setIconSize(QSize(32, 32));
}
//修复button占高过大的问题
/**
 * @brief LogIconButton::sizeHint
 *  修复iconbutton因为图表过大把按钮撑大的问题,根据自体计算按钮高度
 * @return
 */
QSize LogIconButton::sizeHint() const
{
    int h = QFontMetrics(font()).height();
    return QSize(QPushButton::sizeHint().width(), h);
}

void LogIconButton::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}

bool LogIconButton::event(QEvent *e)
{
    if (e->type() != QEvent::HoverEnter) {
        QPushButton::event(e);
    }
    return  false;
}




/**
 * @brief LogIconButton::resizeEvent
 * 修复iconbutton因为图表过大把按钮撑大的问题,根据自体计算按钮高度
 * @param e
 */
void LogIconButton::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e)
    int h = QFontMetrics(font()).height();
    resize(this->sizeHint().width(), h);

}
