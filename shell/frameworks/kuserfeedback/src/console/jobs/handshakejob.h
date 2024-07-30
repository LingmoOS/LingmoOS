/*
    SPDX-FileCopyrightText: 2017 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#ifndef KUSERFEEDBACK_CONSOLE_HANDSHAKEJOB_H
#define KUSERFEEDBACK_CONSOLE_HANDSHAKEJOB_H

#include "job.h"

class QNetworkReply;

namespace KUserFeedback {
namespace Console {

class RESTClient;

class HandshakeJob : public Job
{
    Q_OBJECT
public:
    explicit HandshakeJob(RESTClient *restClient, QObject *parent = nullptr);
    ~HandshakeJob() override;

private:
    void processResponse(QNetworkReply *reply);
    RESTClient *m_restClient;
};

}
}

#endif // KUSERFEEDBACK_CONSOLE_HANDSHAKEJOB_H
