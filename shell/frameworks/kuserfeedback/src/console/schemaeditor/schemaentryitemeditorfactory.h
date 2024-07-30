/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SCHEMAENTRYITEMEDITORFACTORY_H
#define KUSERFEEDBACK_CONSOLE_SCHEMAENTRYITEMEDITORFACTORY_H

#include <QItemEditorFactory>

#include <memory>

namespace KUserFeedback {
namespace Console {

class AggregationElementModel;
class Product;

class SchemaEntryItemEditorFactory : public QItemEditorFactory
{
public:
    SchemaEntryItemEditorFactory();
    ~SchemaEntryItemEditorFactory() override;

    void setProduct(const Product &product);

private:
    Q_DISABLE_COPY(SchemaEntryItemEditorFactory)
    std::unique_ptr<AggregationElementModel> m_elementModel;
};


}
}

#endif // KUSERFEEDBACK_CONSOLE_SCHEMAENTRYITEMEDITORFACTORY_H
