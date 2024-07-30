/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2023 Kai Uwe Broulik <kde@broulik.de>
    SPDX-License-Identifier: MIT
*/

#include "mecard.h"

#include <vector>

using namespace Prison;

class Prison::MeCardPrivate
{
public:
    QStringView header;
    struct Element {
        QStringView key;
        QStringList values;
        bool operator<(QStringView other) const;
    };
    std::vector<Element> elements;
};

bool MeCardPrivate::Element::operator<(QStringView other) const
{
    return key < other;
}

MeCard::MeCard()
    : d(new MeCardPrivate())
{
}

MeCard::MeCard(MeCard &&other) noexcept
    : d(std::move(other.d))
{
    other.d.reset();
}

MeCard &MeCard::operator=(MeCard &&other) noexcept
{
    std::swap(d, other.d);
    return *this;
}

MeCard::~MeCard() = default;

std::optional<MeCard> MeCard::parse(const QString &data)
{
    const auto idx = data.indexOf(QLatin1Char(':'));
    if (idx <= 0 || idx >= data.size() - 1) {
        return std::nullopt;
    }

    MeCard m;
    m.d->header = QStringView(data).left(idx);

    auto remaining = QStringView(data).mid(idx + 1);
    while (remaining.size() > 0) {
        const auto keyIdx = remaining.indexOf(QLatin1Char(':'));
        if (keyIdx <= 0 || keyIdx + 2 >= remaining.size()) {
            break;
        }

        QString value;
        auto elemIdx = keyIdx + 1;
        bool inQuote = false;
        for (; elemIdx < remaining.size() - 1; ++elemIdx) {
            auto c = remaining.at(elemIdx);
            if (elemIdx == (keyIdx + 1) && c == QLatin1Char('"')) { // leading quote
                inQuote = true;
                continue;
            }
            if (inQuote && c == QLatin1Char('"') && remaining.at(elemIdx + 1) == QLatin1Char(';')) { // trailing quote
                break;
            }
            if (c == QLatin1Char(';')) { // end of element
                break;
            }
            if (c == QLatin1Char('\\')) { // quoted character
                ++elemIdx;
                c = remaining.at(elemIdx);
            }
            value.push_back(c);
        }

        const auto key = remaining.left(keyIdx);
        auto it = std::lower_bound(m.d->elements.begin(), m.d->elements.end(), key);
        if (it == m.d->elements.end()) {
            m.d->elements.push_back(MeCardPrivate::Element());
            it = std::prev(m.d->elements.end());
        } else if ((*it).key != key) {
            it = m.d->elements.insert(it, MeCardPrivate::Element());
        }
        (*it).key = key;
        (*it).values.push_back(value);

        remaining = remaining.mid(elemIdx + 1);
    }

    if (m.d->elements.empty()) {
        return std::nullopt;
    }

    return m;
}

QString MeCard::header() const
{
    return d->header.toString();
}

QStringView MeCard::headerView() const
{
    return d->header;
}

QString MeCard::value(QStringView key) const
{
    const auto it = std::lower_bound(d->elements.begin(), d->elements.end(), key);
    if (it != d->elements.end() && (*it).key == key && (*it).values.size() == 1) {
        return (*it).values.at(0);
    }
    return {};
}

QStringList MeCard::values(QStringView key) const
{
    const auto it = std::lower_bound(d->elements.begin(), d->elements.end(), key);
    if (it != d->elements.end() && (*it).key == key) {
        return (*it).values;
    }
    return {};
}

QVariantMap MeCard::toVariantMap() const
{
    QVariantMap map;
    for (const auto &element : std::as_const(d->elements)) {
        if (element.values.size() > 1) {
            map.insert(element.key.toString(), element.values);
        } else {
            map.insert(element.key.toString(), element.values.at(0));
        }
    }
    return map;
}
