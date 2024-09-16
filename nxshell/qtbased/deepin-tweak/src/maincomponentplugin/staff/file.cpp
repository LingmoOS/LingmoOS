// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

#include "file.h"

bool FileBackend::open(const QVariant &mode)
{
    return file->open(static_cast<QFile::OpenModeFlag>(mode.toInt()));
}

void FileBackend::close()
{
    return file->close();
}

QByteArray FileBackend::read(qint64 maxSize)
{
    return file->read(maxSize);
}

QByteArray FileBackend::readAll()
{
    return file->readAll();
}

qint64 FileBackend::write(const QByteArray &data)
{
    return file->write(data);
}

bool FileBackend::flush()
{
    return file->flush();
}

bool FileBackend::rename(const QString &newName)
{
    return file->rename(newName);
}

bool FileBackend::remove()
{
    return file->remove();
}

bool FileBackend::exists() const
{
    return file->exists();
}

bool FileBackend::seek(qint64 pos)
{
    return file->seek(pos);
}

qint64 FileBackend::pos() const
{
    return file->pos();
}

qint64 FileBackend::size() const
{
    return file->size();
}
