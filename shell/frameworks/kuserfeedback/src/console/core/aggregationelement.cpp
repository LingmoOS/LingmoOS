/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "aggregationelement.h"
#include "product.h"
#include "util.h"

#include <QJsonArray>
#include <QJsonObject>

using namespace KUserFeedback::Console;

static const struct {
    AggregationElement::Type type;
    const char *name;
} aggregation_element_types_table[] {
    { AggregationElement::Value, "value" },
    { AggregationElement::Size, "size" }
};

AggregationElement::AggregationElement() = default;
AggregationElement::~AggregationElement() = default;

bool AggregationElement::isValid() const
{
    return !m_entry.name().isEmpty();
}

SchemaEntry AggregationElement::schemaEntry() const
{
    return m_entry;
}

void AggregationElement::setSchemaEntry(const SchemaEntry& entry)
{
    m_entry = entry;
}

SchemaEntryElement AggregationElement::schemaEntryElement() const
{
    return m_element;
}

void AggregationElement::setSchemaEntryElement(const SchemaEntryElement& element)
{
    m_element = element;
}

AggregationElement::Type AggregationElement::type() const
{
    return m_type;
}

void AggregationElement::setType(AggregationElement::Type t)
{
    m_type = t;
}

QString AggregationElement::displayString() const
{
    switch (m_type) {
        case Value:
           return m_entry.name() + QLatin1Char('.') + m_element.name();
        case Size:
            return m_entry.name() + QLatin1String("[size]");
    }
    Q_UNREACHABLE();
}

bool AggregationElement::operator==(const AggregationElement &other) const
{
    if (m_type != other.m_type)
        return false;

    switch (m_type) {
        case Value:
            return m_element.name() == other.m_element.name() && m_entry.name() == other.m_entry.name();
        case Size:
            return m_element.name() == other.m_element.name();
    }
    Q_UNREACHABLE();
}

QJsonObject AggregationElement::toJsonObject() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("type"), QLatin1String(aggregation_element_types_table[m_type].name));
    switch (m_type) {
        case Value:
            obj.insert(QStringLiteral("schemaEntry"), m_entry.name());
            obj.insert(QStringLiteral("schemaEntryElement"), m_element.name());
            break;
        case Size:
            obj.insert(QStringLiteral("schemaEntry"), m_entry.name());
            break;
    }
    return obj;
}

QVector<AggregationElement> AggregationElement::fromJson(const Product &product, const QJsonArray& a)
{
    QVector<AggregationElement> elems;
    elems.reserve(a.size());
    for (const auto &v : a) {
        if (!v.isObject())
            continue;
        const auto obj = v.toObject();

        AggregationElement e;
        e.setType(Util::stringToEnum<AggregationElement::Type>(obj.value(QLatin1String("type")).toString(), aggregation_element_types_table));
        switch (e.type()) {
            case Value:
                e.setSchemaEntry(product.schemaEntry(obj.value(QLatin1String("schemaEntry")).toString()));
                e.setSchemaEntryElement(e.schemaEntry().element(obj.value(QLatin1String("schemaEntryElement")).toString()));
                break;
            case Size:
                e.setSchemaEntry(product.schemaEntry(obj.value(QLatin1String("schemaEntry")).toString()));
                break;
        }
        elems.push_back(e);
    }
    return elems;
}
