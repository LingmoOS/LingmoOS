/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "productimportjob.h"

#include <rest/restapi.h>
#include <rest/restclient.h>
#include <core/sample.h>
#include <core/survey.h>

#include <QDir>
#include <QNetworkReply>

using namespace KUserFeedback::Console;

ProductImportJob::ProductImportJob(const QString& source, RESTClient* restClient, QObject* parent)
    : Job(parent)
    , m_source(source)
    , m_restClient(restClient)
{
    Q_ASSERT(m_restClient->isConnected());
    doImportSchema();
}

ProductImportJob::~ProductImportJob() = default;

void ProductImportJob::doImportSchema()
{
    QDir source(m_source);
    Q_ASSERT(source.exists());

    QFile f(source.absoluteFilePath(source.dirName() + QLatin1String(".schema")));
    if (!f.open(QFile::ReadOnly)) {
        emitError(tr("Unable to open file: %1").arg(f.errorString()));
        return;
    }

    const auto products = Product::fromJson(f.readAll());
    if (products.size() != 1) {
        emitError(tr("Invalid product schema file."));
        return;
    }
    m_product = products.at(0);

    auto reply = RESTApi::createProduct(m_restClient, m_product);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            deleteLater();
            return;
        }
        doImportSurveys();
    });
}

void ProductImportJob::doImportSurveys()
{
    QDir source(m_source);
    QFile f(source.absoluteFilePath(source.dirName() + QLatin1String(".surveys")));
    if (!f.open(QFile::ReadOnly)) {
        doImportData();
        return;
    }

    const auto surveys = Survey::fromJson(f.readAll());
    if (surveys.isEmpty()) {
        doImportData();
        return;
    }

    for (const auto &s : surveys) {
        ++m_jobCount;
        auto reply = RESTApi::createSurvey(m_restClient, m_product, s);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            reply->deleteLater();
            --m_jobCount;
            if (reply->error() != QNetworkReply::NoError) {
                deleteLater();
                return;
            }
            if (m_jobCount == 0)
                doImportData();
        });
    }
}

void ProductImportJob::doImportData()
{
    QDir source(m_source);
    QFile f(source.absoluteFilePath(source.dirName() + QLatin1String(".data")));
    if (!f.open(QFile::ReadOnly)) {
        emitFinished();
        return;
    }

    const auto samples = Sample::fromJson(f.readAll(), m_product);
    if (samples.isEmpty()) {
        emitFinished();
        return;
    }

    auto reply = RESTApi::addSamples(m_restClient, m_product, samples);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError)
            emitFinished();
    });
}

#include "moc_productimportjob.cpp"
