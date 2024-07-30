// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022 Daniel Vrátil <dvratil@kde.org>

#include "ipvalidator.h"

#include <QRegularExpression>

// AF_INET + inet_pton() for Linux, FreeBSD
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace
{

/// Whether @c netmask is within range for the given IP version
QValidator::State checkNetmaskInRange(IPValidator::IPVersion ipVersion, int netmask)
{
    const auto maxNetmask = ipVersion == IPValidator::IPVersion::IPv4 ? 32 : 128;
    if (netmask >= 0 && netmask <= maxNetmask) {
        return QValidator::Acceptable;
    }

    return QValidator::Invalid;
}

/// Whether @c nmStr contains a valid netmask
QValidator::State checkNetmaskValid(IPValidator::IPVersion ipVersion, const QString &nmStr)
{
    if (nmStr.isEmpty()) {
        return QValidator::Intermediate;
    }

    bool valid = false;
    const auto netmask = nmStr.toInt(&valid);
    if (!valid) {
        return QValidator::Invalid;
    }
    return checkNetmaskInRange(ipVersion, netmask);
}

/// Whether @c addr contains a valid IP address
QValidator::State checkAddressValid(IPValidator::IPVersion ipVersion, const QString &addr)
{
    const auto addrChar = addr.toLatin1();
    const int af = ipVersion == IPValidator::IPVersion::IPv4 ? AF_INET : AF_INET6;
    char buf[sizeof(struct in6_addr)]; // Enough for both ipv4 and v6
    // Ask standard library to convert the string representation to a binary representation
    // and check whether it succeeded or not, rather than using a regular expression.
    if (inet_pton(af, addrChar.constData(), buf) == 1) {
        return QValidator::Acceptable;
    }

    return QValidator::Intermediate;
}

QRegularExpression regexForAddress(IPValidator::IPVersion version)
{
    if (version == IPValidator::IPVersion::IPv4) {
        return QRegularExpression{QStringLiteral(R"(^[0-9\./]+$)")};
    } else {
        return QRegularExpression{QStringLiteral(R"(^[a-fA-F0-9:\./]+$)")};
    }
}

} // namespace

IPValidator::IPValidator(QObject *parent)
    : QValidator(parent)
{
}

IPValidator::IPVersion IPValidator::ipVersion() const
{
    return mIPVersion;
}

void IPValidator::setIPVersion(IPVersion version)
{
    if (mIPVersion != version) {
        mIPVersion = version;
        Q_EMIT ipVersionChanged(mIPVersion);
    }
}

QValidator::State IPValidator::validate(QString &arg, int &pos) const
{
    Q_UNUSED(pos);

    const auto rx = regexForAddress(mIPVersion);
    // Quick regex check to immediatelly reject anything that does not look like an IP address.
    if (!rx.match(arg).hasMatch()) {
        return QValidator::Invalid;
    }

    const auto parts = arg.split(QLatin1Char('/'));
    // Check there's at most one split
    if (parts.size() < 1 || parts.size() > 2) {
        return QValidator::Invalid;
    }

    // Validate the address
    const auto addressValidity = checkAddressValid(mIPVersion, parts[0]);
    if (addressValidity == QValidator::Invalid) {
        return QValidator::Invalid;
    }

    // Validate netmask, if present.
    const auto netmaskValidity = parts.size() == 2 ? checkNetmaskValid(mIPVersion, parts[1]) : QValidator::Acceptable;
    if (netmaskValidity == QValidator::Invalid) {
        return QValidator::Invalid;
    }

    return std::min(addressValidity, netmaskValidity);
}
