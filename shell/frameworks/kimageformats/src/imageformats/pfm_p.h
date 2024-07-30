/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2024 Mirco Miranda <mircomir@outlook.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMG_PFM_P_H
#define KIMG_PFM_P_H

#include <QImageIOPlugin>
#include <QScopedPointer>

class PFMHandlerPrivate;
class PFMHandler : public QImageIOHandler
{
public:
    PFMHandler();

    bool canRead() const override;
    bool read(QImage *image) override;

    bool supportsOption(QImageIOHandler::ImageOption option) const override;
    QVariant option(QImageIOHandler::ImageOption option) const override;

    static bool canRead(QIODevice *device);

private:
    const QScopedPointer<PFMHandlerPrivate> d;
};

class PFMPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "pfm.json")

public:
    Capabilities capabilities(QIODevice *device, const QByteArray &format) const override;
    QImageIOHandler *create(QIODevice *device, const QByteArray &format = QByteArray()) const override;
};

#endif // KIMG_PFM_P_H
