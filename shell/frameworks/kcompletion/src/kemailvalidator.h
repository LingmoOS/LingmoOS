/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KEMAILVALIDATOR_H
#define KEMAILVALIDATOR_H

#include "kcompletion_export.h"

#include <QValidator>

/**
  An input validator that checks for valid email addresses.
  @see KEmailAddress::isValidSimpleAddress
  @since 6.0
*/
class KCOMPLETION_EXPORT KEmailValidator : public QValidator
{
    Q_OBJECT
public:
    explicit KEmailValidator(QObject *parent = nullptr);
    ~KEmailValidator();

    State validate(QString &str, int &pos) const override;
    void fixup(QString &str) const override;
};

#endif
