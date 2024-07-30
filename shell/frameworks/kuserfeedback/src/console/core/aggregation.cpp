/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "aggregation.h"
#include "util.h"

#include <QJsonArray>
#include <QJsonObject>

using namespace KUserFeedback::Console;

static const struct {
    Aggregation::Type type;
    const char *name;
} aggregation_types_table[] {
    { Aggregation::None, "none" },
    { Aggregation::Category, "category" },
    { Aggregation::RatioSet, "ratio_set" },
    { Aggregation::Numeric, "numeric" }
};

Aggregation::Aggregation() = default;
Aggregation::~Aggregation() = default;

bool Aggregation::isValid() const
{
    return m_type != None && m_elements.size() > 0;
}

Aggregation::Type Aggregation::type() const
{
    return m_type;
}

void Aggregation::setType(Aggregation::Type t)
{
    m_type = t;
}

QString Aggregation::name() const
{
    return m_name;
}

void Aggregation::setName(const QString& name)
{
    m_name = name;
}

QVector<AggregationElement> Aggregation::elements() const
{
    return m_elements;
}

void Aggregation::setElements(const QVector<AggregationElement>& elements)
{
    m_elements = elements;
}

QJsonObject Aggregation::toJsonObject() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("type"), QLatin1String(aggregation_types_table[m_type].name));
    obj.insert(QStringLiteral("name"), m_name);
    QJsonArray elems;
    for (const auto &e : m_elements)
        elems.push_back(e.toJsonObject());
    obj.insert(QStringLiteral("elements"), elems);
    return obj;
}

QVector<Aggregation> Aggregation::fromJson(const Product &product, const QJsonArray& a)
{
    QVector<Aggregation> aggrs;
    aggrs.reserve(a.size());
    for (const auto &v : a) {
        if (!v.isObject())
            continue;
        const auto obj = v.toObject();

        Aggregation aggr;
        aggr.setType(Util::stringToEnum<Aggregation::Type>(obj.value(QLatin1String("type")).toString(), aggregation_types_table));
        aggr.setName(obj.value(QLatin1String("name")).toString());
        aggr.setElements(AggregationElement::fromJson(product, obj.value(QLatin1String("elements")).toArray()));
        aggrs.push_back(aggr);
    }
    return aggrs;
}

#include "moc_aggregation.cpp"
