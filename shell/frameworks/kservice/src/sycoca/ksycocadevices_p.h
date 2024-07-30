/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2009 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KSYCOCADEVICES_P_H
#define KSYCOCADEVICES_P_H

#include <config-ksycoca.h>
#include <stdlib.h>
// TODO: remove mmap() from kdewin32 and use QFile::mmap() when needed
#ifdef Q_OS_WIN
#define HAVE_MMAP 0
#endif

class QString;
class QDataStream;
class QBuffer;
class QFile;
class QIODevice;
class KMemFile;

class KSycocaAbstractDevice
{
public:
    KSycocaAbstractDevice()
        : m_stream(nullptr)
    {
    }

    virtual ~KSycocaAbstractDevice();

    virtual QIODevice *device() = 0;

    QDataStream *&stream();

private:
    QDataStream *m_stream;
};

#if HAVE_MMAP
// Reading from a mmap'ed file
class KSycocaMmapDevice : public KSycocaAbstractDevice
{
public:
    KSycocaMmapDevice(const char *sycoca_mmap, size_t sycoca_size);
    ~KSycocaMmapDevice() override;
    QIODevice *device() override;

private:
    QBuffer *m_buffer;
};
#endif

// Reading from a QFile
class KSycocaFileDevice : public KSycocaAbstractDevice
{
public:
    explicit KSycocaFileDevice(const QString &path);
    ~KSycocaFileDevice() override;
    QIODevice *device() override;

private:
    QFile *m_database;
};

#ifndef QT_NO_SHAREDMEMORY
// Reading from a KMemFile
class KSycocaMemFileDevice : public KSycocaAbstractDevice
{
public:
    explicit KSycocaMemFileDevice(const QString &path);
    ~KSycocaMemFileDevice() override;
    QIODevice *device() override;

private:
    KMemFile *m_database;
};
#endif

// Reading from a dummy memory buffer
class KSycocaBufferDevice : public KSycocaAbstractDevice
{
public:
    KSycocaBufferDevice();
    ~KSycocaBufferDevice() override;
    QIODevice *device() override;

private:
    QBuffer *m_buffer;
};

#endif /* KSYCOCADEVICES_P_H */
