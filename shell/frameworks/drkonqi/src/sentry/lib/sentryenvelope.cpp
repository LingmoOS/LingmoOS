// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include "sentryenvelope.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace Qt::StringLiterals;

static constexpr auto EVENT_ID = "event_id"_L1;

SentryEnvelope::SentryEnvelope()
    : m_headers({{EVENT_ID, QUuid::createUuid().toString(QUuid::Id128)}})
{
}

SentryEvent::SentryEvent(const QByteArray &payload)
{
    m_headers.insert("type"_L1, "event"_L1);
    m_payload = payload;
}

SentryUserFeedback::SentryUserFeedback(const QByteArray &payload)
{
    m_headers.insert("type"_L1, "user_report"_L1);
    m_payload = payload;
}

QByteArray SentryItem::toEnvelopePayload() const
{
    auto headers = m_headers;
    headers[u"length"_s] = m_payload.length();
    QJsonDocument doc(QJsonObject::fromVariantMap(headers));
    const auto header = doc.toJson(QJsonDocument::Compact);
    return "%1\n%2"_L1.arg(QString::fromUtf8(header), QString::fromUtf8(m_payload)).toUtf8();
}

QByteArray SentryEnvelope::toEnvelope() const
{
    QJsonDocument doc(QJsonObject::fromVariantMap(m_headers));
    const auto header = doc.toJson(QJsonDocument::Compact);
    QList<QByteArray> payloads;
    payloads.reserve(m_items.size());
    for (const auto &item : m_items) {
        payloads.push_back(item.toEnvelopePayload());
    }
    return "%1\n%2"_L1.arg(QString::fromUtf8(header), QString::fromUtf8(payloads.join("\n"_qba))).toUtf8();
}

void SentryEnvelope::addItem(const SentryEvent &event)
{
    m_hasEvent = true;
    addItem(static_cast<const SentryItem &>(event));
}

void SentryEnvelope::addItem(const SentryItem &item)
{
    m_items.push_back(item);
}

QString SentryEnvelope::eventId() const
{
    return m_headers.value(EVENT_ID).toString();
}

void SentryEnvelope::setDSN(const QUrl &dsn)
{
    m_headers.insert("dsn"_L1, dsn.toString());
}

bool SentryEnvelope::isEmpty() const
{
    // NOTE: At the time of writing we can only have an Event or UserFeedback as items and the latter cannot
    //   currently be submitted on its own (a v2 where the feedback is itself an event is in the works upstream).
    return m_items.isEmpty() || !m_hasEvent;
}
