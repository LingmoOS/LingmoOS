/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_PRODUCT_H
#define KUSERFEEDBACK_CONSOLE_PRODUCT_H

#include "schemaentry.h"

#include <QMetaType>
#include <QSharedDataPointer>
#include <QVector>

class QString;

namespace KUserFeedback {
namespace Console {

class Aggregation;
class ProductData;

/** Product data. */
class Product
{
public:
    Product();
    Product(const Product&);
    ~Product();
    Product& operator=(const Product&);

    bool isValid() const;

    QString name() const;
    void setName(const QString &name);

    QVector<SchemaEntry> schema() const;
    void setSchema(const QVector<SchemaEntry>& schema);
    SchemaEntry schemaEntry(const QString &name) const;

    QVector<Aggregation> aggregations() const;
    void setAggregations(const QVector<Aggregation> &aggregations);

    void addTemplate(const Product &tpl);

    QByteArray toJson() const;
    static QVector<Product> fromJson(const QByteArray &data);

private:
    QSharedDataPointer<ProductData> d;

};

}
}

Q_DECLARE_TYPEINFO(KUserFeedback::Console::Product, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(KUserFeedback::Console::Product)

#endif // KUSERFEEDBACK_CONSOLE_PRODUCT_H
