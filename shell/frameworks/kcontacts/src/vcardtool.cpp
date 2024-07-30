/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2003 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2015-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fieldgroup.h"
#include "gender.h"
#include "kcontacts_debug.h"
#include "key.h"
#include "lang.h"
#include "picture.h"
#include "related.h"
#include "secrecy.h"
#include "sound.h"
#include "vcardtool_p.h"

#include <QString>
#include <QTimeZone>

using namespace KContacts;

static bool needsEncoding(const QString &value)
{
    int length = value.length();
    for (int i = 0; i < length; ++i) {
        char c = value.at(i).toLatin1();
        if ((c < 33 || c > 126) && c != ' ' && c != '=') {
            return true;
        }
    }

    return false;
}

struct AddressTypeInfo {
    const char *addressType;
    Address::TypeFlag flag;
};

static const AddressTypeInfo s_addressTypes[] = {
    {"dom", Address::Dom},
    {"home", Address::Home},
    {"intl", Address::Intl},
    {"parcel", Address::Parcel},
    {"postal", Address::Postal},
    {"pref", Address::Pref},
    {"work", Address::Work},
};

static Address::TypeFlag stringToAddressType(const QString &str)
{
    auto it = std::find_if(std::begin(s_addressTypes), std::end(s_addressTypes), [&str](const AddressTypeInfo &info) {
        return str == QLatin1String(info.addressType);
    });
    return it != std::end(s_addressTypes) ? it->flag : Address::TypeFlag{};
}

struct PhoneTypeInfo {
    const char *phoneType;
    PhoneNumber::TypeFlag flag;
};

static const PhoneTypeInfo s_phoneTypes[] = {
    {"BBS", PhoneNumber::Bbs},
    {"CAR", PhoneNumber::Car},
    {"CELL", PhoneNumber::Cell},
    {"FAX", PhoneNumber::Fax},
    {"HOME", PhoneNumber::Home},
    {"ISDN", PhoneNumber::Isdn},
    {"MODEM", PhoneNumber::Modem},
    {"MSG", PhoneNumber::Msg},
    {"PAGER", PhoneNumber::Pager},
    {"PCS", PhoneNumber::Pcs},
    {"PREF", PhoneNumber::Pref},
    {"VIDEO", PhoneNumber::Video},
    {"VOICE", PhoneNumber::Voice},
    {"WORK", PhoneNumber::Work},
};

static PhoneNumber::TypeFlag stringToPhoneType(const QString &str)
{
    auto it = std::find_if(std::begin(s_phoneTypes), std::end(s_phoneTypes), [&str](const PhoneTypeInfo &info) {
        return str == QLatin1String(info.phoneType);
    });
    return it != std::end(s_phoneTypes) ? it->flag : PhoneNumber::TypeFlag{};
}

VCardTool::VCardTool()
{
}

VCardTool::~VCardTool()
{
}

QByteArray VCardTool::exportVCards(const Addressee::List &list, VCard::Version version) const
{
    return createVCards(list, version, true /*export vcard*/);
}

QByteArray VCardTool::createVCards(const Addressee::List &list, VCard::Version version) const
{
    return createVCards(list, version, false /*don't export*/);
}

void VCardTool::addParameter(VCardLine *line, VCard::Version version, const QString &key, const QStringList &valueStringList) const
{
    if (version == VCard::v2_1) {
        for (const QString &valueStr : valueStringList) {
            line->addParameter(valueStr, QString());
        }
    } else if (version == VCard::v3_0) {
        line->addParameter(key, valueStringList.join(QLatin1Char(',')));
    } else {
        if (valueStringList.count() < 2) {
            line->addParameter(key, valueStringList.join(QLatin1Char(',')));
        } else {
            line->addParameter(key, QLatin1Char('"') + valueStringList.join(QLatin1Char(',')) + QLatin1Char('"'));
        }
    }
}

void VCardTool::processAddresses(const Address::List &addresses, VCard::Version version, VCard *card) const
{
    for (const auto &addr : addresses) {
        QStringList address;

        // clang-format off
        const bool isEmpty = addr.postOfficeBox().isEmpty()
                             && addr.extended().isEmpty()
                             && addr.street().isEmpty()
                             && addr.locality().isEmpty()
                             && addr.region().isEmpty()
                             && addr.postalCode().isEmpty()
                             && addr.country().isEmpty();
        // clang-format on

        address.append(addr.postOfficeBox().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        address.append(addr.extended().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        address.append(addr.street().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        address.append(addr.locality().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        address.append(addr.region().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        address.append(addr.postalCode().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        address.append(addr.country().replace(QLatin1Char(';'), QStringLiteral("\\;")));

        const QString addressJoined(address.join(QLatin1Char(';')));
        VCardLine adrLine(QStringLiteral("ADR"), addressJoined);
        if (version == VCard::v2_1 && needsEncoding(addressJoined)) {
            adrLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
            adrLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
        }

        const bool hasLabel = !addr.label().isEmpty();
        QStringList addreLineType;
        QStringList labelLineType;

        for (const auto &info : s_addressTypes) {
            if (info.flag & addr.type()) {
                const QString str = QString::fromLatin1(info.addressType);
                addreLineType << str;
                if (hasLabel) {
                    labelLineType << str;
                }
            }
        }

        if (hasLabel) {
            if (version == VCard::v4_0) {
                if (!addr.label().isEmpty()) {
                    adrLine.addParameter(QStringLiteral("LABEL"), QStringLiteral("\"%1\"").arg(addr.label()));
                }
            } else {
                VCardLine labelLine(QStringLiteral("LABEL"), addr.label());
                if (version == VCard::v2_1 && needsEncoding(addr.label())) {
                    labelLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
                    labelLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
                }
                addParameter(&labelLine, version, QStringLiteral("TYPE"), labelLineType);
                card->addLine(labelLine);
            }
        }
        if (version == VCard::v4_0) {
            Geo geo = addr.geo();
            if (geo.isValid()) {
                QString str = QString::asprintf("\"geo:%.6f,%.6f\"", geo.latitude(), geo.longitude());
                adrLine.addParameter(QStringLiteral("GEO"), str);
            }
        }
        if (!isEmpty) {
            addParameter(&adrLine, version, QStringLiteral("TYPE"), addreLineType);
            card->addLine(adrLine);
        }
    }
}

void VCardTool::processEmailList(const Email::List &emailList, VCard::Version version, VCard *card) const
{
    for (const auto &email : emailList) {
        VCardLine line(QStringLiteral("EMAIL"), email.mail());
        const ParameterMap pMap = email.params();
        for (const auto &[param, l] : pMap) {
            QStringList list = l;
            if (version == VCard::v2_1) {
                if (param.toLower() == QLatin1String("type")) {
                    bool hasPreferred = false;
                    const int removeItems = list.removeAll(QStringLiteral("PREF"));
                    if (removeItems > 0) {
                        hasPreferred = true;
                    }
                    if (!list.isEmpty()) {
                        addParameter(&line, version, param, list);
                    }
                    if (hasPreferred) {
                        line.addParameter(QStringLiteral("PREF"), QString());
                    }
                } else {
                    line.addParameter(param, list.join(QLatin1Char(',')));
                }
            } else {
                line.addParameter(param, list.join(QLatin1Char(',')));
            }
        }
        card->addLine(line);
    }
}

void VCardTool::processOrganizations(const Addressee &addressee, VCard::Version version, VCard *card) const
{
    const QList<Org> lstOrg = addressee.extraOrganizationList();
    for (const Org &org : lstOrg) {
        QStringList organization{org.organization().replace(QLatin1Char(';'), QLatin1String("\\;"))};
        if (!addressee.department().isEmpty()) {
            organization.append(addressee.department().replace(QLatin1Char(';'), QLatin1String("\\;")));
        }
        const QString orgStr = organization.join(QLatin1Char(';'));
        VCardLine orgLine(QStringLiteral("ORG"), orgStr);
        if (version == VCard::v2_1 && needsEncoding(orgStr)) {
            orgLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
            orgLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
        }
        orgLine.addParameters(org.params());
        card->addLine(orgLine);
    }
}

void VCardTool::processPhoneNumbers(const PhoneNumber::List &phoneNumbers, VCard::Version version, VCard *card) const
{
    for (const auto &phone : phoneNumbers) {
        VCardLine line(QStringLiteral("TEL"), phone.number());
        const ParameterMap paramsMap = phone.params();
        for (const auto &[param, list] : paramsMap) {
            if (param.toUpper() != QLatin1String("TYPE")) {
                line.addParameter(param, list.join(QLatin1Char(',')));
            }
        }

        const PhoneNumber::Type type = phone.type();
        QStringList lst;
        for (const auto &pType : s_phoneTypes) {
            if (pType.flag & type) {
                const QString str = QString::fromLatin1(pType.phoneType);
                if (version == VCard::v4_0) {
                    lst << str.toLower();
                } else {
                    lst << str;
                }
            }
        }
        if (!lst.isEmpty()) {
            addParameter(&line, version, QStringLiteral("TYPE"), lst);
        }
        card->addLine(line);
    }
}

void VCardTool::processCustoms(const QStringList &customs, VCard::Version version, VCard *card, bool exportVcard) const
{
    for (const auto &str : customs) {
        QString identifier = QLatin1String("X-") + QStringView(str).left(str.indexOf(QLatin1Char(':')));
        const QString value = str.mid(str.indexOf(QLatin1Char(':')) + 1);
        if (value.isEmpty()) {
            continue;
        }
        // Convert to standard identifier
        if (exportVcard) {
            if (identifier == QLatin1String("X-messaging/aim-All")) {
                identifier = QStringLiteral("X-AIM");
            } else if (identifier == QLatin1String("X-messaging/icq-All")) {
                identifier = QStringLiteral("X-ICQ");
            } else if (identifier == QLatin1String("X-messaging/xmpp-All")) {
                identifier = QStringLiteral("X-JABBER");
            } else if (identifier == QLatin1String("X-messaging/msn-All")) {
                identifier = QStringLiteral("X-MSN");
            } else if (identifier == QLatin1String("X-messaging/yahoo-All")) {
                identifier = QStringLiteral("X-YAHOO");
            } else if (identifier == QLatin1String("X-messaging/gadu-All")) {
                identifier = QStringLiteral("X-GADUGADU");
            } else if (identifier == QLatin1String("X-messaging/skype-All")) {
                identifier = QStringLiteral("X-SKYPE");
            } else if (identifier == QLatin1String("X-messaging/groupwise-All")) {
                identifier = QStringLiteral("X-GROUPWISE");
            } else if (identifier == QLatin1String("X-messaging/sms-All")) {
                identifier = QStringLiteral("X-SMS");
            } else if (identifier == QLatin1String("X-messaging/meanwhile-All")) {
                identifier = QStringLiteral("X-MEANWHILE");
            } else if (identifier == QLatin1String("X-messaging/irc-All")) {
                identifier = QStringLiteral("X-IRC"); // Not defined by rfc but need for fixing #300869
            } else if (identifier == QLatin1String("X-messaging/googletalk-All")) {
                // Not defined by rfc but need for fixing #300869
                identifier = QStringLiteral("X-GTALK");
            } else if (identifier == QLatin1String("X-messaging/twitter-All")) {
                identifier = QStringLiteral("X-TWITTER");
            }
        }

        if (identifier.toLower() == QLatin1String("x-kaddressbook-x-anniversary") && version == VCard::v4_0) {
            // ANNIVERSARY
            if (!value.isEmpty()) {
                const QDate date = QDate::fromString(value, Qt::ISODate);
                QDateTime dt = QDateTime(date.startOfDay());
                dt.setTime(QTime());
                VCardLine line(QStringLiteral("ANNIVERSARY"), createDateTime(dt, version, false));
                card->addLine(line);
            }
        } else if (identifier.toLower() == QLatin1String("x-kaddressbook-x-spousesname") && version == VCard::v4_0) {
            if (!value.isEmpty()) {
                VCardLine line(QStringLiteral("RELATED"), QStringLiteral(";"));
                line.addParameter(QStringLiteral("TYPE"), QStringLiteral("spouse"));
                line.addParameter(QStringLiteral("VALUE"), value);
                card->addLine(line);
            }
        } else {
            VCardLine line(identifier, value);
            if (version == VCard::v2_1 && needsEncoding(value)) {
                line.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
                line.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
            }
            card->addLine(line);
        }
    }
}

QByteArray VCardTool::createVCards(const Addressee::List &list, VCard::Version version, bool exportVcard) const
{
    VCard::List vCardList;

    for (const auto &addressee : list) {
        VCard card;
        // VERSION
        if (version == VCard::v2_1) {
            card.addLine(VCardLine(QStringLiteral("VERSION"), QStringLiteral("2.1")));
        } else if (version == VCard::v3_0) {
            card.addLine(VCardLine(QStringLiteral("VERSION"), QStringLiteral("3.0")));
        } else if (version == VCard::v4_0) {
            card.addLine(VCardLine(QStringLiteral("VERSION"), QStringLiteral("4.0")));
        }

        // ADR + LABEL
        const Address::List addresses = addressee.addresses();
        processAddresses(addresses, version, &card);

        // BDAY
        const bool withTime = addressee.birthdayHasTime();
        const QString birthdayString = createDateTime(addressee.birthday(), version, withTime);
        card.addLine(VCardLine(QStringLiteral("BDAY"), birthdayString));

        // CATEGORIES only > 2.1
        if (version != VCard::v2_1) {
            QStringList categories = addressee.categories();
            for (auto &cat : categories) {
                cat.replace(QLatin1Char(','), QLatin1String("\\,"));
            }

            VCardLine catLine(QStringLiteral("CATEGORIES"), categories.join(QLatin1Char(',')));
            card.addLine(catLine);
        }
        // MEMBER (only in 4.0)
        if (version == VCard::v4_0) {
            // The KIND property must be set to "group" in order to use this property.
            if (addressee.kind().toLower() == QLatin1String("group")) {
                const QStringList lst = addressee.members();
                for (const QString &member : lst) {
                    card.addLine(VCardLine(QStringLiteral("MEMBER"), member));
                }
            }
        }
        // SOURCE
        const QList<QUrl> lstUrl = addressee.sourcesUrlList();
        for (const QUrl &url : lstUrl) {
            VCardLine line = VCardLine(QStringLiteral("SOURCE"), url.url());
            card.addLine(line);
        }

        const Related::List relatedList = addressee.relationships();
        for (const auto &rel : relatedList) {
            VCardLine line(QStringLiteral("RELATED"), rel.related());
            line.addParameters(rel.params());
            card.addLine(line);
        }
        // CLASS only for version == 3.0
        if (version == VCard::v3_0) {
            card.addLine(createSecrecy(addressee.secrecy()));
        }
        // LANG only for version == 4.0
        if (version == VCard::v4_0) {
            const Lang::List langList = addressee.langs();
            for (const auto &lang : langList) {
                VCardLine line(QStringLiteral("LANG"), lang.language());
                line.addParameters(lang.params());
                card.addLine(line);
            }
        }
        // CLIENTPIDMAP
        if (version == VCard::v4_0) {
            const ClientPidMap::List clientpidmapList = addressee.clientPidMapList();
            for (const auto &pMap : clientpidmapList) {
                VCardLine line(QStringLiteral("CLIENTPIDMAP"), pMap.clientPidMap());
                line.addParameters(pMap.params());
                card.addLine(line);
            }
        }
        // EMAIL
        const Email::List emailList = addressee.emailList();
        processEmailList(emailList, version, &card);

        // FN required for only version > 2.1
        VCardLine fnLine(QStringLiteral("FN"), addressee.formattedName());
        if (version == VCard::v2_1 && needsEncoding(addressee.formattedName())) {
            fnLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
            fnLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
        }
        card.addLine(fnLine);

        // GEO
        const Geo geo = addressee.geo();
        if (geo.isValid()) {
            QString str;
            if (version == VCard::v4_0) {
                str = QString::asprintf("geo:%.6f,%.6f", geo.latitude(), geo.longitude());
            } else {
                str = QString::asprintf("%.6f;%.6f", geo.latitude(), geo.longitude());
            }
            card.addLine(VCardLine(QStringLiteral("GEO"), str));
        }

        // KEY
        const Key::List keys = addressee.keys();
        for (const auto &k : keys) {
            card.addLine(createKey(k, version));
        }

        // LOGO
        card.addLine(createPicture(QStringLiteral("LOGO"), addressee.logo(), version));
        const QList<Picture> lstLogo = addressee.extraLogoList();
        for (const Picture &logo : lstLogo) {
            card.addLine(createPicture(QStringLiteral("LOGO"), logo, version));
        }

        // MAILER only for version < 4.0
        if (version != VCard::v4_0) {
            VCardLine mailerLine(QStringLiteral("MAILER"), addressee.mailer());
            if (version == VCard::v2_1 && needsEncoding(addressee.mailer())) {
                mailerLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
                mailerLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
            }
            card.addLine(mailerLine);
        }

        // N required for only version < 4.0
        QStringList name;
        name.append(addressee.familyName().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        name.append(addressee.givenName().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        name.append(addressee.additionalName().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        name.append(addressee.prefix().replace(QLatin1Char(';'), QStringLiteral("\\;")));
        name.append(addressee.suffix().replace(QLatin1Char(';'), QStringLiteral("\\;")));

        VCardLine nLine(QStringLiteral("N"), name.join(QLatin1Char(';')));
        if (version == VCard::v2_1 && needsEncoding(name.join(QLatin1Char(';')))) {
            nLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
            nLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
        }
        if (version == VCard::v4_0 && !addressee.sortString().isEmpty()) {
            nLine.addParameter(QStringLiteral("SORT-AS"), addressee.sortString());
        }

        card.addLine(nLine);

        // NAME only for version < 4.0
        if (version != VCard::v4_0) {
            VCardLine nameLine(QStringLiteral("NAME"), addressee.name());
            if (version == VCard::v2_1 && needsEncoding(addressee.name())) {
                nameLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
                nameLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
            }
            card.addLine(nameLine);
        }

        // NICKNAME only for version > 2.1
        if (version != VCard::v2_1) {
            const QList<NickName> lstNickName = addressee.extraNickNameList();
            for (const NickName &nickName : lstNickName) {
                VCardLine nickNameLine(QStringLiteral("NICKNAME"), nickName.nickname());
                nickNameLine.addParameters(nickName.params());

                card.addLine(nickNameLine);
            }
        }

        // NOTE
        VCardLine noteLine(QStringLiteral("NOTE"), addressee.note());
        if (version == VCard::v2_1 && needsEncoding(addressee.note())) {
            noteLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
            noteLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
        }
        card.addLine(noteLine);

        // ORG
        processOrganizations(addressee, version, &card);

        // PHOTO
        card.addLine(createPicture(QStringLiteral("PHOTO"), addressee.photo(), version));
        const QList<Picture> lstExtraPhoto = addressee.extraPhotoList();
        for (const Picture &photo : lstExtraPhoto) {
            card.addLine(createPicture(QStringLiteral("PHOTO"), photo, version));
        }

        // PROID only for version > 2.1
        if (version != VCard::v2_1) {
            card.addLine(VCardLine(QStringLiteral("PRODID"), addressee.productId()));
        }

        // REV
        card.addLine(VCardLine(QStringLiteral("REV"), createDateTime(addressee.revision(), version)));

        // ROLE
        const QList<Role> lstExtraRole = addressee.extraRoleList();
        for (const Role &role : lstExtraRole) {
            VCardLine roleLine(QStringLiteral("ROLE"), role.role());
            if (version == VCard::v2_1 && needsEncoding(role.role())) {
                roleLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
                roleLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
            }
            roleLine.addParameters(role.params());
            card.addLine(roleLine);
        }

        // SORT-STRING
        if (version == VCard::v3_0) {
            card.addLine(VCardLine(QStringLiteral("SORT-STRING"), addressee.sortString()));
        }

        // SOUND
        card.addLine(createSound(addressee.sound(), version));
        const QList<Sound> lstSound = addressee.extraSoundList();
        for (const Sound &sound : lstSound) {
            card.addLine(createSound(sound, version));
        }

        // TEL
        const PhoneNumber::List phoneNumbers = addressee.phoneNumbers();
        processPhoneNumbers(phoneNumbers, version, &card);

        // TITLE
        const QList<Title> lstTitle = addressee.extraTitleList();
        for (const Title &title : lstTitle) {
            VCardLine titleLine(QStringLiteral("TITLE"), title.title());
            if (version == VCard::v2_1 && needsEncoding(title.title())) {
                titleLine.addParameter(QStringLiteral("charset"), QStringLiteral("UTF-8"));
                titleLine.addParameter(QStringLiteral("encoding"), QStringLiteral("QUOTED-PRINTABLE"));
            }
            titleLine.addParameters(title.params());

            card.addLine(titleLine);
        }

        // TZ
        // TODO Add vcard4.0 support
        const TimeZone timeZone = addressee.timeZone();
        if (timeZone.isValid()) {
            int neg = 1;
            if (timeZone.offset() < 0) {
                neg = -1;
            }

            QString str =
                QString::asprintf("%c%02d:%02d", (timeZone.offset() >= 0 ? '+' : '-'), (timeZone.offset() / 60) * neg, (timeZone.offset() % 60) * neg);

            card.addLine(VCardLine(QStringLiteral("TZ"), str));
        }

        // UID
        card.addLine(VCardLine(QStringLiteral("UID"), addressee.uid()));

        // URL
        const QList<ResourceLocatorUrl> lstExtraUrl = addressee.extraUrlList();
        for (const ResourceLocatorUrl &url : lstExtraUrl) {
            VCardLine line(QStringLiteral("URL"), url.url());
            line.addParameters(url.params());
            card.addLine(line);
        }
        if (version == VCard::v4_0) {
            // GENDER
            const Gender gender = addressee.gender();
            if (gender.isValid()) {
                QString genderStr;
                if (!gender.gender().isEmpty()) {
                    genderStr = gender.gender();
                }
                if (!gender.comment().isEmpty()) {
                    genderStr += QLatin1Char(';') + gender.comment();
                }
                VCardLine line(QStringLiteral("GENDER"), genderStr);
                card.addLine(line);
            }
            // KIND
            if (!addressee.kind().isEmpty()) {
                VCardLine line(QStringLiteral("KIND"), addressee.kind());
                card.addLine(line);
            }
        }
        // From vcard4.
        if (version == VCard::v4_0) {
            const QList<CalendarUrl> lstCalendarUrl = addressee.calendarUrlList();
            for (const CalendarUrl &url : lstCalendarUrl) {
                if (url.isValid()) {
                    QString type;
                    switch (url.type()) {
                    case CalendarUrl::Unknown:
                    case CalendarUrl::EndCalendarType:
                        break;
                    case CalendarUrl::FBUrl:
                        type = QStringLiteral("FBURL");
                        break;
                    case CalendarUrl::CALUri:
                        type = QStringLiteral("CALURI");
                        break;
                    case CalendarUrl::CALADRUri:
                        type = QStringLiteral("CALADRURI");
                        break;
                    }
                    if (!type.isEmpty()) {
                        VCardLine line(type, url.url().toDisplayString());
                        line.addParameters(url.params());
                        card.addLine(line);
                    }
                }
            }
        }

        // FieldGroup
        const QList<FieldGroup> lstGroup = addressee.fieldGroupList();
        for (const FieldGroup &group : lstGroup) {
            VCardLine line(group.fieldGroupName(), group.value());
            line.addParameters(group.params());
            card.addLine(line);
        }

        // IMPP (supported in vcard 3 too)
        const QList<Impp> lstImpp = addressee.imppList();
        for (const Impp &impp : lstImpp) {
            VCardLine line(QStringLiteral("IMPP"), impp.address().url());
            const ParameterMap pMap = impp.params();
            for (const auto &[param, list] : pMap) {
                if (param.toLower() != QLatin1String("x-service-type")) {
                    line.addParameter(param, list.join(QLatin1Char(',')));
                }
            }
            card.addLine(line);
        }

        // X-
        const QStringList customs = addressee.customs();
        processCustoms(customs, version, &card, exportVcard);

        vCardList.append(card);
    }

    return VCardParser::createVCards(vCardList);
}

Addressee::List VCardTool::parseVCards(const QByteArray &vcard) const
{
    static const QLatin1Char semicolonSep(';');
    static const QLatin1Char commaSep(',');
    QString identifier;
    QString group;
    Addressee::List addrList;
    const VCard::List vCardList = VCardParser::parseVCards(vcard);

    VCard::List::ConstIterator cardIt;
    VCard::List::ConstIterator listEnd(vCardList.end());
    for (cardIt = vCardList.begin(); cardIt != listEnd; ++cardIt) {
        Addressee addr;

        const QStringList idents = (*cardIt).identifiers();
        QStringList::ConstIterator identIt;
        QStringList::ConstIterator identEnd(idents.end());
        for (identIt = idents.begin(); identIt != identEnd; ++identIt) {
            const VCardLine::List lines = (*cardIt).lines((*identIt));
            VCardLine::List::ConstIterator lineIt;

            // iterate over the lines
            for (lineIt = lines.begin(); lineIt != lines.end(); ++lineIt) {
                identifier = (*lineIt).identifier().toLower();
                group = (*lineIt).group();
                if (!group.isEmpty() && identifier != QLatin1String("adr")) {
                    KContacts::FieldGroup groupField(group + QLatin1Char('.') + (*lineIt).identifier());
                    groupField.setParams((*lineIt).parameterMap());
                    groupField.setValue((*lineIt).value().toString());
                    addr.insertFieldGroup(groupField);
                }
                // ADR
                else if (identifier == QLatin1String("adr")) {
                    Address address;
                    const QStringList addrParts = splitString(semicolonSep, (*lineIt).value().toString());
                    const int addrPartsCount(addrParts.count());
                    if (addrPartsCount > 0) {
                        address.setPostOfficeBox(addrParts.at(0));
                    }
                    if (addrPartsCount > 1) {
                        address.setExtended(addrParts.at(1));
                    }
                    if (addrPartsCount > 2) {
                        address.setStreet(addrParts.at(2));
                    }
                    if (addrPartsCount > 3) {
                        address.setLocality(addrParts.at(3));
                    }
                    if (addrPartsCount > 4) {
                        address.setRegion(addrParts.at(4));
                    }
                    if (addrPartsCount > 5) {
                        address.setPostalCode(addrParts.at(5));
                    }
                    if (addrPartsCount > 6) {
                        address.setCountry(addrParts.at(6));
                    }

                    Address::Type type;

                    const QStringList types = (*lineIt).parameters(QStringLiteral("type"));
                    QStringList::ConstIterator end(types.end());
                    for (QStringList::ConstIterator it = types.begin(); it != end; ++it) {
                        type |= stringToAddressType((*it).toLower());
                    }

                    address.setType(type);
                    QString label = (*lineIt).parameter(QStringLiteral("label"));
                    if (!label.isEmpty()) {
                        if (label.length() > 1) {
                            if (label.at(0) == QLatin1Char('"') && label.at(label.length() - 1) == QLatin1Char('"')) {
                                label = label.mid(1, label.length() - 2);
                            }
                        }
                        address.setLabel(label);
                    }
                    QString geoStr = (*lineIt).parameter(QStringLiteral("geo"));
                    if (!geoStr.isEmpty()) {
                        geoStr.remove(QLatin1Char('\"'));
                        geoStr.remove(QStringLiteral("geo:"));
                        if (geoStr.contains(QLatin1Char(','))) {
                            QStringList arguments = geoStr.split(QLatin1Char(','));
                            KContacts::Geo geo;
                            geo.setLatitude(arguments.at(0).toDouble());
                            geo.setLongitude(arguments.at(1).toDouble());
                            address.setGeo(geo);
                        }
                    }
                    addr.insertAddress(address);
                }
                // ANNIVERSARY
                else if (identifier == QLatin1String("anniversary")) {
                    const QString t = (*lineIt).value().toString();
                    const QDateTime dt(parseDateTime(t));
                    addr.insertCustom(QStringLiteral("KADDRESSBOOK"), QStringLiteral("X-Anniversary"), dt.date().toString(Qt::ISODate));
                }
                // BDAY
                else if (identifier == QLatin1String("bday")) {
                    bool withTime;
                    const QDateTime bday = parseDateTime((*lineIt).value().toString(), &withTime);
                    addr.setBirthday(bday, withTime);
                }
                // CATEGORIES
                else if (identifier == QLatin1String("categories")) {
                    const QStringList categories = splitString(commaSep, (*lineIt).value().toString());
                    addr.setCategories(categories);
                }
                // FBURL
                else if (identifier == QLatin1String("fburl")) {
                    CalendarUrl calurl;
                    calurl.setType(CalendarUrl::FBUrl);
                    const QUrl url = QUrl((*lineIt).value().toString());
                    calurl.setUrl(url);
                    calurl.setParams((*lineIt).parameterMap());
                    addr.insertCalendarUrl(calurl);
                }
                // CALADRURI
                else if (identifier == QLatin1String("caladruri")) {
                    CalendarUrl calurl;
                    calurl.setType(CalendarUrl::CALADRUri);
                    const QUrl url = QUrl((*lineIt).value().toString());
                    calurl.setUrl(url);
                    calurl.setParams((*lineIt).parameterMap());
                    addr.insertCalendarUrl(calurl);
                }
                // CALURI
                else if (identifier == QLatin1String("caluri")) {
                    CalendarUrl calurl;
                    calurl.setType(CalendarUrl::CALUri);
                    const QUrl url = QUrl((*lineIt).value().toString());
                    calurl.setUrl(url);
                    calurl.setParams((*lineIt).parameterMap());
                    addr.insertCalendarUrl(calurl);
                }
                // IMPP
                else if (identifier == QLatin1String("impp")) {
                    QUrl imppUrl((*lineIt).value().toString());
                    Impp impp;
                    impp.setParams((*lineIt).parameterMap());
                    if (!(*lineIt).parameter(QStringLiteral("x-service-type")).isEmpty() && imppUrl.scheme().isEmpty()) {
                        imppUrl.setScheme(normalizeImppServiceType((*lineIt).parameter(QStringLiteral("x-service-type")).toLower()));
                    }
                    impp.setAddress(imppUrl);
                    addr.insertImpp(impp);
                }
                // CLASS
                else if (identifier == QLatin1String("class")) {
                    addr.setSecrecy(parseSecrecy(*lineIt));
                }
                // GENDER
                else if (identifier == QLatin1String("gender")) {
                    QString genderStr = (*lineIt).value().toString();
                    if (!genderStr.isEmpty()) {
                        Gender gender;
                        if (genderStr.at(0) != QLatin1Char(';')) {
                            gender.setGender(genderStr.at(0));
                            if (genderStr.length() > 2 && (genderStr.at(1) == QLatin1Char(';'))) {
                                gender.setComment(genderStr.right(genderStr.length() - 2));
                            }
                        } else {
                            gender.setComment(genderStr.right(genderStr.length() - 1));
                        }
                        addr.setGender(gender);
                    }
                }
                // LANG
                else if (identifier == QLatin1String("lang")) {
                    Lang lang;
                    lang.setLanguage((*lineIt).value().toString());
                    lang.setParams((*lineIt).parameterMap());
                    addr.insertLang(lang);
                }
                // EMAIL
                else if (identifier == QLatin1String("email")) {
                    const QStringList types = (*lineIt).parameters(QStringLiteral("type"));
                    Email mail((*lineIt).value().toString());
                    mail.setParams((*lineIt).parameterMap());
                    addr.addEmail(mail);
                }
                // KIND
                else if (identifier == QLatin1String("kind")) {
                    addr.setKind((*lineIt).value().toString());
                }
                // FN
                else if (identifier == QLatin1String("fn")) {
                    addr.setFormattedName((*lineIt).value().toString());
                }
                // GEO
                else if (identifier == QLatin1String("geo")) {
                    Geo geo;
                    QString lineStr = (*lineIt).value().toString();
                    if (lineStr.startsWith(QLatin1String("geo:"))) { // VCard 4.0
                        lineStr.remove(QStringLiteral("geo:"));
                        const QStringList geoParts = lineStr.split(QLatin1Char(','), Qt::KeepEmptyParts);
                        if (geoParts.size() >= 2) {
                            geo.setLatitude(geoParts.at(0).toFloat());
                            geo.setLongitude(geoParts.at(1).toFloat());
                            addr.setGeo(geo);
                        }
                    } else {
                        const QStringList geoParts = lineStr.split(QLatin1Char(';'), Qt::KeepEmptyParts);
                        if (geoParts.size() >= 2) {
                            geo.setLatitude(geoParts.at(0).toFloat());
                            geo.setLongitude(geoParts.at(1).toFloat());
                            addr.setGeo(geo);
                        }
                    }
                }
                // KEY
                else if (identifier == QLatin1String("key")) {
                    addr.insertKey(parseKey(*lineIt));
                }
                // LABEL
                else if (identifier == QLatin1String("label")) {
                    Address::Type type;

                    const QStringList types = (*lineIt).parameters(QStringLiteral("type"));
                    QStringList::ConstIterator end(types.end());
                    for (QStringList::ConstIterator it = types.begin(); it != end; ++it) {
                        type |= stringToAddressType((*it).toLower());
                    }

                    bool available = false;
                    KContacts::Address::List addressList = addr.addresses();
                    for (KContacts::Address::List::Iterator it = addressList.begin(); it != addressList.end(); ++it) {
                        if ((*it).type() == type) {
                            (*it).setLabel((*lineIt).value().toString());
                            addr.insertAddress(*it);
                            available = true;
                            break;
                        }
                    }

                    if (!available) { // a standalone LABEL tag
                        KContacts::Address address(type);
                        address.setLabel((*lineIt).value().toString());
                        addr.insertAddress(address);
                    }
                }
                // LOGO
                else if (identifier == QLatin1String("logo")) {
                    Picture picture = parsePicture(*lineIt);
                    if (addr.logo().isEmpty()) {
                        addr.setLogo(picture);
                    } else {
                        addr.insertExtraLogo(picture);
                    }
                }
                // MAILER
                else if (identifier == QLatin1String("mailer")) {
                    addr.setMailer((*lineIt).value().toString());
                }
                // N
                else if (identifier == QLatin1Char('n')) {
                    const QStringList nameParts = splitString(semicolonSep, (*lineIt).value().toString());
                    const int numberOfParts(nameParts.count());
                    if (numberOfParts > 0) {
                        addr.setFamilyName(nameParts.at(0));
                    }
                    if (numberOfParts > 1) {
                        addr.setGivenName(nameParts.at(1));
                    }
                    if (numberOfParts > 2) {
                        addr.setAdditionalName(nameParts.at(2));
                    }
                    if (numberOfParts > 3) {
                        addr.setPrefix(nameParts.at(3));
                    }
                    if (numberOfParts > 4) {
                        addr.setSuffix(nameParts.at(4));
                    }
                    if (!(*lineIt).parameter(QStringLiteral("sort-as")).isEmpty()) {
                        addr.setSortString((*lineIt).parameter(QStringLiteral("sort-as")));
                    }
                }
                // NAME
                else if (identifier == QLatin1String("name")) {
                    addr.setName((*lineIt).value().toString());
                }
                // NICKNAME
                else if (identifier == QLatin1String("nickname")) {
                    NickName nickName((*lineIt).value().toString());
                    nickName.setParams((*lineIt).parameterMap());
                    addr.insertExtraNickName(nickName);
                }
                // NOTE
                else if (identifier == QLatin1String("note")) {
                    addr.setNote((*lineIt).value().toString());
                }
                // ORGANIZATION
                else if (identifier == QLatin1String("org")) {
                    const QStringList orgParts = splitString(semicolonSep, (*lineIt).value().toString());
                    const int orgPartsCount(orgParts.count());
                    if (orgPartsCount > 0) {
                        Org organization(orgParts.at(0));
                        organization.setParams((*lineIt).parameterMap());
                        addr.insertExtraOrganization(organization);
                    }
                    if (orgPartsCount > 1) {
                        addr.setDepartment(orgParts.at(1));
                    }
                    if (!(*lineIt).parameter(QStringLiteral("sort-as")).isEmpty()) {
                        addr.setSortString((*lineIt).parameter(QStringLiteral("sort-as")));
                    }
                }
                // PHOTO
                else if (identifier == QLatin1String("photo")) {
                    Picture picture = parsePicture(*lineIt);
                    if (addr.photo().isEmpty()) {
                        addr.setPhoto(picture);
                    } else {
                        addr.insertExtraPhoto(picture);
                    }
                }
                // PROID
                else if (identifier == QLatin1String("prodid")) {
                    addr.setProductId((*lineIt).value().toString());
                }
                // REV
                else if (identifier == QLatin1String("rev")) {
                    addr.setRevision(parseDateTime((*lineIt).value().toString()));
                }
                // ROLE
                else if (identifier == QLatin1String("role")) {
                    Role role((*lineIt).value().toString());
                    role.setParams((*lineIt).parameterMap());
                    addr.insertExtraRole(role);
                }
                // SORT-STRING
                else if (identifier == QLatin1String("sort-string")) {
                    addr.setSortString((*lineIt).value().toString());
                }
                // SOUND
                else if (identifier == QLatin1String("sound")) {
                    Sound sound = parseSound(*lineIt);
                    if (addr.sound().isEmpty()) {
                        addr.setSound(sound);
                    } else {
                        addr.insertExtraSound(sound);
                    }
                }
                // TEL
                else if (identifier == QLatin1String("tel")) {
                    PhoneNumber phone;
                    phone.setNumber((*lineIt).value().toString());

                    PhoneNumber::Type type;
                    bool foundType = false;
                    const QStringList types = (*lineIt).parameters(QStringLiteral("type"));
                    QStringList::ConstIterator typeEnd(types.constEnd());
                    for (QStringList::ConstIterator it = types.constBegin(); it != typeEnd; ++it) {
                        type |= stringToPhoneType((*it).toUpper());
                        foundType = true;
                    }
                    phone.setType(foundType ? type : PhoneNumber::Undefined);
                    phone.setParams((*lineIt).parameterMap());

                    addr.insertPhoneNumber(phone);
                }
                // TITLE
                else if (identifier == QLatin1String("title")) {
                    Title title((*lineIt).value().toString());
                    title.setParams((*lineIt).parameterMap());
                    addr.insertExtraTitle(title);
                }
                // TZ
                else if (identifier == QLatin1String("tz")) {
                    // TODO add vcard4 support
                    TimeZone tz;
                    const QString date = (*lineIt).value().toString();

                    if (!date.isEmpty()) {
                        const QStringView dateView(date);
                        int hours = dateView.mid(1, 2).toInt();
                        int minutes = dateView.mid(4, 2).toInt();
                        int offset = (hours * 60) + minutes;
                        offset = offset * (date[0] == QLatin1Char('+') ? 1 : -1);

                        tz.setOffset(offset);
                        addr.setTimeZone(tz);
                    }
                }
                // UID
                else if (identifier == QLatin1String("uid")) {
                    addr.setUid((*lineIt).value().toString());
                }
                // URL
                else if (identifier == QLatin1String("url")) {
                    const QUrl url = QUrl((*lineIt).value().toString());
                    ResourceLocatorUrl resourceLocatorUrl;
                    resourceLocatorUrl.setUrl(url);
                    resourceLocatorUrl.setParams((*lineIt).parameterMap());
                    addr.insertExtraUrl(resourceLocatorUrl);
                }
                // SOURCE
                else if (identifier == QLatin1String("source")) {
                    const QUrl url = QUrl((*lineIt).value().toString());
                    addr.insertSourceUrl(url);
                }
                // MEMBER (vcard 4.0)
                else if (identifier == QLatin1String("member")) {
                    addr.insertMember((*lineIt).value().toString());
                }
                // RELATED (vcard 4.0)
                else if (identifier == QLatin1String("related")) {
                    Related related;
                    related.setRelated((*lineIt).value().toString());
                    related.setParams((*lineIt).parameterMap());
                    addr.insertRelationship(related);
                }
                // CLIENTPIDMAP (vcard 4.0)
                else if (identifier == QLatin1String("clientpidmap")) {
                    ClientPidMap clientpidmap;
                    clientpidmap.setClientPidMap((*lineIt).value().toString());
                    clientpidmap.setParams((*lineIt).parameterMap());
                    addr.insertClientPidMap(clientpidmap);
                }
                // X-
                // TODO import X-GENDER
                else if (identifier.startsWith(QLatin1String("x-"))) {
                    QString ident = (*lineIt).identifier();
                    // clang-format off
                    //X-Evolution
                    // also normalize case of our own extensions, some backends "adjust" that
                    if (identifier == QLatin1String("x-evolution-spouse")
                        || identifier == QLatin1String("x-spouse")) {
                        ident = QStringLiteral("X-KADDRESSBOOK-X-SpousesName");
                    } else if (identifier == QLatin1String("x-evolution-blog-url") || identifier.compare(QLatin1String("X-KADDRESSBOOK-BLOGFEED"), Qt::CaseInsensitive) == 0) {
                        ident = QStringLiteral("X-KADDRESSBOOK-BlogFeed");
                    } else if (identifier == QLatin1String("x-evolution-assistant")
                               || identifier == QLatin1String("x-assistant")
                               || identifier.compare(QLatin1String("X-KADDRESSBOOK-X-ASSISTANTSNAME"), Qt::CaseInsensitive) == 0) {
                        ident = QStringLiteral("X-KADDRESSBOOK-X-AssistantsName");
                    } else if (identifier == QLatin1String("x-evolution-anniversary")
                               || identifier == QLatin1String("x-anniversary")
                               || identifier.compare(QLatin1String("X-KADDRESSBOOK-X-ANNIVERSARY"), Qt::CaseInsensitive) == 0) {
                        ident = QStringLiteral("X-KADDRESSBOOK-X-Anniversary");
                    } else if (identifier == QLatin1String("x-evolution-manager")
                               || identifier == QLatin1String("x-manager")
                               || identifier.compare(QLatin1String("X-KADDRESSBOOK-X-MANAGERSNAME"), Qt::CaseInsensitive) == 0) {
                        // clang-format on
                        ident = QStringLiteral("X-KADDRESSBOOK-X-ManagersName");
                    } else if (identifier.compare(QLatin1String("X-KADDRESSBOOK-X-PROFESSION"), Qt::CaseInsensitive) == 0) {
                        ident = QStringLiteral("X-KADDRESSBOOK-X-Profession");
                    } else if (identifier.compare(QLatin1String("X-KADDRESSBOOK-X-OFFICE"), Qt::CaseInsensitive) == 0) {
                        ident = QStringLiteral("X-KADDRESSBOOK-X-Office");
                    } else if (identifier.compare(QLatin1String("X-KADDRESSBOOK-X-SPOUSESNAME"), Qt::CaseInsensitive) == 0) {
                        ident = QStringLiteral("X-KADDRESSBOOK-X-SpousesName");
                    } else if (identifier == QLatin1String("x-aim")) {
                        ident = QStringLiteral("X-messaging/aim-All");
                    } else if (identifier == QLatin1String("x-icq")) {
                        ident = QStringLiteral("X-messaging/icq-All");
                    } else if (identifier == QLatin1String("x-jabber")) {
                        ident = QStringLiteral("X-messaging/xmpp-All");
                    } else if (identifier == QLatin1String("x-jabber")) {
                        ident = QStringLiteral("X-messaging/xmpp-All");
                    } else if (identifier == QLatin1String("x-msn")) {
                        ident = QStringLiteral("X-messaging/msn-All");
                    } else if (identifier == QLatin1String("x-yahoo")) {
                        ident = QStringLiteral("X-messaging/yahoo-All");
                    } else if (identifier == QLatin1String("x-gadugadu")) {
                        ident = QStringLiteral("X-messaging/gadu-All");
                    } else if (identifier == QLatin1String("x-skype")) {
                        ident = QStringLiteral("X-messaging/skype-All");
                    } else if (identifier == QLatin1String("x-groupwise")) {
                        ident = QStringLiteral("X-messaging/groupwise-All");
                    } else if (identifier == QLatin1String("x-sms")) {
                        ident = QStringLiteral("X-messaging/sms-All");
                    } else if (identifier == QLatin1String("x-meanwhile")) {
                        ident = QStringLiteral("X-messaging/meanwhile-All");
                    } else if (identifier == QLatin1String("x-irc")) {
                        ident = QStringLiteral("X-messaging/irc-All");
                    } else if (identifier == QLatin1String("x-gtalk")) {
                        ident = QStringLiteral("X-messaging/googletalk-All");
                    } else if (identifier == QLatin1String("x-twitter")) {
                        ident = QStringLiteral("X-messaging/twitter-All");
                    }

                    const QString key = ident.mid(2);
                    const int dash = key.indexOf(QLatin1Char('-'));

                    // convert legacy messaging fields into IMPP ones
                    if (key.startsWith(QLatin1String("messaging/"))) {
                        QUrl url;
                        url.setScheme(normalizeImppServiceType(key.mid(10, dash - 10)));
                        const auto values = (*lineIt).value().toString().split(QChar(0xE000), Qt::SkipEmptyParts);
                        for (const auto &value : values) {
                            url.setPath(value);
                            Impp impp;
                            impp.setParams((*lineIt).parameterMap());
                            impp.setAddress(url);
                            addr.insertImpp(impp);
                        }
                    } else {
                        addr.insertCustom(key.left(dash), key.mid(dash + 1), (*lineIt).value().toString());
                    }
                }
            }
        }

        addrList.append(addr);
    }

    return addrList;
}

QDateTime VCardTool::parseDateTime(const QString &str, bool *timeValid)
{
    static const QLatin1Char sep('-');

    const int posT = str.indexOf(QLatin1Char('T'));
    QString dateString = posT >= 0 ? str.left(posT) : str;
    const bool noYear = dateString.startsWith(QLatin1String("--"));
    dateString.remove(QLatin1Char('-'));
    QDate date;

    const QStringView dstr{dateString};
    if (noYear) {
        date.setDate(-1, dstr.mid(0, 2).toInt(), dstr.mid(2, 2).toInt());
    } else {
        // E.g. 20160120
        date.setDate(dstr.mid(0, 4).toInt(), dstr.mid(4, 2).toInt(), dstr.mid(6, 2).toInt());
    }

    QTime time;
    QTimeZone tz = QTimeZone::LocalTime;
    if (posT >= 0) {
        QString timeString = str.mid(posT + 1);
        timeString.remove(QLatin1Char(':'));
        const int zPos = timeString.indexOf(QLatin1Char('Z'));
        const int plusPos = timeString.indexOf(QLatin1Char('+'));
        const int minusPos = timeString.indexOf(sep);
        const int tzPos = qMax(qMax(zPos, plusPos), minusPos);
        const QStringView hhmmssString = tzPos >= 0 ? QStringView(timeString).left(tzPos) : QStringView(timeString);
        int hour = 0;
        int minutes = 0;
        int seconds = 0;
        switch (hhmmssString.size()) {
        case 2:
            hour = hhmmssString.toInt();
            break;
        case 4:
            hour = hhmmssString.mid(0, 2).toInt();
            minutes = hhmmssString.mid(2, 2).toInt();
            break;
        case 6:
            hour = hhmmssString.mid(0, 2).toInt();
            minutes = hhmmssString.mid(2, 2).toInt();
            seconds = hhmmssString.mid(4, 2).toInt();
            break;
        }
        time.setHMS(hour, minutes, seconds);

        if (tzPos >= 0) {
            if (zPos >= 0) {
                tz = QTimeZone::UTC;
            } else {
                int offsetSecs = 0;
                const auto offsetString = QStringView(timeString).mid(tzPos + 1);
                switch (offsetString.size()) {
                case 2: // format: "hh"
                    offsetSecs = offsetString.left(2).toInt() * 3600;
                    break;
                case 4: // format: "hhmm"
                    offsetSecs = offsetString.left(2).toInt() * 3600 + offsetString.mid(2, 2).toInt() * 60;
                    break;
                }
                if (minusPos >= 0) {
                    offsetSecs *= -1;
                }
                tz = QTimeZone::fromSecondsAheadOfUtc(offsetSecs);
            }
        }
    }
    if (timeValid) {
        *timeValid = time.isValid();
    }

    return QDateTime(date, time, tz);
}

QString VCardTool::createDateTime(const QDateTime &dateTime, VCard::Version version, bool withTime)
{
    if (!dateTime.date().isValid()) {
        return QString();
    }
    QString str = createDate(dateTime.date(), version);
    if (!withTime) {
        return str;
    }
    str += createTime(dateTime.time(), version);
    if (dateTime.timeSpec() == Qt::UTC) {
        str += QLatin1Char('Z');
    } else if (dateTime.timeSpec() == Qt::OffsetFromUTC) {
        const int offsetSecs = dateTime.offsetFromUtc();
        if (offsetSecs >= 0) {
            str += QLatin1Char('+');
        } else {
            str += QLatin1Char('-');
        }
        QTime offsetTime = QTime(0, 0).addSecs(abs(offsetSecs));
        if (version == VCard::v4_0) {
            str += offsetTime.toString(QStringLiteral("HHmm"));
        } else {
            str += offsetTime.toString(QStringLiteral("HH:mm"));
        }
    }
    return str;
}

QString VCardTool::createDate(const QDate &date, VCard::Version version)
{
    QString format;
    if (date.year() > 0) {
        format = QStringLiteral("yyyyMMdd");
    } else {
        format = QStringLiteral("--MMdd");
    }
    if (version != VCard::v4_0) {
        format.replace(QStringLiteral("yyyy"), QStringLiteral("yyyy-"));
        format.replace(QStringLiteral("MM"), QStringLiteral("MM-"));
    }
    return date.toString(format);
}

QString VCardTool::createTime(const QTime &time, VCard::Version version)
{
    QString format;
    if (version == VCard::v4_0) {
        format = QStringLiteral("HHmmss");
    } else {
        format = QStringLiteral("HH:mm:ss");
    }
    return QLatin1Char('T') + time.toString(format);
}

Picture VCardTool::parsePicture(const VCardLine &line) const
{
    Picture pic;

    const QStringList params = line.parameterList();
    QString type;
    if (params.contains(QLatin1String("type"))) {
        type = line.parameter(QStringLiteral("type"));
    }
    if (params.contains(QLatin1String("encoding"))) {
        pic.setRawData(line.value().toByteArray(), type);
    } else if (params.contains(QLatin1String("value"))) {
        if (line.parameter(QStringLiteral("value")).toLower() == QLatin1String("uri")) {
            pic.setUrl(line.value().toString());
        }
    }

    return pic;
}

VCardLine VCardTool::createPicture(const QString &identifier, const Picture &pic, VCard::Version version) const
{
    VCardLine line(identifier);

    if (pic.isEmpty()) {
        return line;
    }

    if (pic.isIntern()) {
        line.setValue(pic.rawData());
        if (version == VCard::v2_1) {
            line.addParameter(QStringLiteral("ENCODING"), QStringLiteral("BASE64"));
            line.addParameter(pic.type(), QString());
        } else { /*if (version == VCard::v3_0) */
            line.addParameter(QStringLiteral("encoding"), QStringLiteral("b"));
            line.addParameter(QStringLiteral("type"), pic.type());
#if 0
        } else { //version 4.0
            line.addParameter(QStringLiteral("data") + QStringLiteral(":image/") + pic.type(), QStringLiteral("base64"));
#endif
        }
    } else {
        line.setValue(pic.url());
        line.addParameter(QStringLiteral("value"), QStringLiteral("URI"));
    }

    return line;
}

Sound VCardTool::parseSound(const VCardLine &line) const
{
    Sound snd;

    const QStringList params = line.parameterList();
    if (params.contains(QLatin1String("encoding"))) {
        snd.setData(line.value().toByteArray());
    } else if (params.contains(QLatin1String("value"))) {
        if (line.parameter(QStringLiteral("value")).toLower() == QLatin1String("uri")) {
            snd.setUrl(line.value().toString());
        }
    }

    /* TODO: support sound types
      if ( params.contains( "type" ) )
        snd.setType( line.parameter( "type" ) );
    */

    return snd;
}

VCardLine VCardTool::createSound(const Sound &snd, VCard::Version version) const
{
    Q_UNUSED(version);
    VCardLine line(QStringLiteral("SOUND"));

    if (snd.isIntern()) {
        if (!snd.data().isEmpty()) {
            line.setValue(snd.data());
            if (version == VCard::v2_1) {
                line.addParameter(QStringLiteral("ENCODING"), QStringLiteral("BASE64"));
            } else {
                line.addParameter(QStringLiteral("encoding"), QStringLiteral("b"));
            }
            // TODO: need to store sound type!!!
        }
    } else if (!snd.url().isEmpty()) {
        line.setValue(snd.url());
        line.addParameter(QStringLiteral("value"), QStringLiteral("URI"));
    }

    return line;
}

Key VCardTool::parseKey(const VCardLine &line) const
{
    Key key;

    const QStringList params = line.parameterList();
    if (params.contains(QLatin1String("encoding"))) {
        key.setBinaryData(line.value().toByteArray());
    } else {
        key.setTextData(line.value().toString());
    }

    if (params.contains(QLatin1String("type"))) {
        if (line.parameter(QStringLiteral("type")).toLower() == QLatin1String("x509")) {
            key.setType(Key::X509);
        } else if (line.parameter(QStringLiteral("type")).toLower() == QLatin1String("pgp")) {
            key.setType(Key::PGP);
        } else {
            key.setType(Key::Custom);
            key.setCustomTypeString(line.parameter(QStringLiteral("type")));
        }
    } else if (params.contains(QLatin1String("mediatype"))) {
        const QString param = line.parameter(QStringLiteral("mediatype")).toLower();
        if (param == QLatin1String("application/x-x509-ca-cert")) {
            key.setType(Key::X509);
        } else if (param == QLatin1String("application/pgp-keys")) {
            key.setType(Key::PGP);
        } else {
            key.setType(Key::Custom);
            key.setCustomTypeString(line.parameter(QStringLiteral("type")));
        }
    }

    return key;
}

VCardLine VCardTool::createKey(const Key &key, VCard::Version version) const
{
    VCardLine line(QStringLiteral("KEY"));

    if (key.isBinary()) {
        if (!key.binaryData().isEmpty()) {
            line.setValue(key.binaryData());
            if (version == VCard::v2_1) {
                line.addParameter(QStringLiteral("ENCODING"), QStringLiteral("BASE64"));
            } else {
                line.addParameter(QStringLiteral("encoding"), QStringLiteral("b"));
            }
        }
    } else if (!key.textData().isEmpty()) {
        line.setValue(key.textData());
    }

    if (version == VCard::v4_0) {
        if (key.type() == Key::X509) {
            line.addParameter(QStringLiteral("MEDIATYPE"), QStringLiteral("application/x-x509-ca-cert"));
        } else if (key.type() == Key::PGP) {
            line.addParameter(QStringLiteral("MEDIATYPE"), QStringLiteral("application/pgp-keys"));
        } else if (key.type() == Key::Custom) {
            line.addParameter(QStringLiteral("MEDIATYPE"), key.customTypeString());
        }
    } else {
        if (key.type() == Key::X509) {
            line.addParameter(QStringLiteral("type"), QStringLiteral("X509"));
        } else if (key.type() == Key::PGP) {
            line.addParameter(QStringLiteral("type"), QStringLiteral("PGP"));
        } else if (key.type() == Key::Custom) {
            line.addParameter(QStringLiteral("type"), key.customTypeString());
        }
    }

    return line;
}

Secrecy VCardTool::parseSecrecy(const VCardLine &line) const
{
    Secrecy secrecy;

    const QString value = line.value().toString().toLower();
    if (value == QLatin1String("public")) {
        secrecy.setType(Secrecy::Public);
    } else if (value == QLatin1String("private")) {
        secrecy.setType(Secrecy::Private);
    } else if (value == QLatin1String("confidential")) {
        secrecy.setType(Secrecy::Confidential);
    }

    return secrecy;
}

VCardLine VCardTool::createSecrecy(const Secrecy &secrecy) const
{
    VCardLine line(QStringLiteral("CLASS"));

    int type = secrecy.type();

    if (type == Secrecy::Public) {
        line.setValue(QStringLiteral("PUBLIC"));
    } else if (type == Secrecy::Private) {
        line.setValue(QStringLiteral("PRIVATE"));
    } else if (type == Secrecy::Confidential) {
        line.setValue(QStringLiteral("CONFIDENTIAL"));
    }

    return line;
}

QStringList VCardTool::splitString(QChar sep, const QString &str) const
{
    QStringList list;
    QString value(str);

    int start = 0;
    int pos = value.indexOf(sep, start);

    while (pos != -1) {
        if (pos == 0 || value[pos - 1] != QLatin1Char('\\')) {
            if (pos > start && pos <= value.length()) {
                list << value.mid(start, pos - start);
            } else {
                list << QString();
            }

            start = pos + 1;
            pos = value.indexOf(sep, start);
        } else {
            value.replace(pos - 1, 2, sep);
            pos = value.indexOf(sep, pos);
        }
    }

    int l = value.length() - 1;
    const QString mid = value.mid(start, l - start + 1);
    if (!mid.isEmpty()) {
        list << mid;
    } else {
        list << QString();
    }

    return list;
}

QString VCardTool::normalizeImppServiceType(const QString &serviceType) const
{
    if (serviceType == QLatin1String("jabber")) {
        return QStringLiteral("xmpp");
    }
    if (serviceType == QLatin1String("yahoo")) {
        return QStringLiteral("ymsgr");
    }
    if (serviceType == QLatin1String("gadugadu")) {
        return QStringLiteral("gg");
    }
    return serviceType;
}
