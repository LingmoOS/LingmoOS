/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Ignacio Castaño <castano@ludicon.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_PSD_P_H
#define KIMG_PSD_P_H

#include <QImageIOPlugin>
#include <QScopedPointer>

class PSDHandlerPrivate;
class PSDHandler : public QImageIOHandler
{
public:
    PSDHandler();

    bool canRead() const override;
    bool read(QImage *image) override;

    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    const QScopedPointer<PSDHandlerPrivate> d;
};

class PSDPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "psd.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_PSD_P_H
