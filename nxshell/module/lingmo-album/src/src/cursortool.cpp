// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cursortool.h"

#include <QTimer>
#include <QCursor>
#include <DGuiApplicationHelper>

static const int sc_ESampleInterval = 50;  // 采样间隔 50ms

CursorTool::CursorTool(QObject *parent)
    : QObject(parent)
{
    m_CaptureTimer = new QTimer(this);
    m_CaptureTimer->setInterval(sc_ESampleInterval);

    connect(m_CaptureTimer, &QTimer::timeout, this, [this]() {
        QPoint pos = QCursor::pos();
        if (pos != m_lastPos) {
            m_lastPos = pos;
            // 发送当前光标的全局位置
            Q_EMIT this->cursorPosChanged(pos.x(), pos.y());
        }
    });

    connect(Dtk::Gui::DGuiApplicationHelper::instance(), &Dtk::Gui::DGuiApplicationHelper::applicationPaletteChanged, [this]() {
        auto newColor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
        emit activeColorChanged(newColor);
    });
}

/**
   @return 返回当前鼠标光标在屏幕的位置
 */
QPoint CursorTool::currentCursorPos() const
{
    return QCursor::pos();
}

/**
 * @param b 设置是否启动定时查询光标位置
 *  默认间隔20ms
 */
void CursorTool::setCaptureCursor(bool b)
{
    if (b) {
        m_CaptureTimer->start();
    } else {
        m_CaptureTimer->stop();
    }
}

QColor CursorTool::activeColor()
{
    return Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color();
}
