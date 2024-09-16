// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef REFRESHTIMER_H
#define REFRESHTIMER_H
#include <QObject>
#include <QTimer>
#include <QDateTime>

/*!
 * \~chinese \class RefreshTimer
 * \~chinese \brief 每900ms刷新一次timeLabel上的时间信息
 */
class RefreshTimer : public QObject
{
    Q_OBJECT
public :
    static RefreshTimer *instance()
    {
        static RefreshTimer *timer = new RefreshTimer;
        return timer;
    }
private:
    RefreshTimer()
    {
        start();
    }

    void start()
    {
        m_timer.start(900);
        connect(&m_timer, &QTimer::timeout, [ = ] {
            if (QDateTime::currentDateTime().toString("hhmmss") == "000000") {
                Q_EMIT forceRefresh();
            }
        });
    }

Q_SIGNALS:
    void forceRefresh();

private:
    QTimer m_timer;
};

#endif // REFRESHTIMER_H
