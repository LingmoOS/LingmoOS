/*
    SPDX-FileCopyrightText: 2024 Stephen Robinson <stephen.robinson@eqware.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "interfacenamevalidator.h"

InterfaceNameValidator::InterfaceNameValidator(QObject *parent)
    : QValidator(parent)
{
}

InterfaceNameValidator::~InterfaceNameValidator() = default;

QValidator::State InterfaceNameValidator::validate(QString &name, int &pos) const
{
    Q_UNUSED(pos);

    if (name.length() > 15) {
        return QValidator::State::Invalid;
    }

    const QString invalidChars = QStringLiteral(" \n\t\r\f\v\240:/");
    for (QChar c : std::as_const(name)) {
        if (invalidChars.contains(c)) {
            return QValidator::State::Invalid;
        }
    }

    if (name.isEmpty() || name == "." || name == "..") {
        return QValidator::State::Intermediate;
    }

    return QValidator::State::Acceptable;
}

#include "moc_interfacenamevalidator.cpp"
