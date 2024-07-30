/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Dominik Seichter <domseichter@web.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_TGA_P_H
#define KIMG_TGA_P_H

#include <QImageIOPlugin>
#include <QScopedPointer>

class TGAHandlerPrivate;
class TGAHandler : public QImageIOHandler
{
public:
    TGAHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;

    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    const QScopedPointer<TGAHandlerPrivate> d;
};

class TGAPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "tga.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_TGA_P_H
