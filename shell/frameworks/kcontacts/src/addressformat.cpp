/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addressformat.h"
#include "address.h"
#include "addressformat_p.h"
#include "addressformatparser_p.h"
#include "addressformatscript_p.h"

#include <KConfig>
#include <KConfigGroup>
#include <KCountry>

#include <QDebug>
#include <QLocale>

using namespace KContacts;

AddressFormatElement::AddressFormatElement()
    : d(new AddressFormatElementPrivate)
{
}

AddressFormatElement::AddressFormatElement(const AddressFormatElement &) = default;
AddressFormatElement::~AddressFormatElement() = default;
AddressFormatElement &AddressFormatElement::operator=(const AddressFormatElement &) = default;

bool AddressFormatElement::isField() const
{
    return d->field != AddressFormatField::NoField;
}

AddressFormatField AddressFormatElement::field() const
{
    return d->field;
}

bool AddressFormatElement::isLiteral() const
{
    return !d->literal.isEmpty();
}

QString AddressFormatElement::literal() const
{
    return d->literal;
}

bool AddressFormatElement::isSeparator() const
{
    return !isField() && !isLiteral();
}

AddressFormat::AddressFormat()
    : d(new AddressFormatPrivate)
{
}

AddressFormat::AddressFormat(const AddressFormat &) = default;
AddressFormat::~AddressFormat() = default;
AddressFormat &AddressFormat::operator=(const AddressFormat &) = default;

const std::vector<AddressFormatElement> &AddressFormat::elements() const
{
    return d->elements;
}

AddressFormatFields AddressFormat::requiredFields() const
{
    return d->required;
}

AddressFormatFields AddressFormat::usedFields() const
{
    return std::accumulate(d->elements.begin(), d->elements.end(), AddressFormatFields(AddressFormatField::NoField), [](auto lhs, const auto &rhs) {
        return lhs | rhs.field();
    });
}

AddressFormatFields AddressFormat::upperCaseFields() const
{
    return d->upper;
}

QString AddressFormat::postalCodeRegularExpression() const
{
    return d->postalCodeFormat;
}

QString AddressFormat::country() const
{
    return d->country;
}

QList<AddressFormatElement> AddressFormat::elementsForQml() const
{
    QList<AddressFormatElement> l;
    l.reserve(d->elements.size());
    std::copy(d->elements.begin(), d->elements.end(), std::back_inserter(l));
    return l;
}

static QString addressFormatRc()
{
    Q_INIT_RESOURCE(kcontacts); // must be called outside of a namespace
    return QStringLiteral(":/org.kde.kcontacts/addressformatrc");
}

AddressFormat
AddressFormatRepository::formatForCountry(const QString &countryCode, AddressFormatScriptPreference scriptPref, AddressFormatPreference formatPref)
{
    static const KConfig entry(addressFormatRc(), KConfig::SimpleConfig);
    KConfigGroup group = entry.group(countryCode);

    AddressFormat format;
    auto fmt = AddressFormatPrivate::get(format);
    fmt->required = AddressFormatParser::parseFields(group.readEntry("Required", QString()));
    fmt->upper = AddressFormatParser::parseFields(group.readEntry("Upper", QString()));

    QString formatString;
    if (scriptPref == AddressFormatScriptPreference::Latin && formatPref == AddressFormatPreference::Business) {
        formatString = group.readEntry("LatinBusinessAddressFormat", QString());
    }
    if (formatString.isEmpty() && scriptPref == AddressFormatScriptPreference::Latin) {
        formatString = group.readEntry("LatinAddressFormat", QString());
    }
    if (formatString.isEmpty() && formatPref == AddressFormatPreference::Business) {
        formatString = group.readEntry("BusinessAddressFormat", QString());
    }
    if (formatString.isEmpty()) {
        formatString = group.readEntry("AddressFormat", QStringLiteral("%N%n%O%n%A%nPO BOX %P%n%C %S %Z"));
    }
    fmt->elements = AddressFormatParser::parseElements(formatString);
    fmt->postalCodeFormat = group.readEntry("PostalCodeFormat", QString());
    fmt->country = countryCode;
    return format;
}

AddressFormat AddressFormatRepository::formatForAddress(const Address &address, AddressFormatPreference formatPref)
{
    KCountry c;
    if (address.country().size() == 2) {
        c = KCountry::fromAlpha2(address.country());
    }
    if (!c.isValid()) {
        c = KCountry::fromName(address.country());
    }
    // fall back to our own country
    if (!c.isValid()) {
        c = KCountry::fromQLocale(QLocale().territory());
    }

    const auto scriptPref = AddressFormatScript::detect(address) == AddressFormatScript::LatinLikeScript ? AddressFormatScriptPreference::Latin
                                                                                                         : AddressFormatScriptPreference::Local;
    return formatForCountry(c.alpha2(), scriptPref, formatPref);
}

#include "moc_addressformat.cpp"
