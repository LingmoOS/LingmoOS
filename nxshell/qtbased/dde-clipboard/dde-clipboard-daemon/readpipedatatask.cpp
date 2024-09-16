// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "readpipedatatask.h"

#include <QDebug>
#include <QFile>
#include <utility>

#include <DWayland/Client/connection_thread.h>
#include <DWayland/Client/datacontroloffer.h>

#include <QtConcurrent>

#include <unistd.h>

#include <mutex>

std::mutex PIPELINE_GUARD;

constexpr int BYTE_MAX = 1024 * 4;

ReadPipeDataTask::ReadPipeDataTask(ConnectionThread *connectionThread, DataControlOfferV1 *offerV1, const QString &mimeType, QObject *parent)
    : QObject(parent)
    , m_mimeType(mimeType)
    , m_pipeIsForcedClosed(false)
    , m_pOffer(offerV1)
    , m_pConnectionThread(connectionThread)
{

}

void ReadPipeDataTask::run()
{
    if (!m_pConnectionThread || !m_pOffer || m_mimeType.isEmpty())
        return;

    int pipeFds[2];
    if (pipe(pipeFds) != 0) {
        qWarning() << "Create pipe failed.";

        // 避免返回数据量少
        Q_EMIT dataReady((qint64)m_pOffer, m_mimeType, QByteArray());
        return;
    }

    // 根据mime类取数据，写入pipe中
    m_pOffer->receive(m_mimeType, pipeFds[1]);
    m_pConnectionThread->roundtrip();
    close(pipeFds[1]);

    // force to close the piepline
    connect(this, &ReadPipeDataTask::forceClosePipeLine, this, [pipeFds, this] {
        std::lock_guard<std::mutex> guard(PIPELINE_GUARD);
        m_pipeIsForcedClosed = true;
        close(pipeFds[0]);
    });

    QtConcurrent::run([this, pipeFds] {
        QByteArray data;
        readData(pipeFds[0], data);
        std::lock_guard<std::mutex> guard(PIPELINE_GUARD);
        this->deleteLater();
        if (m_pipeIsForcedClosed) {
            qDebug() << "pipeline is block here;" << "mimetype is: " << m_mimeType;
            return ;
        }

        Q_EMIT dataReady((qint64)m_pOffer, m_mimeType, data);

        close(pipeFds[0]);
    });
}

bool ReadPipeDataTask::readData(int fd, QByteArray &data)
{
    QFile readPipe;
    if (!readPipe.open(fd, QIODevice::ReadOnly)) {
        return false;
    }

    if (!readPipe.isReadable()) {
        qWarning() << "Pipe is not readable";
        readPipe.close();
        return false;
    }

    int retCount = BYTE_MAX;
    do {
        QByteArray bytes = readPipe.read(BYTE_MAX);
        retCount = bytes.count();
        if (!bytes.isEmpty())
            data.append(bytes);
    } while (retCount == BYTE_MAX);

    readPipe.close();

    return true;
}


