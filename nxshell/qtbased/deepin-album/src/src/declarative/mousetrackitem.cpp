// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mousetrackitem.h"

/**
   @brief 鼠标事件跟踪处理，用于处理 MouseArea 捕获 press 事件后无法向下传递的问题
 */
MouseTrackItem::MouseTrackItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFiltersChildMouseEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
}

/**
   @brief 设置当前是否处于 \a press 状态
 */
void MouseTrackItem::setPressed(bool press)
{
    if (isPressed != press) {
        isPressed = press;
        Q_EMIT pressedChanged();
    }
}

/**
   @return 返回当前是否处于点击状态
 */
bool MouseTrackItem::pressed() const
{
    return isPressed;
}

/**
   @brief 捕获鼠标点击事件 \a event
 */
void MouseTrackItem::mousePressEvent(QMouseEvent *event)
{
    setPressed(true);
    event->accept();
}

/**
   @brief 捕获鼠标释放事件 \a event
 */
void MouseTrackItem::mouseReleaseEvent(QMouseEvent *event)
{
    setPressed(false);
    event->accept();
}

/**
   @brief 捕获鼠标双击事件 \a event
 */
void MouseTrackItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_EMIT doubleClicked();
    event->accept();
}
