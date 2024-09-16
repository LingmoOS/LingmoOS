/*
 * Copyright (C) 2012 Alberto Mardegan <info@mardy.it>
 *
 * This file is part of QtRaw.
 *
 * QtRaw is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QtRaw is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QtRaw.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "datastream.h"

#include <QIODevice>
#include <QTextStream>

Datastream::Datastream(QIODevice *device):
    m_device(device)
{
}

Datastream::~Datastream()
{
}

int Datastream::valid()
{
    return m_device->isReadable();
}

/**
 * @brief 按块读取数据，参数和返回值方式与 fread(ptr,size,nmemb,file) 相似。
 * @param ptr   读取数据写入指针
 * @param size  块大小
 * @param nmemb 读取块数
 * @return 成功读取的块数
 */
int Datastream::read(void *ptr, size_t size, size_t nmemb)
{
    qint64 readDataLen = m_device->read(static_cast<char *>(ptr), qint64(size * nmemb));
    // 返回读取文本块数量而非读取数据总长度
    return static_cast<int>(readDataLen / qint64(size > 0 ? size : 1));
}

int Datastream::seek(INT64 offset, int whence)
{
    if (! m_device->isOpen()) return -1;

    qint64 pos;
    switch (whence) {
    case SEEK_SET:
        pos = offset;
        break;
    case SEEK_CUR:
        pos = m_device->pos() + offset;
        break;
    case SEEK_END:
        pos = m_device->size();
        break;
    default:
        return -1;
    }

    if (pos < 0) pos = 0;

    if (m_device->bytesAvailable() < pos) return -1;

    return m_device->seek(pos) ? 0 : -1;
}

INT64 Datastream::tell()
{
    return m_device->pos();
}

INT64 Datastream::size()
{
    return m_device->size();
}

int Datastream::get_char()
{
    char c;
    return m_device->getChar(&c) ? (unsigned char)c : -1;
}

char *Datastream::gets(char *s, int n)
{
    return m_device->readLine(s, n) >= 0 ? s : nullptr;
}

int Datastream::scanf_one(const char *fmt, void *val)
{
    QTextStream stream(m_device);
    /* This is only used for %d or %f */
    if (qstrcmp(fmt, "%d") == 0) {
        int d;
        stream >> d;
        *(static_cast<int *>(val)) = d;
    } else if (qstrcmp(fmt, "%f") == 0) {
        float f;
        stream >> f;
        *(static_cast<float *>(val)) = f;
    } else {
        return 0;
    }
    return (stream.status() == QTextStream::Ok) ? 1 : EOF;
}

int Datastream::eof()
{
    return m_device->atEnd();
}

void *Datastream::make_jas_stream()
{
    return nullptr;
}
