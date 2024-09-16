// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAPPLICATIONEVENTMONITOR_H
#define DAPPLICATIONEVENTMONITOR_H

#include "global.h"

#include <QObject>
#include <QEvent>

DPP_BEGIN_NAMESPACE

class DApplicationEventMonitor : public QObject
{
    Q_OBJECT

public:
    enum InputDeviceType {
        None = 0, // 初始状态
        Mouse = 1, //鼠标
        Tablet = 2, //平板外围设备
        Keyboard = 3, //键盘
        TouchScreen = 4 //触摸屏
    };

    DApplicationEventMonitor(QObject *parent = nullptr);
    ~DApplicationEventMonitor() override;

    InputDeviceType lastInputDeviceType() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    static InputDeviceType eventType(QEvent *type);

signals:
    void lastInputDeviceTypeChanged();

private:
    InputDeviceType m_lastInputDeviceType;
};

DPP_END_NAMESPACE

#endif // DAPPLICATIONEVENTMONITOR_H
