/*
    SPDX-FileCopyrightText: 2018 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef WIREGUARDKEYVALIDATOR_H
#define WIREGUARDKEYVALIDATOR_H

#include <QValidator>

class WireGuardKeyValidator : public QValidator
{
    Q_OBJECT
public:
    explicit WireGuardKeyValidator(QObject *parent = nullptr);
    ~WireGuardKeyValidator() override;

    QValidator::State validate(QString &, int &) const override;

private:
    QRegularExpressionValidator *const m_validator;
};

#endif // SIMPLEIPV4ADDRESSVALIDATOR_H
