// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

namespace GrandSearch {

class Downloader : public QObject
{
    Q_OBJECT

public:
    explicit Downloader(const QString &directory, QObject *parent = nullptr);
    ~Downloader();

    void addDownloadTask(const QUrl &url);
    void cancelDownloads();
    bool isFinished() { return m_finished; }

signals:
    void downloadFinished();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private slots:
    void onDownloadFinished(QNetworkReply *reply);
    void onReadyRead();

private:
    QNetworkAccessManager *m_manager;
    QString m_downloadDirectory;
    QList<QNetworkReply*> m_activeDownloads;
    bool m_finished;

    QMutex mutex;
    QWaitCondition waitCondition;
    QHash<QNetworkReply*, QFile*> m_openFiles;
};
}

#endif // DOWNLOADER_H
