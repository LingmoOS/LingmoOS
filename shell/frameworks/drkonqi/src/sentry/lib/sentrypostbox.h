// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

#pragma once

#include <memory>

#include <QNetworkAccessManager>
#include <QObject>

#include "sentrydsns.h"
#include "sentryenvelope.h"

// Manager for sentry envelope construction and delivery
class SentryPostbox : public QObject
{
    Q_OBJECT
public:
    explicit SentryPostbox(const QString &applicationName, std::shared_ptr<SentryConnection> connection, QObject *parent = nullptr);

    void addEventPayload(const QJsonDocument &document);
    void addUserFeedback(const QString &feedbackString);

    void deliver();
    bool hasDelivered() const;
    bool isReadyToDeliver() const;
    [[nodiscard]] QString eventId() const;

Q_SIGNALS:
    void hasDeliveredChanged();

private Q_SLOTS:
    void onDSNsLoaded();

private:
    SentryEnvelope m_envelope;
    SentryDSNs m_dsns;
    SentryDSNContext m_dsnContext;

    QString m_applicationName;
    bool m_hasDelivered = false;
    bool m_dsnSet = false;
};
