#include "downloader.h"
#include <QEventLoop>
#include <QDir>

using namespace GrandSearch;

Downloader::Downloader(const QString &directory, QObject *parent) : QObject(parent),
    m_manager(new QNetworkAccessManager(this)),
    m_downloadDirectory(directory),
    m_finished(false)
{
    connect(m_manager, &QNetworkAccessManager::finished, this, &Downloader::onDownloadFinished);
}

Downloader::~Downloader()
{
    QMutexLocker locker(&mutex);
    m_finished = true;
    foreach (QNetworkReply *reply, m_activeDownloads) {
        if (m_openFiles.contains(reply)) {
            QFile *file = m_openFiles.take(reply);
            if (file->isOpen()) {
                file->close();
            }
            file->deleteLater();
        }
        reply->deleteLater();
    }
    m_manager->deleteLater();
}

void Downloader::addDownloadTask(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = m_manager->get(request);
    {
        QMutexLocker locker(&mutex);
        m_finished = false;
        m_activeDownloads.append(reply);
    }
    if (url.fileName().contains(".gguf"))
        connect(reply, &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgress);
    connect(reply, &QNetworkReply::readyRead, this, &Downloader::onReadyRead);
}

void Downloader::onDownloadFinished(QNetworkReply *reply)
{
    QMutexLocker locker(&mutex);
    disconnect(reply);
    m_activeDownloads.removeAll(reply);

    if (m_openFiles.contains(reply)) {
        QFile *file = m_openFiles.take(reply);
        if (file->isOpen()) {
            file->close();
        }
        file->deleteLater();
    }

    if (m_activeDownloads.isEmpty() && !m_finished) {
        m_finished = true;
        waitCondition.wakeAll();
        emit downloadFinished();
    }

    reply->deleteLater();
}

void Downloader::onReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    QString filename = reply->request().url().fileName();
    QString localFilePath = m_downloadDirectory + "/" + filename;
    QFile *file = m_openFiles.value(reply);
    if (!file) {
        file = new QFile(localFilePath);
        if (QFile::exists(localFilePath)) {
            QFile::remove(localFilePath);
        }
        if (!file->open(QIODevice::WriteOnly)) {
            qDebug() << "Unable to open" << localFilePath << "for writing:" << file->errorString();
            reply->abort();
            delete file;
            return;
        }
        m_openFiles.insert(reply, file);
    }

    file->write(reply->read(reply->bytesAvailable()));
}

void Downloader::cancelDownloads()
{
    QMutexLocker locker(&mutex);
    m_finished = true;
    foreach (QNetworkReply *reply, m_activeDownloads) {
        if (m_openFiles.contains(reply)) {
            QFile *file = m_openFiles.take(reply);
            if (file->isOpen()) {
                file->close();
            }
            file->deleteLater();
        }
        reply->deleteLater();
    }

    m_activeDownloads.clear();
    m_openFiles.clear();

}
