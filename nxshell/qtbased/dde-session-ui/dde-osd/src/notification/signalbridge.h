// SPDX-FileCopyrightText: 2016 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIGNALBRIDGE_H
#define SIGNALBRIDGE_H

#include <QObject>
#include <DSingleton>

class SignalBridge : public QObject, public Dtk::Core::DSingleton<SignalBridge>
{
    Q_OBJECT
    friend class Dtk::Core::DSingleton<SignalBridge>;
private:
    explicit SignalBridge(QObject *parent = nullptr) : QObject(parent) {}

signals:
    /**
     * @brief actionInvoked 提醒action已经执行
     */
    void actionInvoked(uint, const QString &);
};

#endif // SIGNALBRIDGE_H
