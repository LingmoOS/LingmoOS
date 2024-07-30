/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "schemaentry.h"
#include "schemaentryelement.h"
#include "util.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QSharedData>

using namespace KUserFeedback::Console;

namespace KUserFeedback {
namespace Console {

class SchemaEntryData : public QSharedData
{
public:
    QString name;
    int internalId = -1;
    SchemaEntry::DataType dataType = SchemaEntry::Scalar;
    QVector<SchemaEntryElement> elements;
};

static const struct {
    SchemaEntry::DataType type;
    const char *name;
} data_types_table[] {
    { SchemaEntry::Scalar, "scalar" },
    { SchemaEntry::List, "list" },
    { SchemaEntry::Map, "map" }
};

}
}

SchemaEntry::SchemaEntry() : d(new SchemaEntryData) {}
SchemaEntry::SchemaEntry(const SchemaEntry&) = default;
SchemaEntry::~SchemaEntry() = default;
SchemaEntry& SchemaEntry::operator=(const SchemaEntry&) = default;

bool SchemaEntry::operator==(const SchemaEntry &other) const
{
    return d->name == other.d->name
        && d->internalId == other.d->internalId
        && d->dataType == other.d->dataType
        && d->elements == other.d->elements;
}

int SchemaEntry::internalId() const
{
    return d->internalId;
}

void SchemaEntry::setInternalId(int internalId)
{
    d->internalId = internalId;
}

QString SchemaEntry::name() const
{
    return d->name;
}

void SchemaEntry::setName(const QString& name)
{
    d->name = name;
}

SchemaEntry::DataType SchemaEntry::dataType() const
{
    return d->dataType;
}

void SchemaEntry::setDataType(SchemaEntry::DataType type)
{
    d->dataType = type;
}

QVector<SchemaEntryElement> SchemaEntry::elements() const
{
    return d->elements;
}

void SchemaEntry::setElements(const QVector<SchemaEntryElement> &elements)
{
    d->elements = elements;
}

SchemaEntryElement SchemaEntry::element(const QString& name) const
{
    const auto it = std::find_if(d->elements.cbegin(), d->elements.cend(), [name](const auto &entry) {
        return entry.name() == name;
    });
    if (it == d->elements.cend())
        return {};
    return *it;
}

QJsonObject SchemaEntry::toJsonObject() const
{
    QJsonObject obj;
    if (d->internalId >= 0)
        obj.insert(QStringLiteral("id"), d->internalId);
    obj.insert(QStringLiteral("name"), d->name);
    obj.insert(QStringLiteral("type"), QLatin1String(data_types_table[d->dataType].name));

    QJsonArray array;
    for (const auto &element : qAsConst(d->elements))
        array.push_back(element.toJsonObject());
    obj.insert(QStringLiteral("elements"), array);
    return obj;
}

QVector<SchemaEntry> SchemaEntry::fromJson(const QJsonArray &array)
{
    QVector<SchemaEntry> res;
    res.reserve(array.size());

    foreach (const auto &v, array) {
        const auto obj = v.toObject();
        SchemaEntry entry;
        entry.setName(obj.value(QStringLiteral("name")).toString());
        entry.setDataType(Util::stringToEnum<DataType>(obj.value(QLatin1String("type")).toString(), data_types_table));
        entry.setElements(SchemaEntryElement::fromJson(obj.value(QLatin1String("elements")).toArray()));

        res.push_back(entry);
    }

    return res;
}

#include "moc_schemaentry.cpp"
