/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addressformatter_p.h"

#include "address.h"
#include "addressformat.h"
#include "addressformat_p.h"
#include "addressformatscript_p.h"

#include <KCountry>

#include <QDebug>
#include <QStringList>

using namespace KContacts;

static constexpr auto AllFields = AddressFormatField::Country | AddressFormatField::Region | AddressFormatField::Locality
    | AddressFormatField::DependentLocality | AddressFormatField::SortingCode | AddressFormatField::PostalCode | AddressFormatField::StreetAddress
    | AddressFormatField::Organization | AddressFormatField::Name | AddressFormatField::PostOfficeBox;
static constexpr auto AllDomesticFields = AllFields & ~(int)AddressFormatField::Country;
static constexpr auto GeoUriFields = AddressFormatField::StreetAddress | AddressFormatField::PostalCode | AddressFormatField::Locality
    | AddressFormatField::DependentLocality | AddressFormatField::Region | AddressFormatField::Country;

enum Separator { Newline, Comma, Native };

// keep the same order as the enum!
struct {
    Separator separator;
    bool honorUpper;
    bool forceCountry;
    AddressFormatFields includeFields;
} static constexpr const style_map[] = {
    {Newline, true, true, AllDomesticFields}, // AddressFormatStyle::Postal
    {Newline, false, false, AllDomesticFields}, // AddressFormatStyle::MultiLineDomestic
    {Newline, false, true, AllFields}, // AddressFormatStyle::MultiLineInternational
    {Native, false, false, AllDomesticFields}, // AddressFormatStyle::SingleLineDomestic
    {Native, false, true, AllFields}, // AddressFormatStyle::SingleLineInternational
    {Comma, true, true, GeoUriFields}, // AddressFormatStyle::GeoUriQuery
};

static constexpr const char *separator_map[] = {"\n", ","};
static constexpr const char *native_separator_map[] = {", ", "، ", "", " "};

static bool isReverseOrder(const AddressFormat &fmt)
{
    return !fmt.elements().empty() && fmt.elements()[0].field() == AddressFormatField::Country;
}

QString
AddressFormatter::format(const Address &address, const QString &name, const QString &organization, const AddressFormat &format, AddressFormatStyle style)
{
    const auto styleData = style_map[(int)style];
    const auto isFieldEmpty = [&](AddressFormatField f) -> bool {
        if ((styleData.includeFields & f) == 0) {
            return true;
        }
        switch (f) {
        case AddressFormatField::NoField:
        case AddressFormatField::DependentLocality:
        case AddressFormatField::SortingCode:
            return true;
        case AddressFormatField::Name:
            return name.isEmpty();
        case AddressFormatField::Organization:
            return organization.isEmpty();
        case AddressFormatField::PostOfficeBox:
            return address.postOfficeBox().isEmpty();
        case AddressFormatField::StreetAddress:
            return address.street().isEmpty() && (address.extended().isEmpty() || style == AddressFormatStyle::GeoUriQuery);
        case AddressFormatField::PostalCode:
            return address.postalCode().isEmpty();
        case AddressFormatField::Locality:
            return address.locality().isEmpty();
        case AddressFormatField::Region:
            return address.region().isEmpty();
        case AddressFormatField::Country:
            return address.country().isEmpty();
        }
        return true;
    };
    const auto countryName = [&]() -> QString {
        if (address.country().isEmpty()) {
            return {};
        }
        // we use the already ISO 3166-1 resolved country from format here to
        // avoid a potentially expensive second name-based lookup
        return style == AddressFormatStyle::GeoUriQuery ? format.country() : KCountry::fromAlpha2(format.country()).name();
    };

    QStringList lines;
    QString line, secondaryLine;

    for (auto it = format.elements().begin(); it != format.elements().end(); ++it) {
        // add separators if:
        // - the preceding line is not empty
        // - we use newline separators and the preceding element is another separator
        const auto precedingSeparator = (it != format.elements().begin() && (*std::prev(it)).isSeparator());
        if ((*it).isSeparator() && (!line.isEmpty() || (precedingSeparator && styleData.separator == Newline))) {
            lines.push_back(line);
            line.clear();
            if (!secondaryLine.isEmpty()) {
                lines.push_back(secondaryLine);
                secondaryLine.clear();
            }
            continue;
        }

        // literals are only added if they not follow an empty field and are not preceding an empty field
        // to support incomplete addresses we deviate from the libaddressinput algorithm here and also add
        // the separator if any preceding field in the same line is non-empty, not just the immediate one.
        // this is to produce useful output e.g. for "%C %S %Z" if %S is empty.
        bool precedingFieldHasContent = (it == format.elements().begin() || (*std::prev(it)).isSeparator());
        for (auto it2 = it; !(*it2).isSeparator(); --it2) {
            if ((*it2).isField() && !isFieldEmpty((*it2).field())) {
                precedingFieldHasContent = true;
                break;
            }
            if (it2 == format.elements().begin()) {
                break;
            }
        }
        const auto followingFieldEmpty = (std::next(it) != format.elements().end() && (*std::next(it)).isField() && isFieldEmpty((*std::next(it)).field()));
        if ((*it).isLiteral() && precedingFieldHasContent && !followingFieldEmpty) {
            line += (*it).literal();
            continue;
        }

        if ((*it).isField() && (styleData.includeFields & (*it).field())) {
            QString v;
            switch ((*it).field()) {
            case AddressFormatField::NoField:
            case AddressFormatField::DependentLocality:
            case AddressFormatField::SortingCode:
                break;
            case AddressFormatField::Name:
                v = name;
                break;
            case AddressFormatField::Organization:
                v = organization;
                break;
            case AddressFormatField::PostOfficeBox:
                v = address.postOfficeBox();
                break;
            case AddressFormatField::StreetAddress:
                if (!address.street().isEmpty() && !address.extended().isEmpty() && style != AddressFormatStyle::GeoUriQuery) {
                    if (isReverseOrder(format)) {
                        secondaryLine = address.extended();
                    } else {
                        lines.push_back(address.extended());
                    }
                }
                v = address.street().isEmpty() ? address.extended() : address.street();
                break;
            case AddressFormatField::PostalCode:
                v = address.postalCode();
                break;
            case AddressFormatField::Locality:
                v = address.locality();
                break;
            case AddressFormatField::Region:
                v = address.region();
                break;
            case AddressFormatField::Country:
                v = countryName();
                break;
            }
            if (styleData.honorUpper && format.upperCaseFields() & (*it).field()) {
                v = v.toUpper();
            }
            line += v;
        }
    }
    if (!line.isEmpty()) {
        lines.push_back(line);
    }
    if (!secondaryLine.isEmpty()) {
        lines.push_back(secondaryLine);
    }

    // append country for formats that need it (international style + not yet present in format.elements())
    if (styleData.forceCountry && (format.usedFields() & AddressFormatField::Country & styleData.includeFields) == 0 && !address.country().isEmpty()) {
        auto c = countryName();
        if (style == AddressFormatStyle::Postal) {
            // the format of the country for postal addresses depends on the sending country, not the destination
            const auto sourceCountry = KCountry::fromQLocale(QLocale().territory());
            const auto sourceFmt = AddressFormatRepository::formatForCountry(sourceCountry.alpha2(), AddressFormatScriptPreference::Local);
            const auto shouldPrepend = isReverseOrder(sourceFmt);
            if (!lines.isEmpty()) {
                shouldPrepend ? lines.push_front({}) : lines.push_back({});
            }
            if (styleData.honorUpper && (sourceFmt.upperCaseFields() & AddressFormatField::Country)) {
                c = c.toUpper();
            }
            shouldPrepend ? lines.push_front(c) : lines.push_back(c);
        } else {
            if (styleData.honorUpper && (format.upperCaseFields() & AddressFormatField::Country)) {
                c = c.toUpper();
            }
            lines.push_back(c);
        }
    }

    if (styleData.separator == Native) {
        const auto script = AddressFormatScript::detect(address);
        return lines.join(QString::fromUtf8(native_separator_map[script]));
    }
    return lines.join(QLatin1String(separator_map[styleData.separator]));
}
