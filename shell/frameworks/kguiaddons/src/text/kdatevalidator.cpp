/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Tim D. Gilman <tdgilman@best.org>
    SPDX-FileCopyrightText: 1998-2001 Mirko Boehm <mirko@kde.org>
    SPDX-FileCopyrightText: 2007 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kdatevalidator.h"

#include <QDate>
#include <QLocale>

class KDateValidatorPrivate
{
public:
    KDateValidatorPrivate(KDateValidator *qq)
        : q(qq)
    {
    }

    ~KDateValidatorPrivate()
    {
    }

    KDateValidator *const q;
};

KDateValidator::KDateValidator(QObject *parent)
    : QValidator(parent)
{
}

KDateValidator::~KDateValidator() = default;

QValidator::State KDateValidator::validate(QString &text, int &unused) const
{
    Q_UNUSED(unused);

    QDate temp;
    // ----- everything is tested in date():
    return date(text, temp);
}

QValidator::State KDateValidator::date(const QString &text, QDate &d) const
{
    QLocale::FormatType formats[] = {QLocale::LongFormat, QLocale::ShortFormat, QLocale::NarrowFormat};
    QLocale locale;

    for (int i = 0; i < 3; i++) {
        QDate tmp = locale.toDate(text, formats[i]);
        if (tmp.isValid()) {
            d = tmp;
            return Acceptable;
        }
    }

    return QValidator::Intermediate;
}

void KDateValidator::fixup(QString &) const
{
}

#include "moc_kdatevalidator.cpp"
