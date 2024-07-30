/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "surveymodel.h"

#include <rest/restapi.h>
#include <rest/restclient.h>

#include <QDebug>
#include <QNetworkReply>
#include <QUrl>

using namespace KUserFeedback::Console;

SurveyModel::SurveyModel(QObject *parent) : QAbstractTableModel(parent)
{
}

SurveyModel::~SurveyModel() = default;

void SurveyModel::setRESTClient(RESTClient* client)
{
    Q_ASSERT(client);
    m_restClient = client;
    connect(client, &RESTClient::clientConnected, this, &SurveyModel::reload);
    reload();
}

void SurveyModel::setProduct(const Product& product)
{
    m_product = product;
    reload();
}

void SurveyModel::reload()
{
    if (!m_restClient || !m_restClient->isConnected() || !m_product.isValid())
        return;

    auto reply = RESTApi::listSurveys(m_restClient, m_product);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            beginResetModel();
            const auto data = reply->readAll();
            m_surveys = Survey::fromJson(data);
            endResetModel();
        }
        reply->deleteLater();
    });
}

int SurveyModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 4;
}

int SurveyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_surveys.size();
}

QVariant SurveyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return {};

    if (role == Qt::DisplayRole) {
        const auto survey = m_surveys.at(index.row());
        switch (index.column()) {
            case 0: return survey.name();
            case 1: return survey.url().toString();
            case 3: return survey.target();
        }
    } else if (role == Qt::CheckStateRole) {
        if (index.column() == 2)
            return  m_surveys.at(index.row()).isActive() ? Qt::Checked : Qt::Unchecked;
    } else if (role == SurveyRole) {
        return QVariant::fromValue(m_surveys.at(index.row()));
    }
    return {};
}

bool SurveyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() == 2 && role == Qt::CheckStateRole) {
        auto &survey = m_surveys[index.row()];
        survey.setActive(value.toInt() == Qt::Checked);
        auto reply = RESTApi::updateSurvey(m_restClient, survey);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            qDebug() << reply->readAll();
            reload();
        });
        Q_EMIT dataChanged(index, index);
        reply->deleteLater();
        return true;
    }
    return false;
}

QVariant SurveyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0: return tr("Name");
            case 1: return tr("URL");
            case 2: return tr("Active");
            case 3: return tr("Target");
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags SurveyModel::flags(const QModelIndex &index) const
{
    auto f = QAbstractTableModel::flags(index);
    if (index.column() == 2)
        return f | Qt::ItemIsUserCheckable;
    return f;
}

#include "moc_surveymodel.cpp"
