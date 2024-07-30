/*  -*- C++ -*-
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Tim D. Gilman <tdgilman@best.org>
    SPDX-FileCopyrightText: 1998-2001 Mirko Boehm <mirko@kde.org>
    SPDX-FileCopyrightText: 2007 John Layt <john@layt.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDATEVALIDATOR_H
#define KDATEVALIDATOR_H

#include <kguiaddons_export.h>

#include <QValidator>

#include <memory>

class KDateValidatorPrivate;

/**
 * @class KDateValidator kdatevalidator.h KDateValidator
 *
 * Validates user-entered dates.
 */
class KGUIADDONS_EXPORT KDateValidator : public QValidator
{
    Q_OBJECT
public:
    explicit KDateValidator(QObject *parent = nullptr);
    ~KDateValidator() override;

public:
    State validate(QString &text, int &e) const override;
    void fixup(QString &input) const override;
    State date(const QString &text, QDate &date) const;

private:
    std::unique_ptr<KDateValidatorPrivate> const d;
};

#endif // KDATEVALIDATOR_H
