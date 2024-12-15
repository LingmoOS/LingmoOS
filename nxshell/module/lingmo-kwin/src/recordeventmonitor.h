// Copyright (C) 2022 Uniontech Technology Co., Ltd.
// Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef RECORDEVENTMONITOR_H
#define RECORDEVENTMONITOR_H

#include <QObject>
#include <QThread>

class RecordEventMonitor : public QThread
{
    Q_OBJECT
public:
    explicit RecordEventMonitor(QObject *parent = nullptr);
    void stopRecord();

Q_SIGNALS:
    void buttonRelease();
    void motion();
    void touchDown(double x, double y, unsigned int time);
    void touchMotion(double x, double y, unsigned int time);
    void touchUp(unsigned int time);

public Q_SLOTS:

protected:
    void run();
};

#endif // RECORDEVENTMONITOR_H
