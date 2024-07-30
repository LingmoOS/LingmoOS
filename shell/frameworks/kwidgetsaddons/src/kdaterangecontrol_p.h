/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDATERANGECONTROL_P_H
#define KDATERANGECONTROL_P_H

#include <QDate>

/** Common code for widgets supporting input of dates within a specified min/max range. */
class KDateRangeControlPrivate
{
public:
    bool setDateRange(const QDate &minDate, const QDate &maxDate);
    bool isInDateRange(const QDate &date) const;

    QDate m_minDate;
    QDate m_maxDate;
};

#endif
