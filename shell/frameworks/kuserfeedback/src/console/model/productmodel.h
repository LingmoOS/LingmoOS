/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_PRODUCTMODEL_H
#define KUSERFEEDBACK_CONSOLE_PRODUCTMODEL_H

#include <core/product.h>
#include <rest/serverinfo.h>

#include <QAbstractListModel>
#include <QVector>

namespace KUserFeedback {
namespace Console {

class RESTClient;

/** Self-updating product model. */
class ProductModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ProductModel(QObject *parent = nullptr);
    ~ProductModel() override;

    enum Roles {
        ProductRole = Qt::UserRole + 1
    };

    void setRESTClient(RESTClient *client);
    void clear();
    void reload();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    void mergeProducts(QVector<Product> &&products);

    RESTClient *m_restClient = nullptr;
    QVector<Product> m_products;
};
}
}

#endif // KUSERFEEDBACK_CONSOLE_PRODUCTMODEL_H
