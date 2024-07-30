/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "schemaentryitemeditorfactory.h"

#include <core/aggregation.h>
#include <core/schemaentry.h>
#include <core/schemaentryelement.h>
#include <model/aggregationelementmodel.h>
#include <widgets/metaenumcombobox.h>

#include <QDebug>

using namespace KUserFeedback::Console;

namespace KUserFeedback {
namespace Console {

class AggregationElementComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue USER true)
public:
    explicit AggregationElementComboBox(QWidget *parent = nullptr) :
        QComboBox(parent)
    {}

    QVariant value() const
    {
        return currentData(Qt::EditRole);
    }

    void setValue(const QVariant &value)
    {
        const auto idxs = model()->match(model()->index(0, 0), Qt::EditRole, value, 1, Qt::MatchExactly);
        if (idxs.isEmpty())
            return;
        setCurrentIndex(idxs.at(0).row());
    }
};

class AggregationElementEditorCreator : public QStandardItemEditorCreator<AggregationElementComboBox>
{
public:
    explicit AggregationElementEditorCreator(AggregationElementModel *model) :
        m_model(model)
    {}

    QWidget* createWidget(QWidget *parent) const override
    {
        auto w = QStandardItemEditorCreator<AggregationElementComboBox>::createWidget(parent);
        qobject_cast<QComboBox*>(w)->setModel(m_model);
        return w;
    }

private:
    Q_DISABLE_COPY(AggregationElementEditorCreator)
    AggregationElementModel *m_model;
};

}
}

SchemaEntryItemEditorFactory::SchemaEntryItemEditorFactory() :
    m_elementModel(new AggregationElementModel)
{
   registerEditor(qMetaTypeId<SchemaEntry::DataType>(), new QStandardItemEditorCreator<MetaEnumComboBox>());
   registerEditor(qMetaTypeId<SchemaEntryElement::Type>(), new QStandardItemEditorCreator<MetaEnumComboBox>());
   registerEditor(qMetaTypeId<Aggregation::Type>(), new QStandardItemEditorCreator<MetaEnumComboBox>());
   registerEditor(qMetaTypeId<AggregationElement>(), new AggregationElementEditorCreator(m_elementModel.get()));
}

SchemaEntryItemEditorFactory::~SchemaEntryItemEditorFactory() = default;

void SchemaEntryItemEditorFactory::setProduct(const Product& product)
{
    m_elementModel->setProduct(product);
}

#include "schemaentryitemeditorfactory.moc"
