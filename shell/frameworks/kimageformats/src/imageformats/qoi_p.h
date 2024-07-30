/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2023 Ernest Gupik <ernestgupik@wp.pl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_QOI_P_H
#define KIMG_QOI_P_H

#include <QImageIOPlugin>
#include <QScopedPointer>

class QOIHandlerPrivate;
class QOIHandler : public QImageIOHandler
{
public:
    QOIHandler();

    bool canRead() const override;
    bool read(QImage *image) override;
    bool write(const QImage &image) override;

    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    const QScopedPointer<QOIHandlerPrivate> d;
};

class QOIPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "qoi.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_QOI_P_H
