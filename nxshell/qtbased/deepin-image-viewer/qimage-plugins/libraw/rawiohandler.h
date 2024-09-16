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

#ifndef RAW_IO_HANDLER_H
#define RAW_IO_HANDLER_H

#include <QImageIOHandler>

class QImage;
class QByteArray;
class QIODevice;
class QVariant;

class RawIOHandlerPrivate;
class RawIOHandler: public QImageIOHandler
{
public:
    RawIOHandler();
    ~RawIOHandler();

    virtual bool canRead() const;
    virtual bool read(QImage *image);
    static bool canRead(QIODevice *device);
    virtual QVariant option(ImageOption option) const;
    virtual void setOption(ImageOption option, const QVariant &value);
    virtual bool supportsOption(ImageOption option) const;

private:
    RawIOHandlerPrivate *d;
};

#endif // RAW_IO_HANDLER_H
