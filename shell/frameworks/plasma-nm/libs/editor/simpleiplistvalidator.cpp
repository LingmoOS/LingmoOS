/*
    SPDX-FileCopyrightText: 2018 Bruce Anderson <banderson19com@san.rr.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "simpleiplistvalidator.h"

#include <QList>
#include <QStringList>

SimpleIpListValidator::SimpleIpListValidator(AddressStyle style, AddressType type, QObject *parent)
    : QValidator(parent)
{
    if (type == Ipv4 || type == Both) {
        SimpleIpV4AddressValidator::AddressStyle ipv4Style;
        if (style == Base)
            ipv4Style = SimpleIpV4AddressValidator::AddressStyle::Base;
        else if (style == WithCidr)
            ipv4Style = SimpleIpV4AddressValidator::AddressStyle::WithCidr;
        else
            ipv4Style = SimpleIpV4AddressValidator::AddressStyle::WithPort;
        m_ipv4Validator = new SimpleIpV4AddressValidator(ipv4Style, this);
    }
    if (type == Ipv6 || type == Both) {
        SimpleIpV6AddressValidator::AddressStyle ipv6Style;
        if (style == Base)
            ipv6Style = SimpleIpV6AddressValidator::AddressStyle::Base;
        else if (style == WithCidr)
            ipv6Style = SimpleIpV6AddressValidator::AddressStyle::WithCidr;
        else
            ipv6Style = SimpleIpV6AddressValidator::AddressStyle::WithPort;
        m_ipv6Validator = new SimpleIpV6AddressValidator(ipv6Style, this);
    }
}

SimpleIpListValidator::~SimpleIpListValidator() = default;

QValidator::State SimpleIpListValidator::validate(QString &address, int &pos) const
{
    Q_UNUSED(pos)

    // Split the incoming address on commas possibly with spaces on either side
    QStringList addressList = address.split(QStringLiteral(","));

    // Use a local variable for position in the validators so it doesn't screw
    // up the position of the cursor when we return
    int localPos = 0;
    QValidator::State result = QValidator::Acceptable;

    for (QString &rawAddr : addressList) {
        QValidator::State ipv4Result = QValidator::Acceptable;
        QValidator::State ipv6Result = QValidator::Acceptable;

        QString addr = rawAddr.trimmed();

        // If we are starting a new address and all the previous addresses
        // are not Acceptable then the previous addresses need to be completed
        // before a new one is started
        if (result != QValidator::Acceptable)
            return QValidator::Invalid;

        // See if it is an IPv4 address. If we are not testing for IPv4
        // then by definition IPv4 is Invalid
        if (m_ipv4Validator != nullptr)
            ipv4Result = m_ipv4Validator->validate(addr, localPos);
        else
            ipv4Result = QValidator::Invalid;

        // See if it is an IPv6 address. If we are not testing for IPv6
        // then by definition IPv6 is Invalid
        if (m_ipv6Validator != nullptr)
            ipv6Result = m_ipv6Validator->validate(addr, localPos);
        else
            ipv6Result = QValidator::Invalid;

        // If this address is not at least an Intermediate then get out because the list is Invalid
        if (ipv6Result == QValidator::Invalid && ipv4Result == QValidator::Invalid)
            return QValidator::Invalid;

        // If either validator judged this address to be Intermediate then that's the best the
        // final result can be for the whole list. No need to test for Acceptable because
        // that's the default set on entry and we only downgrade it from there.
        if (ipv4Result == QValidator::Intermediate || ipv6Result == QValidator::Intermediate)
            result = QValidator::Intermediate;
    }
    return result;
}

#include "moc_simpleiplistvalidator.cpp"
