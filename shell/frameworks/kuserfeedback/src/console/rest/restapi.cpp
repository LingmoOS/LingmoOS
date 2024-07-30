/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "restapi.h"
#include "restclient.h"

#include <core/product.h>
#include <core/sample.h>
#include <core/survey.h>

#include <QUuid>

using namespace KUserFeedback::Console;

QNetworkReply* RESTApi::checkSchema(RESTClient *client)
{
    return client->get(QStringLiteral("analytics/check_schema"));
}

QNetworkReply* RESTApi::listProducts(RESTClient *client)
{
    return client->get(QStringLiteral("analytics/products"));
}

QNetworkReply* RESTApi::createProduct(RESTClient *client, const Product &p)
{
    Q_ASSERT(p.isValid());
    return client->post(QStringLiteral("admin/products"),  p.toJson());
}

QNetworkReply* RESTApi::updateProduct(RESTClient *client, const Product &p)
{
    Q_ASSERT(p.isValid());
    return client->put(QStringLiteral("admin/products/") + p.name(), p.toJson());
}

QNetworkReply* RESTApi::deleteProduct(RESTClient *client, const Product &p)
{
    Q_ASSERT(p.isValid());
    return client->deleteResource(QStringLiteral("admin/products/") + p.name());
}

QNetworkReply* RESTApi::listSamples(RESTClient *client, const Product &p)
{
    Q_ASSERT(p.isValid());
    return client->get(QStringLiteral("analytics/data/") + p.name());
}

QNetworkReply* RESTApi::addSamples(RESTClient *client, const Product &p, const QVector<Sample> &samples)
{
    Q_ASSERT(p.isValid());
    return client->post(QStringLiteral("admin/data/") + p.name(), Sample::toJson(samples, p));
}

QNetworkReply* RESTApi::listSurveys(RESTClient *client, const Product &p)
{
    Q_ASSERT(p.isValid());
    return client->get(QStringLiteral("analytics/surveys/") + p.name());
}

QNetworkReply* RESTApi::createSurvey(RESTClient *client, const Product &p, const Survey &s)
{
    Q_ASSERT(p.isValid());
    return client->post(QStringLiteral("admin/surveys/") + p.name(), s.toJson());
}

QNetworkReply* RESTApi::updateSurvey(RESTClient *client, const Survey &s)
{
    return client->put(QStringLiteral("admin/surveys/") + s.uuid().toString(), s.toJson());
}

QNetworkReply* RESTApi::deleteSurvey(RESTClient *client, const Survey &s)
{
    return client->deleteResource(QStringLiteral("admin/surveys/") + s.uuid().toString());
}
