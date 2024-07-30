/*
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kemailvalidator.h"

#include <KEmailAddress>

KEmailValidator::KEmailValidator(QObject *parent)
    : QValidator(parent)
{
}

KEmailValidator::~KEmailValidator() = default;

QValidator::State KEmailValidator::validate(QString &str, int &pos) const
{
    Q_UNUSED(pos)

    if (KEmailAddress::isValidSimpleAddress(str)) {
        return QValidator::Acceptable;
    }
    const auto containsSpace = std::any_of(str.begin(), str.end(), [](QChar c) {
        return c.isSpace();
    });
    if (containsSpace) {
        return QValidator::Invalid;
    }
    return QValidator::Intermediate;
}

void KEmailValidator::fixup(QString &str) const
{
    str = str.trimmed();
}

#include "moc_kemailvalidator.cpp"
