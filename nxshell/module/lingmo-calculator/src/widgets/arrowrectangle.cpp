// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "arrowrectangle.h"
#include "../views/memorylistwidget.h"

#include <QDebug>
#include <DListWidget>


//static int init = 0;

ArrowRectangle::ArrowRectangle(ArrowDirection direction, FloatMode floatMode, QWidget *parent)
    : DArrowRectangle(direction, floatMode, parent)
{
    this->installEventFilter(this);
    setMouseTracking(true);
}

ArrowRectangle::~ArrowRectangle()
{

}

bool ArrowRectangle::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusOut) {
        //点击标题栏及外部桌面失去焦点时切换至scientifickeypad
        QFocusEvent *focus_Event = static_cast<QFocusEvent *>(event);
        if (focus_Event->reason() == Qt::MouseFocusReason || focus_Event->reason() == Qt::ActiveWindowFocusReason) {
            emit hidearrowrectangle();
        }
    }
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key_event = static_cast < QKeyEvent *>(event); //将事件转化为键盘事件
        if (key_event->key() == Qt::Key_Escape) {
            emit hidearrowrectangle(true);
            return true;
        } else if (key_event->key() == Qt::Key_Tab && getContent()) {
            if (obj == this) {
                static_cast<MemoryListWidget *>(getContent())->cleanState();
                static_cast<MemoryListWidget *>(getContent())->setFocus(Qt::TabFocusReason);
                return true;
            } else {
                this->setFocus(Qt::TabFocusReason);
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ArrowRectangle::focusInEvent(QFocusEvent *event)
{
    if (event->reason() == Qt::TabFocusReason) {
        static_cast<MemoryListWidget *>(getContent())->cleanState();
        static_cast<MemoryListWidget *>(getContent())->setFocus();
    }
    DArrowRectangle::focusInEvent(event);
}

/**
 * @brief ArrowRectangle::keyPressEvent
 * 防止按其他键时焦点移到输入栏
 */
void ArrowRectangle::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
    return;
}

void ArrowRectangle::mouseMoveToClearFocus()
{
    this->setFocus();
}
