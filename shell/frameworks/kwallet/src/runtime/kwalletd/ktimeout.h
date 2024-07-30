/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2003 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef _KTIMEOUT_H_
#define _KTIMEOUT_H_

#include <QHash>
#include <QObject>

// @internal
class KTimeout : public QObject
{
    Q_OBJECT
public:
    explicit KTimeout(QObject *parent = nullptr);
    ~KTimeout() override;

Q_SIGNALS:
    void timedOut(int id);

public Q_SLOTS:
    void resetTimer(int id, int timeout);
    void addTimer(int id, int timeout);
    void removeTimer(int id);
    void clear();

protected:
    void timerEvent(QTimerEvent *) override;

private:
    QHash<int /*id*/, int /*timerId*/> _timers;
};

#endif
