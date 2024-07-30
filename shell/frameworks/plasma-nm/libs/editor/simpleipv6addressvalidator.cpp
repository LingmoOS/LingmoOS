/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>, based on work by Paul Marchouk <pmarchouk@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "simpleipv6addressvalidator.h"

#include <QList>
#include <QStringList>

SimpleIpV6AddressValidator::SimpleIpV6AddressValidator(AddressStyle style, QObject *parent)
    : QValidator(parent)
    , m_addressStyle(style)
{
    switch (style) {
    case Base:
        m_validator.setRegularExpression(QRegularExpression(QStringLiteral("([0-9a-fA-F]{1,4}|:)+")));
        break;
    case WithCidr:
        m_validator.setRegularExpression(QRegularExpression(QStringLiteral("([0-9a-fA-F]{1,4}|:){2,15}/[0-9]{1,3}")));
        break;
    case WithPort:
        m_validator.setRegularExpression(QRegularExpression(QStringLiteral("\\[([0-9a-fA-F]{1,4}|:)+\\]:[0-9]{1,5}")));
    }
}

SimpleIpV6AddressValidator::~SimpleIpV6AddressValidator() = default;

QValidator::State SimpleIpV6AddressValidator::validate(QString &address, int &pos) const
{
    if (QValidator::Invalid == checkWithInputMask(address, pos)) {
        return QValidator::Invalid;
    }

    return checkTetradsRanges(address);
}

QValidator::State SimpleIpV6AddressValidator::checkWithInputMask(QString &value, int &pos) const
{
    return m_validator.validate(value, pos);
}

QValidator::State SimpleIpV6AddressValidator::checkTetradsRanges(QString &value) const
{
    QStringList addrParts;
    QStringList cidrParts;
    QStringList portParts;
    bool foundBracket = false;
    QValidator::State result = QValidator::Acceptable;

    switch (m_addressStyle) {
    case Base:
        addrParts = value.split(QLatin1Char(':'));
        break;

    case WithCidr:
        cidrParts = value.split(QLatin1Char('/'));
        addrParts = cidrParts[0].split(QLatin1Char(':'));
        break;

    case WithPort:
        if (value.isEmpty())
            return QValidator::Intermediate;
        if (value[0] != '[') {
            return QValidator::Invalid;
        } else {
            // Input: "[1:2:3:4:5:6:7:8]:123"
            // bracketParts: "[1:2:3:4:5:6:7:8" , ":123"
            // addrParts: "" , "1:2:3:4:5:6:7:8"
            // portParts: "", "123"
            QStringList bracketParts = value.split(QLatin1Char(']'));
            if (bracketParts.size() < 2)
                portParts = QStringList();
            else {
                foundBracket = true;
                if (!bracketParts[1].isEmpty() && bracketParts[1][0] != ':')
                    return QValidator::Invalid;
                else
                    portParts = bracketParts[1].split(QLatin1Char(':'));
            }
            addrParts = bracketParts[0].split(QLatin1Char('['))[1].split(QLatin1Char(':'));
        }
    }

    int number = addrParts.size();
    // There is no case where can be more than 8 colons (9 parts)
    // and only one unusual case where there are 8 colons (1:2:3:4:5:6:7::)
    if (number > 9)
        return QValidator::Invalid;
    else if (number == 9 && (!addrParts[7].isEmpty() || !addrParts[8].isEmpty()))
        return QValidator::Invalid;

    // lets check address parts
    bool emptypresent = false;
    int i = 1;
    for (QString part : std::as_const(addrParts)) {
        if (part.isEmpty() && i < number) {
            // There is only one case where you can have 3 empty parts
            // and that is when you have the string: "::" which is valid
            // and useful and of course it can also be extended to ::123 for
            // instance. Anywhere other than the beginning though, having 3 empty
            // parts indicates either a run of 3 colons ("1:::6")" or two sets of
            // 2 colons ("1:2::3:4::") which are always invalid
            if (emptypresent && i != 2) {
                // qCDebug(PLASMA_NM_EDITOR_LOG) << "part.isEmpty()";
                return QValidator::Invalid;
            } else {
                // If this is an empty part then set it to zero to not fail
                // the next test
                part.setNum(0, 16);
                emptypresent = true;
            }
        }
        i++;

        bool ok;
        if (part.toInt(&ok, 16) > 65535) {
            return QValidator::Invalid;
        }
    }

    // A special case: a single colon needs to be  Intermediate not Acceptable
    if (number == 2 && addrParts[0].isEmpty() && addrParts[1].isEmpty())
        result = QValidator::Intermediate;

    // Another special case: a single colon followed by something (i.e. ":123"
    // is invalid
    else if (number > 1 && addrParts[0].isEmpty() && !addrParts[1].isEmpty())
        result = QValidator::Invalid;

    // If we don't have 8 parts yet and none of them are empty we aren't done yet
    else if (number < 8 && !emptypresent)
        result = QValidator::Intermediate;

    // If we have 8 parts but the last one is empty we aren't done yet
    else if (number == 8 && addrParts[7].isEmpty())
        result = QValidator::Intermediate;

    if (m_addressStyle == WithCidr) {
        int cidrSize = cidrParts.size();

        // If we have a '/' and the basic address portion is not
        // yet complete (i.e. Intermediate) then the whole thing  is Invalid
        if (cidrSize == 2 && result == QValidator::Intermediate)
            return QValidator::Invalid;

        if (cidrSize == 1 || (cidrSize == 2 && cidrParts[1].isEmpty()))
            return QValidator::Intermediate;

        int cidrValue = cidrParts[1].toInt();
        if (cidrValue > 128)
            return QValidator::Invalid;
    } else if (m_addressStyle == WithPort) {
        int portSize = portParts.size();

        // If we have a ']' and the basic address portion is not
        // yet complete (i.e. Intermediate) then the whole thing  is Invalid
        if (foundBracket && result == QValidator::Intermediate)
            return QValidator::Invalid;

        if (portSize < 2 || (portSize == 2 && portParts[1].isEmpty())) {
            return QValidator::Intermediate;
        } else {
            int portValue = portParts[1].toInt();
            if (portValue > 65535)
                return QValidator::Invalid;
        }
    }
    return result;
}

#include "moc_simpleipv6addressvalidator.cpp"
