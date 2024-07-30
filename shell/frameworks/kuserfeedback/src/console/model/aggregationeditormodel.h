/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_AGGREGATIONEDITORMODEL_H
#define KUSERFEEDBACK_CONSOLE_AGGREGATIONEDITORMODEL_H

#include <core/product.h>

#include <QAbstractTableModel>

namespace KUserFeedback {
namespace Console {

class AggregationEditorModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AggregationEditorModel(QObject *parent = nullptr);
    ~AggregationEditorModel() override;

    Product product() const;
    void setProduct(const Product &product);

    int columnCount(const QModelIndex & parent) const override;
    int rowCount(const QModelIndex & parent) const override;
    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex & index) const override;
    bool setData(const QModelIndex & index, const QVariant & value, int role) override;

private:
    Product m_product;
};
}
}

#endif // KUSERFEEDBACK_CONSOLE_AGGREGATIONEDITORMODEL_H
