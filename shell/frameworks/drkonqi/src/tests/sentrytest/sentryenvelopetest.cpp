// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include <QTest>

#include <sentryenvelope.h>

using namespace Qt::StringLiterals;

class SentryEnvelopeTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testConstruction()
    {
        const auto payload = "{\"event_id\":\"9ec79c33ec9942ab8353589fcb2e04dc\",\"message\":\"hello world\",\"level\":\"error\"}\n"_qba;
        SentryEnvelope envelope;
        // Fixate for test
        envelope.m_headers["event_id"] = "9ec79c33ec9942ab8353589fcb2e04dc";
        envelope.setDSN(QUrl("https://foo.bar"_L1));
        envelope.addItem(SentryEvent(payload));
        envelope.addItem(SentryUserFeedback("{\"email\":\"john@me.com\",\"name\":\"John Me\",\"comments\":\"It broke.\"}\n"_qba));

        QFile f(QFINDTESTDATA("data/sentryenvelope"));
        QVERIFY(f.open(QFile::ReadOnly));
        QCOMPARE(envelope.toEnvelope(), f.readAll());
    }
};

QTEST_GUILESS_MAIN(SentryEnvelopeTest)

#include "sentryenvelopetest.moc"
