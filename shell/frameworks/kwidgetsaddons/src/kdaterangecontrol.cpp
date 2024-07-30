/*
    SPDX-FileCopyrightText: 2011 John Layt <john@layt.net>
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdaterangecontrol_p.h"

bool KDateRangeControlPrivate::isInDateRange(const QDate &date) const
{
    return date.isValid() //
        && (!m_minDate.isValid() || date >= m_minDate) //
        && (!m_maxDate.isValid() || date <= m_maxDate);
}

bool KDateRangeControlPrivate::setDateRange(const QDate &minDate, const QDate &maxDate)
{
    if (minDate.isValid() && maxDate.isValid() && minDate > maxDate) {
        return false;
    }

    m_minDate = minDate;
    m_maxDate = maxDate;
    return true;
}
