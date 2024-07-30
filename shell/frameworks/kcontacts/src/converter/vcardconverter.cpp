/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2002 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "vcardconverter.h"
#include "vcardtool_p.h"

using namespace KContacts;

VCardConverter::VCardConverter()
    : d(nullptr)
{
}

VCardConverter::~VCardConverter()
{
}

QByteArray VCardConverter::exportVCard(const Addressee &addr, Version version) const
{
    Addressee::List list;
    list.append(addr);

    return exportVCards(list, version);
}

QByteArray VCardConverter::exportVCards(const Addressee::List &list, Version version) const
{
    VCardTool tool;
    QByteArray returnValue;
    switch (version) {
    case v2_1:
        returnValue = tool.exportVCards(list, VCard::v2_1);
        break;
    case v3_0:
        returnValue = tool.exportVCards(list, VCard::v3_0);
        break;
    case v4_0:
        returnValue = tool.exportVCards(list, VCard::v4_0);
        break;
    }

    return returnValue;
}

QByteArray VCardConverter::createVCard(const Addressee &addr, Version version) const
{
    Addressee::List list;
    list.append(addr);

    return createVCards(list, version);
}

QByteArray VCardConverter::createVCards(const Addressee::List &list, Version version) const
{
    VCardTool tool;
    QByteArray returnValue;
    switch (version) {
    case v2_1:
        returnValue = tool.createVCards(list, VCard::v2_1);
        break;
    case v3_0:
        returnValue = tool.createVCards(list, VCard::v3_0);
        break;
    case v4_0:
        returnValue = tool.createVCards(list, VCard::v4_0);
        break;
    }

    return returnValue;
}

Addressee VCardConverter::parseVCard(const QByteArray &vcard) const
{
    Addressee::List list = parseVCards(vcard);

    return list.isEmpty() ? Addressee() : list[0];
}

Addressee::List VCardConverter::parseVCards(const QByteArray &vcard) const
{
    VCardTool tool;

    return tool.parseVCards(vcard);
}

/* Helper functions */

QString KContacts::dateToVCardString(const QDateTime &dateTime)
{
    return dateTime.toString(QStringLiteral("yyyyMMddThhmmssZ"));
}

QString KContacts::dateToVCardString(QDate date)
{
    return date.toString(QStringLiteral("yyyyMMdd"));
}

QDateTime KContacts::VCardStringToDate(const QString &dateString)
{
    QDate date;
    QTime time;
    QString d(dateString);

    d = d.remove(QLatin1Char('-')).remove(QLatin1Char(':'));

    if (d.length() >= 8) {
        const QStringView strView(d);
        date = QDate(strView.mid(0, 4).toUInt(), strView.mid(4, 2).toUInt(), strView.mid(6, 2).toUInt());
    }

    if (d.length() > 9 && d[8].toUpper() == QLatin1Char('T')) {
        const QStringView strView(d);
        time = QTime(strView.mid(9, 2).toUInt(), strView.mid(11, 2).toUInt(), strView.mid(13, 2).toUInt());
    }

    return QDateTime(date, time);
}

void KContacts::adaptIMAttributes(QByteArray &data)
{
    data.replace("X-messaging/aim-All", ("X-AIM"));
    data.replace("X-messaging/icq-All", ("X-ICQ"));
    data.replace("X-messaging/xmpp-All", ("X-JABBER"));
    data.replace("X-messaging/msn-All", ("X-MSN"));
    data.replace("X-messaging/yahoo-All", ("X-YAHOO"));
    data.replace("X-messaging/gadu-All", ("X-GADUGADU"));
    data.replace("X-messaging/skype-All", ("X-SKYPE"));
    data.replace("X-messaging/groupwise-All", ("X-GROUPWISE"));
    data.replace("X-messaging/sms-All", ("X-SMS"));
    data.replace("X-messaging/meanwhile-All", ("X-MEANWHILE"));
    data.replace("X-messaging/irc-All", ("X-IRC"));
    data.replace("X-messaging/googletalk-All", ("X-GTALK"));
}
