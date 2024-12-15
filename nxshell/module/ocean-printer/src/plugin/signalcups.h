// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIGNALCUPS_H
#define SIGNALCUPS_H

#include <QDBusMessage>

class SignalCups : public QObject
{
    Q_OBJECT

public:
    SignalCups(QObject *parent = nullptr);
    ~SignalCups() override;
    bool initWatcher();

protected slots:
    void spoolerEvent(QDBusMessage);

};

#endif //SIGNALCUPS_H
