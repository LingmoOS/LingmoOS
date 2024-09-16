// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDateTime>
#include <QFont>
#include <QPixmap>

namespace dwclock {
class Clock
{
public:
    explicit Clock();
    virtual ~Clock();

    void setUTCOffset(const int utcOffset);
    void setLocation(const QString &location);

    void paint(QPainter *painter, const QRect &rect);

    static int timeDiff(int utcOffset, const QDateTime &localTime = QDateTime());

private:
    void updateClockPixmap(const bool isNightMode);
    void updateLocationFont();
    QPixmap getPixmap(const QString &name, const QSize &size);
    QPixmap getPixmap(const QString &name, const int size, const bool isDark);

    bool m_isBlack = false;
    int m_utcOffset = 0;
    QPixmap m_plat;
    QPixmap m_hour;
    QPixmap m_min;
    QPixmap m_sec;
    QSize m_clockPlatSize;
    QSize m_clockPointSize;
    QString m_location;
    QFont m_locationTxtFont;
};
}
