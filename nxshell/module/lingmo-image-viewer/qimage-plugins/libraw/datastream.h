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

#ifndef DATASTREAM_H
#define DATASTREAM_H

#include <QImageIOHandler>

#include <libraw_datastream.h>

class QIODevice;

class Datastream: public LibRaw_abstract_datastream
{
public:
    explicit Datastream(QIODevice *device);
    ~Datastream();

    // reimplemented virtual methods:
    virtual int valid();
    virtual int read(void *ptr, size_t size, size_t nmemb);
    virtual int seek(INT64 offset, int whence);
    virtual INT64 tell();
    virtual INT64 size();
    virtual int get_char();
    virtual char *gets(char *s, int n);
    virtual int scanf_one(const char *fmt, void *val);
    virtual int eof();
    virtual void *make_jas_stream();

private:
    QIODevice *m_device;
};

#endif // DATASTREAM_H
