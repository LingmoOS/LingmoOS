/*
    kimgio module for SGI images
    SPDX-FileCopyrightText: 2004 Melchior FRANZ <mfranz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_RGB_P_H
#define KIMG_RGB_P_H

#include <QImageIOPlugin>

class RGBHandler : public QImageIOHandler
{
public:
    RGBHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;

    static bool canRead(QIODevice *device);
};

class RGBPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "rgb.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_RGB_P_H
