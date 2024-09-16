// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#define private public
#include <private/qiodevice_p.h>
#undef private

#include "dzlibiodevice.h"
#include "../dglobal.h"

#include <QDataStream>
#include <QFile>
#include <QDebug>

#define BLOCK_SIZE 1024 * 1024

DZlibIODevice::DZlibIODevice(QObject *parent)
    : QIODevice(parent)
{

}

DZlibIODevice::DZlibIODevice(QIODevice *device, QObject *parent)
    : QIODevice(parent)
{
    setDevice(device);
}

DZlibIODevice::~DZlibIODevice()
{
    close();
}

void DZlibIODevice::setDevice(QIODevice *device)
{
    Q_ASSERT(device);

    close();

    m_device = device;
    m_lastBlockSize = BLOCK_SIZE;

    if (device->size() > metaDataSize() && device->open(QIODevice::ReadOnly)) {
        QDataStream stream(device);

        stream.setVersion(QDataStream::Qt_5_6);

        stream >> m_size;
        stream >> m_blockCount;
        stream >> m_lastBlockSize;

        device->close();
    }
}

bool DZlibIODevice::isSequential() const
{
    return true;
}

bool DZlibIODevice::open(QIODevice::OpenMode mode)
{
    if (isOpen()) {
        setErrorString("Device already open");

        return false;
    }

    if (mode != QIODevice::WriteOnly && mode != QIODevice::ReadOnly)
        return false;

    if (!m_device->open(mode))
        return false;

    if (!QIODevice::open(mode))
        return false;

    if (isReadMode()) {
        m_device->seek(metaDataSize());
    } else if (isWriteMode() && m_device->size() < metaDataSize()) {
        char data[metaDataSize() - m_device->size()];

        m_device->write(data, metaDataSize() - m_device->size());
        m_size = 0;
        m_blockCount = 0;
        m_lastBlockSize = BLOCK_SIZE;
    }

    return true;
}

void DZlibIODevice::close()
{
    if (!isOpen())
        return;

    if (m_writeBuffer.isEmpty()) {
        if (m_blockCount > 0)
            m_lastBlockSize = BLOCK_SIZE;
    } else {
        m_lastBlockSize = m_writeBuffer.size();

        if (!writeToBlock())
            m_lastBlockSize = BLOCK_SIZE;
    }

    if (isWriteMode()) {
        m_device->seek(0);
        QDataStream stream(m_device);
        stream.setVersion(QDataStream::Qt_5_6);
        stream << m_size << m_blockCount << m_lastBlockSize;
    }

    m_readBuffer.clear();
    m_currentBlock = -1;
    m_size = 0;
    m_blockCount = 0;
    m_lastBlockSize = 0;
    m_device->close();
    QIODevice::close();
}

qint64 DZlibIODevice::pos() const
{
    if (isWriteMode())
        return m_size;

    return m_size - bytesAvailable();
}

qint64 DZlibIODevice::size() const
{
    return m_size;
}

bool DZlibIODevice::atEnd() const
{
    return (m_currentBlock >= m_blockCount - 1 || m_device->atEnd()) && m_readBuffer.isEmpty();
}

qint64 DZlibIODevice::bytesAvailable() const
{
    if (!isReadMode())
        return QIODevice::bytesAvailable();

    if (m_currentBlock >= m_blockCount - 1)
        return m_readBuffer.size();

    return m_readBuffer.size() + (m_blockCount - m_currentBlock - 2) * BLOCK_SIZE + m_lastBlockSize;
}

qint64 DZlibIODevice::bytesToWrite() const
{
    return m_writeBuffer.size();
}

bool DZlibIODevice::canReadLine() const
{
    return false;
}

bool DZlibIODevice::waitForReadyRead(int msecs)
{
    return m_device->waitForReadyRead(msecs);
}

bool DZlibIODevice::waitForBytesWritten(int msecs)
{
    return m_device->waitForBytesWritten(msecs);
}

QString DZlibIODevice::errorString() const
{
    if (d_func()->errorString.isEmpty()) {
        if (m_device->d_func()->errorString.isEmpty())
            return QString();

        return m_device->errorString();
    }

    return QIODevice::errorString();
}

constexpr int DZlibIODevice::metaDataSize()
{
    return 20;
}

qint64 DZlibIODevice::readData(char *data, qint64 maxlen)
{
    qint64 size = 0;

    while (size < maxlen && !atEnd()) {
        if (m_readBuffer.isEmpty())
            readNextBlock();

        qint64 len = qMin(maxlen - size, (qint64)m_readBuffer.size());
        memcpy(data, m_readBuffer.constData(), len);
        size += len;
        m_readBuffer = m_readBuffer.mid(len);
    }

    return size;
}

qint64 DZlibIODevice::writeData(const char *data, qint64 len)
{
    m_writeBuffer.append(data, len);

    while (m_writeBuffer.size() >= BLOCK_SIZE) {
        if (!writeToBlock())
            return -1;
    }

    return len;
}

QByteArray DZlibIODevice::compress(const QByteArray &data) const
{
    return qCompress(data, Global::compressionLevel);
}

QByteArray DZlibIODevice::uncompress(const QByteArray &data) const
{
    return qUncompress(data);
}

bool DZlibIODevice::isReadMode() const
{
    OpenMode mode = openMode();

    return mode & (ReadOnly | Truncate | Text);
}

bool DZlibIODevice::isWriteMode() const
{
    OpenMode mode = openMode();

    return mode & (WriteOnly | Append);
}

void DZlibIODevice::readNextBlock()
{
    if (m_currentBlock >= m_blockCount - 1)
        return;

    ++m_currentBlock;

    int expectedSize = 0;

    QDataStream stream(m_device);
    stream.setVersion(QDataStream::Qt_5_6);
    stream >> expectedSize;

    if (expectedSize <= 0) {
        m_readBuffer.append(m_device->read(BLOCK_SIZE));

        return;
    }

    const QByteArray &array = m_device->read(expectedSize);

    m_readBuffer.append(uncompress(array));
}

bool DZlibIODevice::writeToBlock()
{
    const QByteArray &data = m_writeBuffer.left(BLOCK_SIZE);
    const QByteArray &compress_data = Global::compressionLevel > 0 ? compress(data) : data;
    QDataStream stream(m_device);
    stream.setVersion(QDataStream::Qt_5_6);
    stream << (Global::compressionLevel > 0 ? compress_data.size() : int(0));
    qint64 write_size = m_device->write(compress_data);

    if (write_size != compress_data.size()) {
        return false;
    }

    ++m_currentBlock;
    ++m_blockCount;
    m_size += data.size();
    emit bytesWritten(data.size());
    m_writeBuffer = m_writeBuffer.mid(data.size());

    return true;
}
