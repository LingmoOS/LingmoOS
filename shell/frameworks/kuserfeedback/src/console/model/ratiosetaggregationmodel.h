/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_RATIOSETAGGREGATIONMODEL_H
#define KUSERFEEDBACK_CONSOLE_RATIOSETAGGREGATIONMODEL_H

#include <QAbstractTableModel>

namespace KUserFeedback {
namespace Console {

class RatioSetAggregationModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RatioSetAggregationModel(QObject *parent = nullptr);
    ~RatioSetAggregationModel() override;

    void setSourceModel(QAbstractItemModel *model);
    void setAggregationValue(const QString &aggrValue);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    void recompute();

    QAbstractItemModel *m_sourceModel = nullptr;
    QString m_aggrValue;
    QVector<QString> m_categories;
    double *m_data = nullptr;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_RATIOSETAGGREGATIONMODEL_H
