/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENT_H
#define KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENT_H

#include "schemaentry.h"
#include "schemaentryelement.h"

class QJsonArray;
class QJsonObject;

namespace KUserFeedback {
namespace Console {

class Product;

class AggregationElement
{
public:
    AggregationElement();
    ~AggregationElement();

    bool operator==(const AggregationElement &other) const;

    enum Type {
        Value,
        Size
    };
    Type type() const;
    void setType(Type t);

    bool isValid() const;

    SchemaEntry schemaEntry() const;
    void setSchemaEntry(const SchemaEntry &entry);

    SchemaEntryElement schemaEntryElement() const;
    void setSchemaEntryElement(const SchemaEntryElement &element);

    QString displayString() const;

    QJsonObject toJsonObject() const;
    static QVector<AggregationElement> fromJson(const Product &product, const QJsonArray &a);

private:
    SchemaEntry m_entry;
    SchemaEntryElement m_element;
    Type m_type = Value;
};

}
}

Q_DECLARE_TYPEINFO(KUserFeedback::Console::AggregationElement, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KUserFeedback::Console::AggregationElement)

#endif // KUSERFEEDBACK_CONSOLE_AGGREGATIONELEMENT_H
