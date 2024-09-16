// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QDir>
#include <QByteArray>
#include <QNetworkRequest>
#include <QNetworkConfiguration>

#include <iostream>
#include <cstdio>

#include "../../core/constants.h"
#include "filedownloader.h"

FileDownloader::FileDownloader(QObject *parent): QObject(parent)
{
    connect(&m_retryTimer, &QTimer::timeout, this, &FileDownloader::retryDownload);
}

void FileDownloader::startDownload(QString urlStr)
{
    // Validate URL
    if (!QUrl(urlStr).isValid())
    {
        m_errCode = QNetworkReply::NoError;
        m_errStr = QString(tr("URL \"%1\" is illegal").arg(urlStr));
        emit error();
        return;
    }

    m_url = QUrl(urlStr);
    if (m_checkSizeThread != nullptr)
    {
        disconnect(m_checkSizeThread);
    }
    m_checkSizeThread = new CheckSizeThread(m_url.toString());
    connect(m_checkSizeThread, &CheckSizeThread::done, this, &FileDownloader::onSizeAvailable);
    connect(m_checkSizeThread, &CheckSizeThread::error, this, [=](const QString errMsg) {
        m_errCode = QNetworkReply::NoError;
        m_errStr = errMsg;
        emit error();
    });
    m_checkSizeThread->start();
}

void FileDownloader::initConnections(QNetworkReply *reply)
{
    connect(reply, &QNetworkReply::downloadProgress, [this] (qint64 bytesReceived, qint64 bytesTotal) {
        m_totalSize = m_curSize + bytesTotal;
        Q_EMIT progress((bytesReceived + m_curSize) * 100 / m_totalSize);
    });
    connect(reply, &QNetworkReply::readyRead, this, [=] {
        // Writting data only under normal situation
        if (reply->error() == QNetworkReply::NoError)
        {
            const QByteArray a = reply->readAll();

            if (m_file != nullptr && m_file->isOpen())
                m_file->write(a);
        }
    });
    connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (m_file && m_file->isOpen())
        {
            m_file->close();
            m_file->deleteLater();
            m_file = nullptr;
        }
    });
    connect(reply, &QNetworkReply::finished, [this, reply] {
        QNetworkReply::NetworkError err = reply->error();
        if (err == QNetworkReply::NoError)
        {
            Q_EMIT done();
        }
        else if (err == QNetworkReply::NetworkSessionFailedError)
        {
            // Handle disconnected error
            qCritical() << "QNetworkReply::NetworkSessionFailedError received";
            qCritical() << "error message" << reply->errorString();
            m_errCode = reply->error();
            m_errStr = reply->errorString();
            this->abort();

            m_disconnectedTime = std::time(nullptr);

            // Start retry timer
            m_retryTimer.start(kRetryInterval);
            return;
        }
        else
        {
            m_errCode = reply->error();
            m_errStr = reply->errorString();
            reply->abort();
            reply->deleteLater();
            qCritical() << "Download error message:";
            qCritical() << "error code:" << m_errCode;
            qCritical() << "error message:" << m_errStr;
            Q_EMIT error();
        }

    });
}

void FileDownloader::abort()
{
    if (m_reply)
    {
        qDebug() << "isRunning:" << m_reply->isRunning();
        qDebug() << "isFinished:" << m_reply->isFinished();

        if (m_reply->isRunning())
        {
            m_reply->abort();
        }

        disconnect(m_reply);
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    else
    {
        // Handle cancel action when size checking is not done yet.
        // And it is required to trigger error signal with QNetworkReply::OperationCanceledError
        m_errCode = QNetworkReply::OperationCanceledError;
        emit error();
    }
}

void FileDownloader::onSizeAvailable(long long size)
{
    // Stop retry timer when downloading is started/resumed.
    m_retryTimer.stop();

    // Update image size and start downloading.
    m_totalSize = size;

    download();
}

void FileDownloader::download()
{
    // If downloading has already been started, then skip starting downloading.
    if (m_file && m_file->isOpen())
        return;

    QDir downloadDir = QDir::home();
    downloadDir.mkpath(kDownloadPath);
    downloadDir.cd(kDownloadPath);


    QString filename = getFileName();

    m_file = new QFile(downloadDir.absoluteFilePath(filename));

    if (QFile::exists(downloadDir.absoluteFilePath(filename)))
    {
        QNetworkRequest req(m_url);
        m_curSize = m_file->size();
        req.setRawHeader(QByteArray("Range"), "bytes=" + QByteArray::number(m_curSize) + "-");
        // Only size check is required here. The integrity check later will ensure that the downloaded image is intact.
        if (m_curSize >= m_totalSize)
        {
            emit done();
            return;
        }

        m_reply = m_netMgr.get(req);
        if (m_reply->error() != QNetworkReply::NoError)
        {
            emit error();
            return;
        }

        if (m_file->open(QFile::WriteOnly | QFile::Append))
        {
            initConnections(m_reply);
        }
        else
        {
            qCritical() << QString("ISO File: %1 cannot be opened as \"Append & Write\"").arg(m_url.toString());
        }
    }
    else
    {
        m_curSize = 0;
        m_reply = m_netMgr.get(QNetworkRequest(m_url));
        if (m_reply->error() != QNetworkReply::NoError)
        {
            emit error();
            return;
        }

        if (m_file->open(QFile::WriteOnly))
        {
            initConnections(m_reply);
        }
    }
    m_reply->setReadBufferSize(kReadBufferSize);
}

void FileDownloader::retryDownload()
{
    qCritical() << "Retry downloading, elapsed time since last disconnected:" << std::time(nullptr) - m_disconnectedTime << "s";
    // If the retrying time is longer than timeout, then abandon retrying and emit error signal.
    if (std::time(nullptr) - m_disconnectedTime > kReconnectTimeout)
    {
        m_retryTimer.stop();
        emit error();
        return;
    }

    // Check network connectivity. And start downloading when network is reconnected.
    if (m_checkSizeThread != nullptr)
    {
        disconnect(m_checkSizeThread);
        m_checkSizeThread->deleteLater();
    }
    m_checkSizeThread = new CheckSizeThread(m_url.toString());
    connect(m_checkSizeThread, &CheckSizeThread::done, this, &FileDownloader::onSizeAvailable);
    m_checkSizeThread->start();
}
