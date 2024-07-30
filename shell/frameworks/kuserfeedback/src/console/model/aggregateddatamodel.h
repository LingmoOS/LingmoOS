/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_AGGREGATEDDATAMODEL_H
#define KUSERFEEDBACK_CONSOLE_AGGREGATEDDATAMODEL_H

#include <QAbstractTableModel>
#include <QVector>

namespace KUserFeedback {
namespace Console {

/** Joint model for all time-aggregated data, for user display and export. */
class AggregatedDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit AggregatedDataModel(QObject *parent = nullptr);
    ~AggregatedDataModel() override;

    void addSourceModel(QAbstractItemModel *model, const QString &prefix = QString());
    void clear();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void recreateColumnMapping();

    QVector<QAbstractItemModel*> m_models;
    QVector<QString> m_prefixes;
    QVector<int> m_columnMapping;
    QVector<int> m_columnOffset;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_AGGREGATEDDATAMODEL_H
