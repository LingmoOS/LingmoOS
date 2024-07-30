/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SURVEYMODEL_H
#define KUSERFEEDBACK_CONSOLE_SURVEYMODEL_H

#include <core/product.h>
#include <core/survey.h>

#include <QAbstractTableModel>

namespace KUserFeedback {
namespace Console {

class RESTClient;

class SurveyModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Role {
        SurveyRole = Qt::UserRole + 1
    };

    explicit SurveyModel(QObject *parent = nullptr);
    ~SurveyModel() override;

    void setRESTClient(RESTClient *client);
    void setProduct(const Product &product);
    void reload();

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    RESTClient *m_restClient = nullptr;
    Product m_product;
    QVector<Survey> m_surveys;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_SURVEYMODEL_H
