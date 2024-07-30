// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#pragma once

#include <QMap>
#include <QUrl>

// A generic item in the envelope
class SentryItem
{
public:
    QByteArray toEnvelopePayload() const;

    QVariantMap m_headers;
    QByteArray m_payload;
};

// A crash event item in the envelope
class SentryEvent : public SentryItem
{
public:
    explicit SentryEvent(const QByteArray &payload);
};

// A user feedback item in the envelope
class SentryUserFeedback : public SentryItem
{
public:
    explicit SentryUserFeedback(const QByteArray &payload);
};

// A blob of data for ingestion in the envelope endpoint of sentry
class SentryEnvelope
{
public:
    SentryEnvelope();
    void addItem(const SentryEvent &event);
    void addItem(const SentryItem &item);
    QByteArray toEnvelope() const;

    void setDSN(const QUrl &dsn);
    QString eventId() const;
    bool isEmpty() const;

private:
    friend class SentryEnvelopeTest;
    // Using map to have an easier time testing this because order will be always the same.
    QVariantMap m_headers;
    QList<SentryItem> m_items;
    bool m_hasEvent = false;
};
