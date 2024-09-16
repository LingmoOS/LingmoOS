// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QThread>
#include <QNetworkReply>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QFile>
#include <QDir>

#include <ctime>

#include "checksizethread.h"
#include "../../core/constants.h"

class FileDownloader: public QObject
{
    Q_OBJECT
public:
    FileDownloader(QObject *parent = nullptr);
    void startDownload(QString url);
    qint64 getTotalSize() const {
        return m_totalSize;
    }
    const QNetworkReply::NetworkError getError() {
        return m_errCode;
    }
    const QString getErrorLog() const {
        return m_errStr;
    }
    const QString getFileName() const {
        return m_url.isValid() ? m_url.fileName() : QString("");
    }
    const QString getFilePath() const
    {
        QDir downloadPath = QDir::home();
        downloadPath.cd(kDownloadPath);
        return downloadPath.absoluteFilePath(getFileName());
    }

Q_SIGNALS:
    void done();
    void error();
    void progress(uint v);

public Q_SLOTS:
    void abort();
    void download();
    void retryDownload();
    void onSizeAvailable(long long size);

private:
    // The initial disconnected time
    time_t  m_disconnectedTime = 0;
    QUrl    m_url;
    qint64  m_curSize = 0;
    qint64  m_totalSize;
    QFile   *m_file = nullptr;
    QNetworkReply::NetworkError m_errCode;
    QString     m_errStr;
    QTimer      m_retryTimer;
    QNetworkAccessManager m_netMgr;
    QNetworkReply   *m_reply = nullptr;
    CheckSizeThread *m_checkSizeThread = nullptr;

    void initConnections(QNetworkReply *reply);
};
