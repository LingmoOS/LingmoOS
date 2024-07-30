/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SCHEMAMODEL_H
#define KUSERFEEDBACK_CONSOLE_SCHEMAMODEL_H

#include <core/product.h>

#include <QAbstractTableModel>

namespace KUserFeedback {
namespace Console {

class Product;

class SchemaModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SchemaModel(QObject *parent = nullptr);
    ~SchemaModel() override;

    Product product() const;
    void setProduct(const Product &product);

    void addEntry(const QString &name);
    void addEntry(const SchemaEntry &entry);
    void addElement(const QModelIndex &parent, const QString &name);
    void deleteRow(const QModelIndex &idx);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

private:
    Product m_product;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_SCHEMAMODEL_H
