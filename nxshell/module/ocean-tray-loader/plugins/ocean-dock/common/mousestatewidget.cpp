// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "mousestatewidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

bool MouseStateWidget::ignoreEnterEvent = false;

MouseStateWidget::MouseStateWidget(QWidget *parent)
        : QWidget(parent)
        , m_state(LEAVE)
        , m_ensureLeave(false)
        , m_enableIgnoreEnterEvent(false)
{
}

void MouseStateWidget::restState()
{
    m_state = LEAVE;
    update();
}

bool MouseStateWidget::event(QEvent *event)
{
    auto setState = [this] (int state) {
        m_state = state;
        update();
    };

    if (event->type() == QEvent::Enter) {
        if (!m_enableIgnoreEnterEvent || !ignoreEnterEvent)
            setState(HOVERD);
    } else if (event->type() == QEvent::Leave) {
        if (!m_ensureLeave || !rect().contains(mapFromGlobal(QCursor::pos())))
            setState(LEAVE);
    } else if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *e = dynamic_cast<QMouseEvent *>(event);
        if (e && e->button() == Qt::LeftButton)
            setState(PRESSED);
        else if (m_ensureLeave) // xembed 插件弹出右键菜单的时候，收到leave 事件，但是ensure leave会阻止设置 LEAVE 状态
            setState(LEAVE);
    } else if (event->type() == QEvent::MouseButtonRelease) {
        setState(rect().contains(mapFromGlobal(QCursor::pos())) ? HOVERD : LEAVE);
    }

    return QWidget::event(event);
}
