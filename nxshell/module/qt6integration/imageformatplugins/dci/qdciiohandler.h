// Copyright (C) 2022 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QDCIIOHANDLER_H
#define QDCIIOHANDLER_H

#include <QtGui/qimageiohandler.h>

QT_BEGIN_NAMESPACE

class QImage;
class QByteArray;
class QIODevice;
class QVariant;

class QDciIOHandlerPrivate;
class QDciIOHandler : public QImageIOHandler
{
public:
    QDciIOHandler();
    ~QDciIOHandler();
    virtual bool canRead() const;
    virtual QByteArray name() const;
    virtual bool read(QImage *image);
    static bool canRead(QIODevice *device);
    virtual QVariant option(ImageOption option) const;
    virtual void setOption(ImageOption option, const QVariant & value);
    virtual bool supportsOption(ImageOption option) const;

private:
    QDciIOHandlerPrivate *d;
};

QT_END_NAMESPACE

#endif // QDCIIOHANDLER_H
