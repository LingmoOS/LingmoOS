/*
    SPDX-FileCopyrightText: 2018 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SIMPLEIPLISTVALIDATOR_H
#define SIMPLEIPLISTVALIDATOR_H

#include "plasmanm_editor_export.h"

#include "simpleipv4addressvalidator.h"
#include "simpleipv6addressvalidator.h"
#include <QValidator>

class PLASMANM_EDITOR_EXPORT SimpleIpListValidator : public QValidator
{
    Q_OBJECT
public:
    enum AddressType { Ipv4, Ipv6, Both };
    enum AddressStyle { Base, WithCidr, WithPort };

    explicit SimpleIpListValidator(AddressStyle style = AddressStyle::Base, AddressType allow = AddressType::Both, QObject *parent = nullptr);
    ~SimpleIpListValidator() override;

    State validate(QString &, int &) const override;

private:
    SimpleIpV6AddressValidator *m_ipv6Validator = nullptr;
    SimpleIpV4AddressValidator *m_ipv4Validator = nullptr;
};

#endif // SIMPLEIPV4ADDRESSVALIDATOR_H
