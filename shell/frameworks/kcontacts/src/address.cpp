/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "address.h"
#include "addressformat.h"
#include "addressformatter_p.h"

#include "kcontacts_debug.h"
#include <KConfig>
#include <KCountry>
#include <KLocalizedString>
#include <krandom.h>

#include <KConfigGroup>

#include <QDataStream>
#include <QSharedData>
#include <QUrlQuery>

using namespace KContacts;

class Q_DECL_HIDDEN Address::Private : public QSharedData
{
public:
    Private()
        : mEmpty(true)
    {
        mId = KRandom::randomString(10);
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        mEmpty = other.mEmpty;
        mId = other.mId;
        mType = other.mType;

        mPostOfficeBox = other.mPostOfficeBox;
        mExtended = other.mExtended;
        mStreet = other.mStreet;
        mLocality = other.mLocality;
        mRegion = other.mRegion;
        mPostalCode = other.mPostalCode;
        mCountry = other.mCountry;
        mLabel = other.mLabel;
    }

    bool mEmpty;
    QString mId;
    Type mType;
    Geo mGeo;

    QString mPostOfficeBox;
    QString mExtended;
    QString mStreet;
    QString mLocality;
    QString mRegion;
    QString mPostalCode;
    QString mCountry;
    QString mLabel;
};

Address::Address()
    : d(new Private)
{
}

Address::Address(Type type)
    : d(new Private)
{
    d->mType = type;
}

Address::Address(const Address &other)
    : d(other.d)
{
}

Address::~Address()
{
}

Address &Address::operator=(const Address &other)
{
    if (this != &other) {
        d = other.d;
    }

    return *this;
}

bool Address::operator==(const Address &other) const
{
    if (d->mId != other.d->mId) {
        return false;
    }
    if (d->mType != other.d->mType) {
        return false;
    }
    if (d->mPostOfficeBox != other.d->mPostOfficeBox) {
        return false;
    }
    if (d->mExtended != other.d->mExtended) {
        return false;
    }
    if (d->mStreet != other.d->mStreet) {
        return false;
    }
    if (d->mLocality != other.d->mLocality) {
        return false;
    }
    if (d->mRegion != other.d->mRegion) {
        return false;
    }
    if (d->mPostalCode != other.d->mPostalCode) {
        return false;
    }
    if (d->mCountry != other.d->mCountry) {
        return false;
    }
    if (d->mLabel != other.d->mLabel) {
        return false;
    }

    if (d->mGeo != other.d->mGeo) {
        return false;
    }

    return true;
}

bool Address::operator!=(const Address &a) const
{
    return !(a == *this);
}

bool Address::isEmpty() const
{
    return d->mEmpty;
}

void Address::clear()
{
    *this = Address();
}

void Address::setId(const QString &id)
{
    d->mEmpty = false;
    d->mId = id;
}

QString Address::id() const
{
    return d->mId;
}

void Address::setType(Type type)
{
    d->mEmpty = false;
    d->mType = type;
}

Address::Type Address::type() const
{
    return d->mType;
}

QString Address::typeLabel(Type type)
{
    QString label;
    const TypeList list = typeList();

    for (const auto typeFlag : list) {
        // these are actually flags
        const TypeFlag flag = static_cast<TypeFlag>(static_cast<int>(typeFlag));
        if (type & flag) {
            label.append(QLatin1Char('/') + typeFlagLabel(flag));
        }
    }

    // Remove the first '/'
    if (!label.isEmpty()) {
        label.remove(0, 1);
    }

    return label;
}

QString Address::typeLabel() const
{
    QString label;
    const TypeList list = typeList();

    for (const auto f : list) {
        if ((type() & f) && (f != Pref)) {
            label.append(QLatin1Char('/') + typeLabel(f));
        }
    }
    // Remove the first '/'
    if (!label.isEmpty()) {
        label.remove(0, 1);
    }
    return label;
}

void Address::setPostOfficeBox(const QString &postOfficeBox)
{
    d->mEmpty = false;
    d->mPostOfficeBox = postOfficeBox;
}

QString Address::postOfficeBox() const
{
    return d->mPostOfficeBox;
}

QString Address::postOfficeBoxLabel()
{
    return i18n("Post Office Box");
}

void Address::setExtended(const QString &extended)
{
    d->mEmpty = false;
    d->mExtended = extended;
}

QString Address::extended() const
{
    return d->mExtended;
}

QString Address::extendedLabel()
{
    return i18n("Extended Address Information");
}

void Address::setStreet(const QString &street)
{
    d->mEmpty = false;
    d->mStreet = street;
}

QString Address::street() const
{
    return d->mStreet;
}

QString Address::streetLabel()
{
    return i18n("Street");
}

void Address::setLocality(const QString &locality)
{
    d->mEmpty = false;
    d->mLocality = locality;
}

QString Address::locality() const
{
    return d->mLocality;
}

QString Address::localityLabel()
{
    return i18n("Locality");
}

void Address::setRegion(const QString &region)
{
    d->mEmpty = false;
    d->mRegion = region;
}

QString Address::region() const
{
    return d->mRegion;
}

QString Address::regionLabel()
{
    return i18n("Region");
}

void Address::setPostalCode(const QString &postalCode)
{
    d->mEmpty = false;
    d->mPostalCode = postalCode;
}

QString Address::postalCode() const
{
    return d->mPostalCode;
}

QString Address::postalCodeLabel()
{
    return i18n("Postal Code");
}

void Address::setCountry(const QString &country)
{
    d->mEmpty = false;
    d->mCountry = country;
}

QString Address::country() const
{
    return d->mCountry;
}

QString Address::countryLabel()
{
    return i18n("Country");
}

void Address::setLabel(const QString &label)
{
    d->mEmpty = false;
    d->mLabel = label;
}

QString Address::label() const
{
    return d->mLabel;
}

QString Address::labelLabel()
{
    return i18n("Delivery Label");
}

Address::TypeList Address::typeList()
{
    static TypeList list;

    if (list.isEmpty()) {
        list << Dom << Intl << Postal << Parcel << Home << Work << Pref;
    }

    return list;
}

QString Address::typeFlagLabel(TypeFlag type)
{
    switch (type) {
    case Dom:
        return i18nc("Address is in home country", "Domestic");
    case Intl:
        return i18nc("Address is not in home country", "International");
    case Postal:
        return i18nc("Address for delivering letters", "Postal");
    case Parcel:
        return i18nc("Address for delivering packages", "Parcel");
    case Home:
        return i18nc("Home Address", "Home");
    case Work:
        return i18nc("Work Address", "Work");
    case Pref:
        return i18n("Preferred Address");
    }
    return i18nc("another type of address", "Other");
}

void Address::setGeo(const Geo &geo)
{
    d->mEmpty = false;
    d->mGeo = geo;
}

Geo Address::geo() const
{
    return d->mGeo;
}

QString Address::toString() const
{
    QString str = QLatin1String("Address {\n");
    str += QStringLiteral("  IsEmpty: %1\n").arg(d->mEmpty ? QStringLiteral("true") : QStringLiteral("false"));
    str += QStringLiteral("  Id: %1\n").arg(d->mId);
    str += QStringLiteral("  Type: %1\n").arg(typeLabel(d->mType));
    str += QStringLiteral("  Post office box: %1\n").arg(d->mPostOfficeBox);
    str += QStringLiteral("  Extended: %1\n").arg(d->mExtended);
    str += QStringLiteral("  Street: %1\n").arg(d->mStreet);
    str += QStringLiteral("  Locality: %1\n").arg(d->mLocality);
    str += QStringLiteral("  Region: %1\n").arg(d->mRegion);
    str += QStringLiteral("  Postal code: %1\n").arg(d->mPostalCode);
    str += QStringLiteral("  Country: %1\n").arg(d->mCountry);
    str += QStringLiteral("  Label: %1\n").arg(d->mLabel);
    str += QStringLiteral("  Geo: %1\n").arg(d->mGeo.toString());
    str += QLatin1String("}\n");

    return str;
}

QString Address::formatted(AddressFormatStyle style, const QString &realName, const QString &orgaName) const
{
    const auto formatPref = (orgaName.isEmpty() || style != AddressFormatStyle::Postal) ? AddressFormatPreference::Generic : AddressFormatPreference::Business;
    const auto format = AddressFormatRepository::formatForAddress(*this, formatPref);
    return AddressFormatter::format(*this, realName, orgaName, format, style);
}

QString Address::formattedPostalAddress() const
{
    return formatted(AddressFormatStyle::Postal);
}

QUrl Address::geoUri() const
{
    QUrl url;
    url.setScheme(QStringLiteral("geo"));

    if (geo().isValid()) {
        url.setPath(QString::number(geo().latitude()) + QLatin1Char(',') + QString::number(geo().longitude()));
        return url;
    }

    if (!isEmpty()) {
        url.setPath(QStringLiteral("0,0"));
        QUrlQuery query;
        query.addQueryItem(QStringLiteral("q"), formatted(KContacts::AddressFormatStyle::GeoUriQuery));
        url.setQuery(query);
        return url;
    }

    return {};
}

// clang-format off
QDataStream &KContacts::operator<<(QDataStream &s, const Address &addr)
{
    return s << addr.d->mId << (uint)addr.d->mType << addr.d->mPostOfficeBox
             << addr.d->mExtended << addr.d->mStreet << addr.d->mLocality
             << addr.d->mRegion << addr.d->mPostalCode << addr.d->mCountry
             << addr.d->mLabel << addr.d->mEmpty << addr.d->mGeo;
}

QDataStream &KContacts::operator>>(QDataStream &s, Address &addr)
{
    uint type;
    s >> addr.d->mId >> type >> addr.d->mPostOfficeBox >> addr.d->mExtended
    >> addr.d->mStreet >> addr.d->mLocality >> addr.d->mRegion
    >> addr.d->mPostalCode >> addr.d->mCountry >> addr.d->mLabel
    >> addr.d->mEmpty >> addr.d->mGeo;

    addr.d->mType = Address::Type(type);

    return s;
}
// clang-format on

#include "moc_address.cpp"
