/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "addressformat_p.h"
#include "addressformatparser_p.h"

#include <QDebug>

using namespace KContacts;

struct {
    char c;
    AddressFormatField f;
} static constexpr const field_map[] = {
    {'A', AddressFormatField::StreetAddress},
    {'C', AddressFormatField::Locality},
    {'D', AddressFormatField::DependentLocality},
    {'N', AddressFormatField::Name},
    {'O', AddressFormatField::Organization},
    {'P', AddressFormatField::PostOfficeBox},
    {'R', AddressFormatField::Country},
    {'S', AddressFormatField::Region},
    {'X', AddressFormatField::SortingCode},
    {'Z', AddressFormatField::PostalCode},
};

AddressFormatField AddressFormatParser::parseField(QChar c)
{
    const auto it = std::lower_bound(std::begin(field_map), std::end(field_map), c.cell(), [](auto lhs, auto rhs) {
        return lhs.c < rhs;
    });
    if (it == std::end(field_map) || (*it).c != c.cell() || c.row() != 0) {
        return AddressFormatField::NoField;
    }
    return (*it).f;
}

AddressFormatFields AddressFormatParser::parseFields(QStringView s)
{
    AddressFormatFields fields;
    for (auto c : s) {
        fields |= parseField(c);
    }
    return fields;
}

std::vector<AddressFormatElement> AddressFormatParser::parseElements(QStringView s)
{
    std::vector<AddressFormatElement> elements;
    QString literal;
    for (auto it = s.begin(); it != s.end(); ++it) {
        if ((*it) == QLatin1Char('%') && std::next(it) != s.end()) {
            if (!literal.isEmpty()) {
                AddressFormatElement elem;
                auto e = AddressFormatElementPrivate::get(elem);
                e->literal = std::move(literal);
                elements.push_back(elem);
            }
            ++it;
            if ((*it) == QLatin1Char('n')) {
                elements.push_back(AddressFormatElement{});
            } else {
                const auto f = parseField(*it);
                if (f == AddressFormatField::NoField) {
                    qWarning() << "invalid format field element: %" << *it << "in" << s;
                } else {
                    AddressFormatElement elem;
                    auto e = AddressFormatElementPrivate::get(elem);
                    e->field = f;
                    elements.push_back(elem);
                }
            }
        } else {
            literal.push_back(*it);
        }
    }
    if (!literal.isEmpty()) {
        AddressFormatElement elem;
        auto e = AddressFormatElementPrivate::get(elem);
        e->literal = std::move(literal);
        elements.push_back(elem);
    }
    return elements;
}
