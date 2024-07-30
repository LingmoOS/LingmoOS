/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef YOUTUBEJOB_H
#define YOUTUBEJOB_H

#include <KJob>
#include <QNetworkAccessManager>
#include <QString>
#include <QUrl>

class YoutubeJob : public KJob
{
    Q_OBJECT
public:
    YoutubeJob(const QUrl &url, const QByteArray &token, const QString &title, const QStringList &tags, const QString &description, QObject *parent = nullptr);
    void start() override;

    QString outputUrl() const
    {
        return m_output;
    }

private:
    void fileFetched(KJob *);
    void createLocation();
    void locationCreated();
    void uploadVideo(const QByteArray &data);
    void videoUploaded();

    QUrl m_url;
    QByteArray m_token;
    QString m_output;
    QNetworkAccessManager m_manager;
    QByteArray m_metadata;
    QUrl m_uploadUrl;
};
#endif /* YOUTUBEJOB_H */
