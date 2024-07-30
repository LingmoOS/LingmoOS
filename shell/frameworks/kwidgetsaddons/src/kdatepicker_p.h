/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Tim D . Gilman <tdgilman@best.org>
    SPDX-FileCopyrightText: 1998-2001 Mirko Boehm <mirko@kde.org>
    SPDX-FileCopyrightText: 2007 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDATEPICKER_P_H
#define KDATEPICKER_P_H

#include <QDate>
#include <QIntValidator>
#include <QLineEdit>

/**
 * @internal
 * Year selection widget.
 * @author Tim Gilman, Mirko Boehm, John Layt
 */
class KDatePickerPrivateYearSelector : public QLineEdit
{
    Q_OBJECT

public:
    explicit KDatePickerPrivateYearSelector(const QDate &currentDate, QWidget *parent = nullptr);
    int year();
    void setYear(const QDate &year);

public Q_SLOTS:
    void yearEnteredSlot();

Q_SIGNALS:
    void closeMe(int);

protected:
    QIntValidator *val;
    int result;

private:
    QDate oldDate;

    Q_DISABLE_COPY(KDatePickerPrivateYearSelector)
};

#endif // KDATEPICKER_P_H
