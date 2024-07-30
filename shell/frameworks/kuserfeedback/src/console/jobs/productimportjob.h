/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_PRODUCTIMPORTJOB_H
#define KUSERFEEDBACK_CONSOLE_PRODUCTIMPORTJOB_H

#include "job.h"

#include <core/product.h>

namespace KUserFeedback {
namespace Console {

class RESTClient;

class ProductImportJob : public Job
{
    Q_OBJECT
public:
    explicit ProductImportJob(const QString &source, RESTClient *restClient, QObject *parent = nullptr);
    ~ProductImportJob() override;

private:
    void doImportSchema();
    void doImportSurveys();
    void doImportData();

    Product m_product;
    QString m_source;
    RESTClient *m_restClient;
    int m_jobCount = 0;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_PRODUCTIMPORTJOB_H
