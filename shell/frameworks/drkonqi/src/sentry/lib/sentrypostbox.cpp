// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2022-2023 Harald Sitter <sitter@kde.org>

#include "sentrypostbox.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>

#include "sentryconnection.h"
#include "sentrypaths.h"

using namespace Qt::StringLiterals;

SentryPostbox::SentryPostbox(const QString &applicationName, std::shared_ptr<SentryConnection> connection, QObject *parent)
    : QObject(parent)
    , m_dsns(std::move(connection))
    , m_applicationName(applicationName)
{
    connect(&m_dsns, &SentryDSNs::loaded, this, &SentryPostbox::onDSNsLoaded);
    m_dsns.load();
}

void SentryPostbox::onDSNsLoaded()
{
    m_dsnContext = m_dsns.context(m_applicationName);

    m_envelope.setDSN(m_dsnContext.dsnUrl());
    m_dsnSet = true;
}

void SentryPostbox::addEventPayload(const QJsonDocument &document)
{
    document.object().insert(u"event_id"_s, QJsonValue(m_envelope.eventId()));
    m_envelope.addItem(SentryEvent(document.toJson(QJsonDocument::Compact)));

    m_hasDelivered = false;
    Q_EMIT hasDeliveredChanged();
}

void SentryPostbox::addUserFeedback(const QString &feedbackString)
{
    const QJsonObject feedbackObject = {
        {u"event_id"_s, m_envelope.eventId()},
        {QStringLiteral("name"), QStringLiteral("DrKonqi")},
        {QStringLiteral("email"), QStringLiteral("anonymous@kde.org")},
        {QStringLiteral("comments"), feedbackString},
    };

    m_envelope.addItem(SentryUserFeedback(QJsonDocument(feedbackObject).toJson(QJsonDocument::Compact)));

    m_hasDelivered = false;
    Q_EMIT hasDeliveredChanged();
}

void SentryPostbox::deliver()
{
    Q_ASSERT(!m_envelope.eventId().isEmpty()); // requirement for path construction
    QFile file(SentryPaths::payloadPath(m_envelope.eventId()));
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        file.write(m_envelope.toEnvelope());
    }
    m_hasDelivered = true;
    Q_EMIT hasDeliveredChanged();
}

bool SentryPostbox::hasDelivered() const
{
    // NOTE: when items are added AFTER delivery we toggle back to not delivered, as the postbox has pending stuff again
    //   this enables us to send the event and then follow it up with sending the feedback if necessary.
    return m_hasDelivered;
}

bool SentryPostbox::isReadyToDeliver() const
{
    return m_dsnSet && !m_envelope.isEmpty();
}

QString SentryPostbox::eventId() const
{
    return m_envelope.eventId();
}

#include "moc_sentrypostbox.cpp"
