/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef APIJOB_H
#define APIJOB_H

#include <QJsonArray> // Not used here but included so clients don't have to
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>

#include <KJob>

class QNetworkRequest;
class QNetworkReply;

namespace KIO
{
class TransferJob;
} // namespace KIO

namespace Bugzilla
{
class APIJob : public KJob
{
    Q_OBJECT
public:
    using KJob::KJob;

    /**
     * @throws ProtocolException on unexpected HTTP statuses or KIO errors
     * @throws APIException when the API returned an error object
     * @return json document returned by api request
     */
    QJsonDocument document() const;

    /**
     * This is a convenience function on top of document() and may throw all
     * the same exceptions.
     * @return json object of document (may be in valid if the doc has none)
     */
    QJsonObject object() const;

    void setAutoStart(bool start);

public:
    // Should be protected but since we call it for testing I don't care.
    virtual QByteArray data() const = 0;

private:
    void start() override
    {
    }
    void connectNotify(const QMetaMethod &signal) override;

    bool m_autostart = true;
};

class NetworkAPIJob : public APIJob
{
    Q_OBJECT
    friend class HTTPConnection; // constructs us, ctor is private though
public:
    QByteArray data() const override
    {
        return m_data;
    }

private:
    explicit NetworkAPIJob(const QUrl &url,
                           const std::function<QNetworkReply *(QNetworkAccessManager &, QNetworkRequest &)> &starter,
                           QObject *parent = nullptr);

    QByteArray m_data;
    QByteArray m_putData;
    QList<QByteArray> m_dataSegments;

    QNetworkAccessManager m_manager;
};

} // namespace Bugzilla

#endif // APIJOB_H
