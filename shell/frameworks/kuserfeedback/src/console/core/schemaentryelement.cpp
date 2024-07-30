/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "schemaentryelement.h"
#include "util.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QSharedData>

using namespace KUserFeedback::Console;

namespace KUserFeedback {
namespace Console {
class SchemaEntryElementData : public QSharedData
{
public:
    QString name;
    SchemaEntryElement::Type type = SchemaEntryElement::String;
};

static const struct {
    SchemaEntryElement::Type type;
    const char *name;
} element_type_table[] {
    { SchemaEntryElement::Integer, "int" },
    { SchemaEntryElement::Number, "number" },
    { SchemaEntryElement::String, "string" },
    { SchemaEntryElement::Boolean, "bool" }
};

}
}

SchemaEntryElement::SchemaEntryElement() :
    d(new SchemaEntryElementData)
{
}

SchemaEntryElement::SchemaEntryElement(const SchemaEntryElement&) = default;
SchemaEntryElement::~SchemaEntryElement() = default;
SchemaEntryElement& SchemaEntryElement::operator=(const SchemaEntryElement&) = default;

bool SchemaEntryElement::operator==(const SchemaEntryElement& other) const
{
    return d->name == other.d->name
        && d->type == other.d->type;
}

QString SchemaEntryElement::name() const
{
    return d->name;
}

void SchemaEntryElement::setName(const QString& name)
{
    d->name = name;
}

SchemaEntryElement::Type SchemaEntryElement::type() const
{
    return d->type;
}

void SchemaEntryElement::setType(SchemaEntryElement::Type type)
{
    d->type = type;
}

QJsonObject SchemaEntryElement::toJsonObject() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("name"), d->name);
    obj.insert(QStringLiteral("type"), QLatin1String(element_type_table[d->type].name));
    return obj;
}

QVector<SchemaEntryElement> SchemaEntryElement::fromJson(const QJsonArray& array)
{
    QVector<SchemaEntryElement> res;
    res.reserve(array.size());

    foreach (const auto &v, array) {
        const auto obj = v.toObject();
        SchemaEntryElement e;
        e.setName(obj.value(QLatin1String("name")).toString());
        e.setType(Util::stringToEnum<SchemaEntryElement::Type>(obj.value(QLatin1String("type")).toString(), element_type_table));
        res.push_back(e);
    }

    return res;
}

#include "moc_schemaentryelement.cpp"
