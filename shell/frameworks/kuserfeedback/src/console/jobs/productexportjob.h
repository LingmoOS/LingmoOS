/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_PRODUCTEXPORTJOB_H
#define KUSERFEEDBACK_CONSOLE_PRODUCTEXPORTJOB_H

#include "job.h"
#include <rest/restclient.h>
#include <core/product.h>

namespace KUserFeedback {
namespace Console {

class ProductExportJob : public Job
{
    Q_OBJECT
public:
    explicit ProductExportJob(const QString &productId, const QString &destination, RESTClient *restClient, QObject *parent = nullptr);
    explicit ProductExportJob(const Product &product, const QString &destination, RESTClient *restClient, QObject *parent = nullptr);
    ~ProductExportJob() override;

private:
    void doExportSchema();
    void doExportSurveys();
    void doExportData();

    QString destination() const;

    Product m_product;
    QString m_dest;
    RESTClient *m_restClient;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_PRODUCTEXPORTJOB_H
