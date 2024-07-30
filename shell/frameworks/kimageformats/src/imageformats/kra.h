/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Boudewijn Rempt <boud@valdyas.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_KRA_H
#define KIMG_KRA_H

#include <QImageIOPlugin>

class KraHandler : public QImageIOHandler
{
public:
    KraHandler();

    bool canRead() const override;
    bool read(QImage *image) override;

    static bool canRead(QIODevice *device);
};

class KraPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "kra.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif
