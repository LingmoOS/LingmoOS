/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_AGGREGATION_H
#define KUSERFEEDBACK_CONSOLE_AGGREGATION_H

#include <core/aggregationelement.h>

#include <QTypeInfo>

class QJsonArray;
class QJsonObject;

namespace KUserFeedback {
namespace Console {

class Product;

class Aggregation
{
    Q_GADGET
public:
    enum Type {
        None,
        Category,
        RatioSet,
        Numeric
    };
    Q_ENUM(Type)

    Aggregation();
    ~Aggregation();

    bool isValid() const;

    Type type() const;
    void setType(Type t);

    QString name() const;
    void setName(const QString &name);

    QVector<AggregationElement> elements() const;
    void setElements(const QVector<AggregationElement> &elements);

    QJsonObject toJsonObject() const;
    static QVector<Aggregation> fromJson(const Product &product, const QJsonArray &a);

private:
    Type m_type = None;
    QString m_name;
    QVector<AggregationElement> m_elements;
};

}
}

Q_DECLARE_TYPEINFO(KUserFeedback::Console::Aggregation, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KUserFeedback::Console::Aggregation::Type)

#endif // KUSERFEEDBACK_CONSOLE_AGGREGATION_H
