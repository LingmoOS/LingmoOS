// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sharedeventfilter.h"

#include <QProcess>
#include <QDebug>

/*! \class SharedEventFilter
 *
 * FullscreenFrame and MiniFrame shared event handler.
 *
 */

SharedEventFilter::SharedEventFilter(QObject *parent)
    :QObject(parent)
{
    m_frame = dynamic_cast<LauncherInterface*>(parent);

    Q_ASSERT_X(m_frame, "SharedEventFilter",
               "SharedEventFilter is supposed used together only with LauncherInterface instances.");
}

bool SharedEventFilter::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);

    if (event->type() == QEvent::KeyPress)
        return handleKeyEvent(static_cast<QKeyEvent *>(event));

    return false;
}

/**
 * @brief SharedEventFilter::handleKeyEvent 处理系统键盘事件
 * @param e 键盘事件指针对象
 * @return 返回true,阻断事件传播,返回false继续向子类对象传递
 */
bool SharedEventFilter::handleKeyEvent(QKeyEvent *e)
{
    bool ctrlPressed = e->modifiers() & Qt::ControlModifier;
    switch (e->key())
    {
    case Qt::Key_F1:            QProcess::startDetached("dman", { "dde" });     return true;
    case Qt::Key_Enter:
    case Qt::Key_Return:        m_frame->launchCurrentApp();                    return true;
    case Qt::Key_Escape:        m_frame->hideLauncher();                        return true;
    case Qt::Key_Space:         m_frame->moveCurrentSelectApp(e->key());        return true;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
    case Qt::Key_Up:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:         m_frame->moveCurrentSelectApp(e->key());        return true;
    case Qt::Key_Backspace:     m_frame->appendToSearchEdit(-1);                return true;
    }

    // handle normal keys
    if ((e->key() <= Qt::Key_Z && e->key() >= Qt::Key_A) ||
        (e->key() <= Qt::Key_9 && e->key() >= Qt::Key_0) || e->key() == Qt::Key_Space)
    {
        // handle the emacs key bindings
        if(ctrlPressed) {
            switch (e->key()) {
            case Qt::Key_P:
                m_frame->moveCurrentSelectApp(Qt::Key_Up);
                return true;
            case Qt::Key_N:
                m_frame->moveCurrentSelectApp(Qt::Key_Down);
                return true;
            case Qt::Key_F:
                m_frame->moveCurrentSelectApp(Qt::Key_Right);
                return true;
            case Qt::Key_B:
                m_frame->moveCurrentSelectApp(Qt::Key_Left);
                return true;
            case Qt::Key_Z:
                m_frame->moveCurrentSelectApp(Qt::Key_Undo);
                return true;
            default:
                return false;
            }
        }

        m_frame->appendToSearchEdit(e->text()[0].toLatin1());

        return true;
    }

    return false;
}


