// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QSVGIOHANDLER_H
#define QSVGIOHANDLER_H

#include <QtGui/qimageiohandler.h>

#ifndef QT_NO_SVGRENDERER

QT_BEGIN_NAMESPACE

class QImage;
class QByteArray;
class QIODevice;
class QVariant;

class QSvgIOHandlerPrivate;
class QSvgIOHandler : public QImageIOHandler
{
public:
    QSvgIOHandler();
    ~QSvgIOHandler();
    virtual bool canRead() const;
    virtual QByteArray name() const;
    virtual bool read(QImage *image);
    static bool canRead(QIODevice *device);
    virtual QVariant option(ImageOption option) const;
    virtual void setOption(ImageOption option, const QVariant & value);
    virtual bool supportsOption(ImageOption option) const;

private:
    QSvgIOHandlerPrivate *d;
};

QT_END_NAMESPACE

#endif // QT_NO_SVGRENDERER
#endif // QSVGIOHANDLER_H
