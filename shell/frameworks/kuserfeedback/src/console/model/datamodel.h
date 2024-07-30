/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_DATAMODEL_H
#define KUSERFEEDBACK_CONSOLE_DATAMODEL_H

#include <core/product.h>
#include <core/schemaentry.h>
#include <core/schemaentryelement.h>

#include <QAbstractTableModel>

namespace KUserFeedback {
namespace Console {

class RESTClient;
class Sample;

/** Raw data model showing reported data of a product. */
class DataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum {
        SampleRole = Qt::UserRole + 1,
        AllSamplesRole
    };

    explicit DataModel(QObject *parent = nullptr);
    ~DataModel() override;

    void setRESTClient(RESTClient *client);

    Product product() const;
    void setProduct(const Product &product);

    void setSamples(const QVector<Sample> &samples);
    void reload();

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    Product m_product;
    struct Column {
        SchemaEntry entry;
        SchemaEntryElement element;

        QString name() const;
    };
    QVector<Column> m_columns;
    RESTClient *m_restClient = nullptr;
    QVector<Sample> m_data;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_DATAMODEL_H
