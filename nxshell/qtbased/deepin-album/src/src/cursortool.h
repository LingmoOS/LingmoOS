// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CURSORTOOL_H
#define CURSORTOOL_H

#include <QObject>
#include <QPoint>
#include <QColor>

class QTimer;

/**
 * @brief 用于主动捕获光标坐标变更时间
 *
 * @note 由于QML中MouseArea无法穿透Hover事件，且上层Item过多，
 *      使用定时查询获取光标位置。
 */
class CursorTool : public QObject
{
    Q_OBJECT
public:
    explicit CursorTool(QObject *parent = nullptr);

    // 取得当前鼠标光标位置
    Q_INVOKABLE QPoint currentCursorPos() const;
    // 设置启动/停止采样
    Q_INVOKABLE void setCaptureCursor(bool b);
    // 当前光标位置信号
    Q_SIGNAL void cursorPosChanged(int x, int y);

    // 获取光标初始活动色
    Q_INVOKABLE QColor activeColor();
    // 光标框选活动色改变
    Q_SIGNAL void activeColorChanged(const QColor &);

private:
    QTimer *m_CaptureTimer;  // 采样捕获定时器
    QPoint m_lastPos;        // 记录最后的位置
};

#endif  // CURSORTOOL_H
