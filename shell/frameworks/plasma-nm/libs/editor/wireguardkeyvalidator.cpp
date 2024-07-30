/*
    SPDX-FileCopyrightText: 2018 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "wireguardkeyvalidator.h"

WireGuardKeyValidator::WireGuardKeyValidator(QObject *parent)
    : QValidator(parent)
    , m_validator(new QRegularExpressionValidator(this))
{
    // A WireGuard key is Base64 encoded and in human readable form consists
    // of 43 Alpha-numeric or  '+' or '/' with a 44th character of an equal sign.
    // The 43rd character is limited such that the converted character zeroes in
    // the 2 LSB.
    m_validator->setRegularExpression(QRegularExpression(QStringLiteral("[0-9a-zA-Z\\+/]{42,42}[AEIMQUYcgkosw048]=")));
}

WireGuardKeyValidator::~WireGuardKeyValidator() = default;

QValidator::State WireGuardKeyValidator::validate(QString &address, int &pos) const
{
    return m_validator->validate(address, pos);
}

#include "moc_wireguardkeyvalidator.cpp"
