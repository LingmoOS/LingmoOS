/*
    SPDX-FileCopyrightText: 2009 George Kiagiadakis <gkiagia@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DETACHEDPROCESSMONITOR_H
#define DETACHEDPROCESSMONITOR_H

#include <QObject>

class DetachedProcessMonitor : public QObject
{
    Q_OBJECT
public:
    explicit DetachedProcessMonitor(QObject *parent = nullptr);
    void startMonitoring(int pid);

Q_SIGNALS:
    void processFinished();

protected:
    void timerEvent(QTimerEvent *) override;

private:
    int m_pid;
};

#endif
