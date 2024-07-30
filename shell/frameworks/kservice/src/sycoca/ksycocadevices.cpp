/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kmemfile_p.h"
#include "ksycocadevices_p.h"
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <fcntl.h>

KSycocaAbstractDevice::~KSycocaAbstractDevice()
{
    delete m_stream;
}

QDataStream *&KSycocaAbstractDevice::stream()
{
    if (!m_stream) {
        m_stream = new QDataStream(device());
        m_stream->setVersion(QDataStream::Qt_5_3);
    }
    return m_stream;
}

#if HAVE_MMAP
KSycocaMmapDevice::KSycocaMmapDevice(const char *sycoca_mmap, size_t sycoca_size)
{
    m_buffer = new QBuffer;
    m_buffer->setData(QByteArray::fromRawData(sycoca_mmap, sycoca_size));
}

KSycocaMmapDevice::~KSycocaMmapDevice()
{
    delete m_buffer;
}

QIODevice *KSycocaMmapDevice::device()
{
    return m_buffer;
}
#endif

KSycocaFileDevice::KSycocaFileDevice(const QString &path)
{
    m_database = new QFile(path);
#ifndef Q_OS_WIN
    (void)fcntl(m_database->handle(), F_SETFD, FD_CLOEXEC);
#endif
}

KSycocaFileDevice::~KSycocaFileDevice()
{
    delete m_database;
}

QIODevice *KSycocaFileDevice::device()
{
    return m_database;
}

#ifndef QT_NO_SHAREDMEMORY
KSycocaMemFileDevice::KSycocaMemFileDevice(const QString &path)
{
    m_database = new KMemFile(path);
}

KSycocaMemFileDevice::~KSycocaMemFileDevice()
{
    delete m_database;
}

QIODevice *KSycocaMemFileDevice::device()
{
    return m_database;
}
#endif

KSycocaBufferDevice::KSycocaBufferDevice()
{
    m_buffer = new QBuffer;
}

KSycocaBufferDevice::~KSycocaBufferDevice()
{
    delete m_buffer;
}

QIODevice *KSycocaBufferDevice::device()
{
    return m_buffer;
}
