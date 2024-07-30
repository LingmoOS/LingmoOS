/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_SECURITYSCANJOB_H
#define KUSERFEEDBACK_CONSOLE_SECURITYSCANJOB_H

#include "job.h"

#include <QVector>

namespace KUserFeedback {
namespace Console {

class RESTClient;

/*! Scan a server for common misconfigurations. */
class SecurityScanJob : public Job
{
    Q_OBJECT
public:
    explicit SecurityScanJob(RESTClient *restClient, QObject *parent = nullptr);
    ~SecurityScanJob() override;

private:
    void processPending();

    RESTClient *m_restClient;
    QVector<QString> m_pendingPaths;
};

}}

#endif // KUSERFEEDBACK_CONSOLE_SECURITYSCANJOB_H
