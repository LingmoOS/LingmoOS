/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef TEST11_TYPES_H
#define TEST11_TYPES_H

class TimePeriod
{
public:
    enum Units {
        Minutes,
        HoursMinutes,
        Days,
        Weeks,
    };
};

#endif
